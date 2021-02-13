//
// @author magicxqq <xqq@xqq.im>
//

#include <cpprest/http_client.h>
#include <nlohmann/json.hpp>
#include "epgstation_models_deserialize.hpp"
#include "log.hpp"
#include "utils.hpp"
#include "epgstation_api.hpp"

using namespace web::http;
using namespace web::http::client;
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
    http_client_config client_config;

    if (has_basic_auth_) {
        web::credentials credentials(UTF8ToPlatformString(basicauth_user_), UTF8ToPlatformString(basicauth_password_));
        client_config.set_credentials(credentials);
    }

    http_client client(UTF8ToPlatformString(base_url_), client_config);
    bool succeed = false;
    bool error = false;
    EPGStation::Config config;

    pplx::task<void> request_task = pplx::create_task([&]() {
        // Request
        return client.request(methods::GET, UTF8ToPlatformString(kEPGStationAPIConfig));
    }).then([&](const http_response& response) {
        // Handle response
        if (response.status_code() != status_codes::OK) {
            error = true;
        }
        return response.extract_utf8string();
    }).then([&](const std::string& response_string) {
        // Handle JSON
        if (error) {
            Log::ErrorF("%s failed: %s", kEPGStationAPIConfig, response_string.c_str());
            return;
        }

        json j = json::parse(response_string);
        config = j.get<EPGStation::Config>();
        succeed = true;
    });

    try {
        request_task.wait();
    } catch (const std::exception& ex) {
        Log::ErrorF(LOG_FILE_FUNCTION_MESSAGE(ex.what()));
    }

    return succeed ? std::make_optional(config) : std::nullopt;
}


std::optional<EPGStation::Channels> EPGStationAPI::GetChannels() {
    http_client_config client_config;

    if (has_basic_auth_) {
        web::credentials credentials(UTF8ToPlatformString(basicauth_user_), UTF8ToPlatformString(basicauth_password_));
        client_config.set_credentials(credentials);
    }

    http_client client(UTF8ToPlatformString(base_url_), client_config);
    bool succeed = false;
    bool error = false;
    EPGStation::Channels channels;

    pplx::task<void> request_task = pplx::create_task([&]() {
        return client.request(methods::GET, UTF8ToPlatformString(kEPGStationAPIChannels));
    }).then([&](const http_response& response) {
        if (response.status_code() != status_codes::OK) {
            error = true;
        }
        return response.extract_utf8string();
    }).then([&](const std::string& response_string) {
        if (error) {
            Log::ErrorF("%s failed: %s", kEPGStationAPIChannels, response_string.c_str());
            return;
        }
        json j = json::parse(response_string);
        channels = j.get<EPGStation::Channels>();
        succeed = true;
    });

    try {
        request_task.wait();
    } catch (const std::exception& ex) {
        Log::ErrorF(LOG_FILE_FUNCTION_MESSAGE(ex.what()));
    }

    return succeed ? std::make_optional(std::move(channels)) : std::nullopt;
}

std::string EPGStationAPI::GetMpegtsLiveStreamPathQuery(int64_t id, int encode_mode) {
    std::string path_query = kEPGStationAPIStreamsLive;
    path_query.append(std::to_string(id));
    path_query.append("/mpegts?mode=");
    path_query.append(std::to_string(encode_mode));
    return path_query;
}
