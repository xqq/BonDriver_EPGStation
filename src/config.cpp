//
// @author magicxqq <xqq@xqq.im>
//

#include <yaml-cpp/yaml.h>
#include "log.hpp"
#include "string_utils.hpp"
#include "config.hpp"

Config::Config() : is_loaded_(false) { }

bool Config::LoadYamlFile(const std::string& filename) {
    try {
        YAML::Node config = YAML::LoadFile(filename);

        if (config["baseURL"]) {
            std::string base_url = config["baseURL"].as<std::string>();
            base_url_ = StringUtils::RemoveSuffixSlash(base_url);
        } else {
            Log::ErrorF("Missing baseURL field in config file");
            return false;
        }

        if (config["version"]) {
            std::string version_desc = config["version"].as<std::string>();
            if (version_desc == "v1") {
                version_ = kEPGStationVersionV1;
            } else if (version_desc == "v2") {
                version_ = kEPGStationVersionV2;
            } else {
                Log::ErrorF("Incorrect EPGStation version: %s", version_desc.c_str());
                return false;
            }
        } else {
            Log::ErrorF("Missing version field in config file");
            return false;
        }

        if (config["basicAuth"]) {
            const YAML::Node& basic_auth_node = config["basicAuth"];
            if (!basic_auth_node.IsMap() || basic_auth_node.size() != 2) {
                Log::ErrorF("Invalid basicAuth parameter in config file");
                return false;
            }

            BasicAuth basic_auth;
            basic_auth.user = basic_auth_node["user"].as<std::string>();
            basic_auth.password = basic_auth_node["password"].as<std::string>();

            basic_auth_ = basic_auth;
        } // else: basicAuth is optional

        if (config["mpegTsStreamingMode"]) {
            mpegts_streaming_mode_ = config["mpegTsStreamingMode"].as<int>();
        } else {
            Log::ErrorF("Missing mpegTsStreamingMode field in config file");
            return false;
        }

        if (config["showInactiveServices"]) {
            show_inactive_services_ = config["showInactiveServices"].as<bool>();
        } // else: showInactiveServices is optional

        if (config["userAgent"]) {
            user_agent_ = config["userAgent"].as<std::string>();
        } else {
            // default UserAgent
            user_agent_ = "BonDriver_Mirakurun";
        }

        if (config["proxy"]) {
            proxy_ = config["proxy"].as<std::string>();
        } // else: proxy is optional

        if (config["headers"]) {
            auto& headers_node = config["headers"];

            if (headers_node.IsMap()) {
                std::map<std::string, std::string> headers;
                for (auto iter = headers_node.begin(); iter != headers_node.end(); ++iter) {
                    headers.insert({iter->first.as<std::string>(), iter->second.as<std::string>()});
                }
                headers_ = headers;
            } else {
                Log::ErrorF("headers field must be a map");
            }
        } // else: headers is optional

    } catch (YAML::BadFile& ex) {
        Log::ErrorF("Load yaml file failed, %s", ex.what());
        return false;
    } catch (YAML::InvalidNode& ex) {
        Log::ErrorF("Parse yaml file failed, %s", ex.what());
        return false;
    }

    is_loaded_ = true;
    return true;
}

bool Config::IsLoaded() const {
    return is_loaded_;
}

std::optional<std::string> Config::GetBaseURL() const {
    return base_url_;
}

std::optional<EPGStationVersion> Config::GetVersion() const {
    return version_;
}

std::optional<BasicAuth> Config::GetBasicAuth() const {
    return basic_auth_;
}

std::optional<int> Config::GetMpegTsStreamingMode() const {
    return mpegts_streaming_mode_;
}

std::optional<bool> Config::GetShowInactiveServices() const {
    return show_inactive_services_;
}

std::optional<std::string> Config::GetUserAgent() const {
    return user_agent_;
}

std::optional<std::string> Config::GetProxy() const {
    return proxy_;
}

std::optional<std::map<std::string, std::string>> Config::GetHeaders() const {
    return headers_;
}
