//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_CONFIG_HPP
#define BONDRIVER_EPGSTATION_CONFIG_HPP

#include <optional>
#include <string>
#include <map>

enum EPGStationVersion : int {
    kEPGStationVersionV1 = 1,
    kEPGStationVersionV2 = 2,
};

struct BasicAuth {
    std::string user;
    std::string password;
};

class Config {
public:
    Config();
    bool LoadYamlFile(const std::string& filename);
    bool IsLoaded() const;

    [[nodiscard]] std::optional<std::string> GetBaseURL() const;
    [[nodiscard]] std::optional<EPGStationVersion> GetVersion() const;
    [[nodiscard]] std::optional<BasicAuth> GetBasicAuth() const;
    [[nodiscard]] std::optional<int> GetMpegTsStreamingMode() const;
    [[nodiscard]] std::optional<bool> GetShowInactiveServices() const;
    [[nodiscard]] std::optional<std::string> GetUserAgent() const;
    [[nodiscard]] std::optional<std::string> GetProxy() const;
    [[nodiscard]] std::optional<std::map<std::string, std::string>> GetHeaders() const;
private:
    bool is_loaded_;
    std::optional<std::string> base_url_;
    std::optional<EPGStationVersion> version_;
    std::optional<BasicAuth> basic_auth_;
    std::optional<int> mpegts_streaming_mode_;
    std::optional<bool> show_inactive_services_;
    std::optional<std::string> user_agent_;
    std::optional<std::string> proxy_;
    std::optional<std::map<std::string, std::string>> headers_;
};

#endif // BONDRIVER_EPGSTATION_CONFIG_HPP
