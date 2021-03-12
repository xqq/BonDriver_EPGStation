//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_CONFIG_HPP
#define BONDRIVER_EPGSTATION_CONFIG_HPP

#include <optional>
#include <string>

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
private:
    bool is_loaded_;
    std::optional<std::string> base_url_;
    std::optional<EPGStationVersion> version_;
    std::optional<BasicAuth> basic_auth_;
    std::optional<int> mpegts_streaming_mode_;
    std::optional<bool> show_inactive_services_;
};

#endif // BONDRIVER_EPGSTATION_CONFIG_HPP
