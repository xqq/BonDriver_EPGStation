//
// @author magicxqq <xqq@xqq.im>
//

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "epgstation_models_deserialize.hpp"
#include "log.hpp"
#include "string_utils.hpp"
#include "epgstation_api.hpp"

using json = nlohmann::json;

static const char* kEPGStationAPI_Config = "/api/config";
static const char* kEPGStationAPI_Channels = "/api/channels";
static const char* kEPGStationAPIv1_Broadcasting = "/api/schedule/broadcasting";
static const char* kEPGStationAPIv2_Broadcasting = "/api/schedules/broadcasting?isHalfWidth=false";
static const char* kEPGStationAPI_StreamsLive = "/api/streams/live/";

EPGStationAPI::EPGStationAPI(const std::string& base_url, EPGStationVersion version)
    : base_url_(base_url), version_(version) {}

void EPGStationAPI::SetBasicAuth(const std::string& user, const std::string& password) {
    has_basic_auth_ = true;
    basicauth_user_ = user;
    basicauth_password_ = password;
}

void EPGStationAPI::SetProxy(const std::string& proxy) {
    has_proxy_ = true;
    proxy_ = proxy;
}

std::optional<EPGStation::Config> EPGStationAPI::GetConfig() {
    cpr::Session session;
    session.SetUrl(cpr::Url{this->base_url_ + kEPGStationAPI_Config});

    if (has_basic_auth_) {
        session.SetAuth(cpr::Authentication{basicauth_user_, basicauth_password_});
    }

    if (has_proxy_) {
        session.SetProxies({{"http", proxy_},
                            {"https", proxy_}});
    }

    cpr::Response response = session.Get();

    if (response.error) {
        Log::ErrorF("curl failed for %s: error_code = %d, msg = %s", kEPGStationAPI_Config, response.error.code, response.error.message.c_str());
        return std::nullopt;
    } else if (response.status_code >= 400) {
        Log::ErrorF("%s error: status_code = %d, body = %s", kEPGStationAPI_Config, response.status_code, response.text.c_str());
        return std::nullopt;
    }

    json j = json::parse(response.text);
    EPGStation::Config config = j.get<EPGStation::Config>();

    return config;
}

std::optional<EPGStation::Channels> EPGStationAPI::GetChannels() {
    cpr::Session session;
    session.SetUrl(cpr::Url{this->base_url_ + kEPGStationAPI_Channels});

    if (has_basic_auth_) {
        session.SetAuth(cpr::Authentication{basicauth_user_, basicauth_password_});
    }

    if (has_proxy_) {
        session.SetProxies({{"http", proxy_},
                            {"https", proxy_}});
    }

    cpr::Response response = session.Get();

    if (response.error) {
        Log::ErrorF("curl failed for %s: error_code = %d, msg = %s", kEPGStationAPI_Channels, response.error.code, response.error.message.c_str());
        return std::nullopt;
    } else if (response.status_code >= 400) {
        Log::ErrorF("%s error: status_code = %d, body = %s", kEPGStationAPI_Channels, response.status_code, response.text.c_str());
        return std::nullopt;
    }

    json j = json::parse(response.text);
    EPGStation::Channels channels = j.get<EPGStation::Channels>();

    return channels;
}

std::optional<EPGStation::Broadcasting> EPGStationAPI::GetBroadcasting() {
    cpr::Session session;

    if (version_ == kEPGStationVersionV1) {
        session.SetUrl(cpr::Url{this->base_url_ + kEPGStationAPIv1_Broadcasting});
    } else if (version_ == kEPGStationVersionV2) {
        session.SetUrl(cpr::Url{this->base_url_ + kEPGStationAPIv2_Broadcasting});
    }

    if (has_basic_auth_) {
        session.SetAuth(cpr::Authentication{basicauth_user_, basicauth_password_});
    }

    if (has_proxy_) {
        session.SetProxies({{"http", proxy_},
                            {"https", proxy_}});
    }

    cpr::Response response = session.Get();

    if (response.error) {
        Log::ErrorF("curl failed for %s: error_code = %d, msg = %s", kEPGStationAPIv1_Broadcasting, response.error.code, response.error.message.c_str());
        return std::nullopt;
    } else if (response.status_code >= 400) {
        Log::ErrorF("%s error: status_code = %d, body = %s", kEPGStationAPIv1_Broadcasting, response.status_code, response.text.c_str());
        return std::nullopt;
    }

    json j = json::parse(response.text);
    EPGStation::Broadcasting broadcasting = j.get<EPGStation::Broadcasting>();

    return broadcasting;
}

std::string EPGStationAPI::GetMpegtsLiveStreamPathQuery(int64_t id, int encode_mode) {
    std::string path_query = kEPGStationAPI_StreamsLive;
    path_query.append(std::to_string(id));

    if (version_ == kEPGStationVersionV1) {
        path_query.append("/mpegts?mode=");
    } else if (version_ == kEPGStationVersionV2) {
        path_query.append("/m2ts?mode=");
    }

    path_query.append(std::to_string(encode_mode));
    return path_query;
}
