//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_EPGSTATION_API_HPP
#define BONDRIVER_EPGSTATION_EPGSTATION_API_HPP

#include <optional>
#include "config.hpp"
#include "epgstation_models.hpp"

class EPGStationAPI {
public:

public:
    EPGStationAPI(const std::string& base_url, EPGStationVersion version);

    void SetBasicAuth(const std::string& user, const std::string& password);
    std::optional<EPGStation::Config> GetConfig();
    std::optional<EPGStation::Channels> GetChannels();
    std::optional<EPGStation::Broadcasting> GetBroadcasting();
    std::string GetMpegtsLiveStreamPathQuery(int64_t id, int encode_mode);
private:
    std::string base_url_;
    EPGStationVersion version_;
    bool has_basic_auth_ = false;
    std::string basicauth_user_;
    std::string basicauth_password_;
};


#endif // BONDRIVER_EPGSTATION_EPGSTATION_API_HPP
