//
// @author magicxqq <xqq@xqq.im>
//

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "epgstation_models_deserialize.hpp"
#include "log.hpp"
#include "utils.hpp"
#include "epgstation_api.hpp"

using json = nlohmann::json;

static const char* kEPGStationAPIConfig = "/api/config";
static const char* kEPGStationAPIChannels = "/api/channels";
static const char* kEPGStationAPIStreamsLive = "/api/streams/live/";

EPGStationAPI::EPGStationAPI(const std::string& base_url) : base_url_(base_url), has_basic_auth_(false) { }

bool EPGStationAPI::SetBasicAuth(const std::string& user, const std::string& password) {
    has_basic_auth_ = true;
    basicauth_user_ = user;
    basicauth_password_ = password;
    return false;
}

std::optional<EPGStation::Config> EPGStationAPI::GetConfig() {
    cpr::Session session;
    session.SetUrl(cpr::Url{this->base_url_ + kEPGStationAPIConfig});

    if (has_basic_auth_) {
        session.SetAuth(cpr::Authentication{basicauth_user_, basicauth_password_});
    }

    cpr::Response response = session.Get();

    if (response.error) {
        Log::ErrorF("curl failed for %s: error_code = %d, msg = %s", kEPGStationAPIConfig, response.error.code, response.error.message.c_str());
        return std::nullopt;
    } else if (response.status_code >= 400) {
        Log::ErrorF("%s error: status_code = %d, body = %s", kEPGStationAPIConfig, response.status_code, response.text.c_str());
        return std::nullopt;
    }

    json j = json::parse(response.text);
    EPGStation::Config config = j.get<EPGStation::Config>();

    return config;
}

std::optional<EPGStation::Channels> EPGStationAPI::GetChannels() {
    cpr::Session session;
    session.SetUrl(cpr::Url{this->base_url_ + kEPGStationAPIChannels});

    if (has_basic_auth_) {
        session.SetAuth(cpr::Authentication{basicauth_user_, basicauth_password_});
    }

    cpr::Response response = session.Get();

    if (response.error) {
        Log::ErrorF("curl failed for %s: error_code = %d, msg = %s", kEPGStationAPIChannels, response.error.code, response.error.message.c_str());
        return std::nullopt;
    } else if (response.status_code >= 400) {
        Log::ErrorF("%s error: status_code = %d, body = %s", kEPGStationAPIChannels, response.status_code, response.text.c_str());
        return std::nullopt;
    }

    json j = json::parse(response.text);
    EPGStation::Channels channels = j.get<EPGStation::Channels>();

    return channels;
}

std::string EPGStationAPI::GetMpegtsLiveStreamPathQuery(int64_t id, int encode_mode) {
    std::string path_query = kEPGStationAPIStreamsLive;
    path_query.append(std::to_string(id));
    path_query.append("/mpegts?mode=");
    path_query.append(std::to_string(encode_mode));
    return path_query;
}
