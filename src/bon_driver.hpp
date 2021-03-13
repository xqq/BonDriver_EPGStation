//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_BON_DRIVER_HPP
#define BONDRIVER_EPGSTATION_BON_DRIVER_HPP

#include <vector>
#include <string>
#include <unordered_set>
#include <memory>
#include "IBonDriver2.h"
#include "config.hpp"
#include "epgstation_models.hpp"
#include "epgstation_api.hpp"

class StreamLoader;

class BonDriver : public IBonDriver2 {
public:
    BonDriver(const Config& config);
    ~BonDriver();
protected:
    void Release(void) override;

    const BOOL OpenTuner(void) override;
    void CloseTuner(void) override;
    const BOOL SetChannel(const BYTE bCh) override;
    const float GetSignalLevel(void) override;
    const DWORD WaitTsStream(const DWORD dwTimeOut = 0) override;
    const DWORD GetReadyCount(void) override;
    const BOOL GetTsStream(BYTE* pDst, DWORD* pdwSize, DWORD* pdwRemain) override;
    const BOOL GetTsStream(BYTE** ppDst, DWORD* pdwSize, DWORD* pdwRemain) override;
    void PurgeTsStream(void) override;

    LPCTSTR GetTunerName(void) override;
    const BOOL IsTunerOpening(void) override;
    LPCTSTR EnumTuningSpace(const DWORD dwSpace) override;
    LPCTSTR EnumChannelName(const DWORD dwSpace, const DWORD dwChannel) override;
    const BOOL SetChannel(const DWORD dwSpace, const DWORD dwChannel) override;
    const DWORD GetCurSpace(void) override;
    const DWORD GetCurChannel(void) override;
private:
    void InitChannels();
private:
    const Config& yaml_config_;
    EPGStationAPI api_;

    bool init_channels_succeed = false;
    EPGStation::Config epgstation_config_;
    std::vector<EPGStation::Channel> channels_;

    size_t chunk_size_ = 188 * 1024;
    std::unique_ptr<StreamLoader> stream_loader_;

    EPGStation::Channel current_channel_;
    DWORD current_dwspace_ = 0;
    DWORD current_dwchannel_ = 0;

    std::unordered_set<std::string> space_set_;
    std::vector<std::string> space_types_;
    std::vector<size_t> space_channel_bases_;
};


#endif // BONDRIVER_EPGSTATION_BON_DRIVER_HPP
