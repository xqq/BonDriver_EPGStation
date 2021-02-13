//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_EPGSTATION_API_HPP
#define BONDRIVER_EPGSTATION_EPGSTATION_API_HPP

#include <optional>
#include "epgstation_models.hpp"

class EPGStationAPI {
public:

public:
    EPGStationAPI(const std::string& base_url);

    bool SetBasicAuth(const std::string& user, const std::string& password);
    std::optional<EPGStation::Config> GetConfig();
    std::optional<EPGStation::Channels> GetChannels();
private:
    std::string base_url_;
    bool has_basic_auth_;
    std::string basicauth_user_;
    std::string basicauth_password_;
};


#endif // BONDRIVER_EPGSTATION_EPGSTATION_API_HPP
