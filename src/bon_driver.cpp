//
// @author magicxqq <xqq@xqq.im>
//

#include "log.hpp"
#include "stream_loader.hpp"
#include "bon_driver.hpp"

BonDriver::BonDriver(const Config& config) : yaml_config_(config), api_(config.GetBaseURL().value(), config.GetVersion().value()) {
    Log::InfoF(LOG_FUNCTION);
    if (config.GetBasicAuth().has_value()) {
        api_.SetBasicAuth(config.GetBasicAuth()->user, config.GetBasicAuth()->password);
    }
    InitChannels();
}

BonDriver::~BonDriver() {
    Log::InfoF(LOG_FUNCTION);
    if (stream_loader_) {
        CloseTuner();
    }
}

void BonDriver::Release(void) {
    Log::InfoF(LOG_FUNCTION);
    delete this;
}

void BonDriver::InitChannels() {
    std::optional<EPGStation::Config> config = api_.GetConfig();
    if (!config.has_value()) {
        Log::ErrorF("EPGStationAPI::GetConfig() failed");
        return;
    }
    if (!config->enable_live_streaming) {
        // Server doesn't enable live streaming, return failed
        Log::ErrorF("config->enable_live_streaming is false");
    }
    epgstation_config_ = config.value();


    auto show_inactive_services = yaml_config_.GetShowInactiveServices();

    if (show_inactive_services.has_value() && show_inactive_services.value() == true) {
        // showInactiveServices == true
        std::optional<EPGStation::Channels> channels_holder = api_.GetChannels();
        if (!channels_holder.has_value()) {
            Log::ErrorF("EPGStationAPI::GetChannels() failed");
            return;
        }
        channels_ = std::move(channels_holder->channels);
    } else {
        std::optional<EPGStation::Broadcasting> channels_holder = api_.GetBroadcasting();
        if (!channels_holder.has_value()) {
            Log::ErrorF("EPGStationAPI::GetBroadcasting() failed");
            return;
        }
        channels_ = std::move(channels_holder->channels);
    }


    for (size_t i = 0; i < channels_.size(); i++) {
        auto& channel = channels_[i];

        auto iter = space_set_.find(channel.channel_type);
        if (iter == space_set_.end()) {
            // channel type not found, insert as new space
            space_set_.insert(channel.channel_type);
            space_types_.push_back(channel.channel_type);
            space_channel_bases_.push_back(i);
        }
    }
}

const BOOL BonDriver::OpenTuner(void) {
    Log::InfoF(LOG_FUNCTION);

    if (!epgstation_config_.enable_live_streaming) {
        Log::ErrorF("config->enable_live_streaming is false, OpenTuner() failed");
        return FALSE;
    }

    if (channels_.empty()) {
        Log::ErrorF("Get channels failed or channel list is empty, OpenTuner() failed");
        return FALSE;
    }

    return TRUE;
}

void BonDriver::CloseTuner(void) {
    Log::InfoF(LOG_FUNCTION);

    if (stream_loader_) {
        if (stream_loader_->IsPolling()) {
            stream_loader_->Abort();
        }
        stream_loader_.reset();
    }

    current_channel_ = EPGStation::Channel();
    current_dwspace_ = 0;
    current_dwchannel_ = 0;
}

const BOOL BonDriver::SetChannel(const BYTE bCh) {
    return SetChannel(0, static_cast<DWORD>(bCh - 13));
}

const BOOL BonDriver::SetChannel(const DWORD dwSpace, const DWORD dwChannel) {
    Log::InfoF("BonDriver::SetChannel(): dwSpace = %u, dwChannel = %u", dwSpace, dwChannel);

    if (dwSpace >= space_types_.size()) {
        return FALSE;
    }

    if (dwSpace < space_types_.size() - 1) {
        if (dwChannel >= space_channel_bases_[static_cast<size_t>(dwSpace) + 1] - space_channel_bases_[dwSpace]) {
            return FALSE;
        }
    }

    size_t channel_index = space_channel_bases_[dwSpace] + dwChannel;
    if (channel_index >= channels_.size()) {
        return FALSE;
    }

    EPGStation::Channel& channel = channels_[channel_index];

    if (stream_loader_) {
        CloseTuner();
    }

    current_channel_ = channel;
    current_dwspace_ = dwSpace;
    current_dwchannel_ = dwChannel;

    stream_loader_ = std::make_unique<StreamLoader>(chunk_size_, 10, 3);

    std::string path_query = api_.GetMpegtsLiveStreamPathQuery(channel.id, yaml_config_.GetMpegTsStreamingMode().value());

    stream_loader_->Open(yaml_config_.GetBaseURL().value(), path_query, yaml_config_.GetBasicAuth());

    return TRUE;
}

const float BonDriver::GetSignalLevel(void) {
    if (!stream_loader_) {
        return 0;
    }
    return stream_loader_->GetCurrentSpeedKByte() * 8 / 1000.0f;
}

const DWORD BonDriver::WaitTsStream(const DWORD dwTimeOut) {
    if (!stream_loader_) {
        return static_cast<DWORD>(-1);
    }

    stream_loader_->WaitForResponse();
    stream_loader_->WaitForData();
    // TODO: Return value
    return 0;
}

const DWORD BonDriver::GetReadyCount(void) {
    if (!stream_loader_) {
        return 0;
    }

    return static_cast<DWORD>(stream_loader_->RemainReadable());
}

const BOOL BonDriver::GetTsStream(BYTE *pDst, DWORD *pdwSize, DWORD *pdwRemain) {
    if (!stream_loader_ || !stream_loader_->IsPolling()) {
        return FALSE;
    }

    if (stream_loader_->RemainReadable() == 0) {
        *pdwSize = 0;
        *pdwRemain = 0;
        return TRUE;
    }

    size_t bytes_read = stream_loader_->Read(static_cast<uint8_t*>(pDst), chunk_size_);
    *pdwSize = static_cast<DWORD>(bytes_read);
    *pdwRemain = static_cast<DWORD>(stream_loader_->RemainReadable());

    return TRUE;
}

const BOOL BonDriver::GetTsStream(BYTE **ppDst, DWORD *pdwSize, DWORD *pdwRemain) {
    if (!stream_loader_ || !stream_loader_->IsPolling()) {
        return FALSE;
    }

    if (stream_loader_->RemainReadable() == 0) {
        *pdwSize = 0;
        *pdwRemain = 0;
        return TRUE;
    }

    std::pair<uint8_t*, size_t> data = stream_loader_->ReadChunkAndRetain();

    *ppDst = data.first;
    *pdwSize = static_cast<DWORD>(data.second);
    *pdwRemain = static_cast<DWORD>(stream_loader_->RemainReadable());

    return TRUE;
}

void BonDriver::PurgeTsStream(void) {
    // do nothing
}

LPCTSTR BonDriver::GetTunerName(void) {
    static const TCHAR* kTunerName = TEXT("BonDriver_EPGStation");
    return kTunerName;
}

const BOOL BonDriver::IsTunerOpening(void) {
    return stream_loader_ && stream_loader_->IsPolling();
}

LPCTSTR BonDriver::EnumTuningSpace(const DWORD dwSpace) {
    if (dwSpace >= space_types_.size()) {
        return nullptr;
    }

    static PlatformString space_buffer;
    space_buffer = UTF8ToPlatformString(space_types_[dwSpace]);

    return space_buffer.c_str();
}

LPCTSTR BonDriver::EnumChannelName(const DWORD dwSpace, const DWORD dwChannel) {
    if (dwSpace >= space_types_.size()) {
        return nullptr;
    }

    if (dwSpace < space_types_.size() - 1) {
        if (dwChannel >= space_channel_bases_[static_cast<size_t>(dwSpace) + 1] - space_channel_bases_[dwSpace]) {
            return nullptr;
        }
    }

    size_t channel_index = space_channel_bases_[dwSpace] + dwChannel;
    if (channel_index >= channels_.size()) {
        return nullptr;
    }

    EPGStation::Channel& channel = channels_[channel_index];

    static PlatformString name_buffer;
    name_buffer = UTF8ToPlatformString(channel.name);

    return name_buffer.c_str();
}

const DWORD BonDriver::GetCurSpace(void) {
    return current_dwspace_;
}

const DWORD BonDriver::GetCurChannel(void) {
    return current_dwchannel_;
}
