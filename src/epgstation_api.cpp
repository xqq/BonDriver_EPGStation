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

void EPGStationAPI::SetUserAgent(const std::string& user_agent) {
    has_user_agent_ = true;
    user_agent_ = user_agent;
}

void EPGStationAPI::SetProxy(const std::string& proxy) {
    has_proxy_ = true;
    proxy_ = proxy;
}

void EPGStationAPI::SetHeaders(const std::map<std::string, std::string>& headers) {
    has_headers_ = true;
    headers_ = headers;
}

std::optional<EPGStation::Config> EPGStationAPI::GetConfig() {
    cpr::Session session;
    session.SetUrl(cpr::Url{this->base_url_ + kEPGStationAPI_Config});

    if (has_basic_auth_) {
        session.SetAuth(cpr::Authentication{basicauth_user_, basicauth_password_});
    }

    if (has_user_agent_) {
        session.SetUserAgent({user_agent_});
    }

    if (has_proxy_) {
        session.SetProxies({{"http", proxy_},
                            {"https", proxy_}});
    }

    if (has_headers_) {
        for (auto& pair : headers_) {
            cpr::Header header{{pair.first, pair.second}};
            session.UpdateHeader(header);
        }
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

    if (has_user_agent_) {
        session.SetUserAgent({user_agent_});
    }

    if (has_proxy_) {
        session.SetProxies({{"http", proxy_},
                            {"https", proxy_}});
    }

    if (has_headers_) {
        for (auto& pair : headers_) {
            cpr::Header header{{pair.first, pair.second}};
            session.UpdateHeader(header);
        }
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
    const char* path_query = "";

    if (version_ == kEPGStationVersionV1) {
        path_query = kEPGStationAPIv1_Broadcasting;
    } else if (version_ == kEPGStationVersionV2) {
        path_query = kEPGStationAPIv2_Broadcasting;
    }

    cpr::Session session;
    session.SetUrl(cpr::Url{this->base_url_ + path_query});

    if (has_basic_auth_) {
        session.SetAuth(cpr::Authentication{basicauth_user_, basicauth_password_});
    }

    if (has_user_agent_) {
        session.SetUserAgent({user_agent_});
    }

    if (has_proxy_) {
        session.SetProxies({{"http", proxy_},
                            {"https", proxy_}});
    }

    if (has_headers_) {
        for (auto& pair : headers_) {
            cpr::Header header{{pair.first, pair.second}};
            session.UpdateHeader(header);
        }
    }

    cpr::Response response = session.Get();

    if (response.error) {
        Log::ErrorF("curl failed for %s: error_code = %d, msg = %s", path_query, response.error.code, response.error.message.c_str());
        return std::nullopt;
    } else if (response.status_code >= 400) {
        Log::ErrorF("%s error: status_code = %d, body = %s", path_query, response.status_code, response.text.c_str());
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
