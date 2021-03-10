//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_EPGSTATION_MODELS_HPP
#define BONDRIVER_EPGSTATION_EPGSTATION_MODELS_HPP

#include <cstdint>
#include <string>
#include <vector>
#include "string_utils.hpp"

namespace EPGStation {

struct Error {
    int code = 0;
    std::string message;
    std::string errors;
};

struct Channel {
    int64_t id = 0;
    int service_id = 0;
    int network_id = 0;
    std::string name;
    int remote_control_key_id = 0;
    bool has_logo_data = false;
    std::string channel_type;
    int channel_type_id = 0;
    std::string channel;
    int type = 0;
};

struct Channels {
    std::vector<Channel> channels;
};

struct Config {
public:
    struct Broadcast {
        bool GR = false;
        bool BS = false;
        bool CS = false;
        bool SKY = false;
    };
public:
    bool enable_live_streaming = false;
    Broadcast broadcast;
};

}

#endif // BONDRIVER_EPGSTATION_EPGSTATION_MODELS_HPP
