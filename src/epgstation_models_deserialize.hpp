//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_EPGSTATION_MODELS_DESERIALIZE_HPP
#define BONDRIVER_EPGSTATION_EPGSTATION_MODELS_DESERIALIZE_HPP

#include <cassert>
#include <nlohmann/json.hpp>
#include "epgstation_models.hpp"

namespace EPGStation {

// Error
void from_json(const nlohmann::json& json, Error& error) {
    error.code = json.at("code").get<int>();
    error.message = json.at("message").get<std::string>();
    error.errors = json.at("errors").get<std::string>();
}

// Channel
void from_json(const nlohmann::json& json, Channel& channel) {
    assert(json.is_object());

    channel.id = json.at("id").get<int64_t>();
    channel.service_id = json.at("serviceId").get<int>();
    channel.network_id = json.at("networkId").get<int>();
    channel.name = json.at("name").get<std::string>();
    channel.has_logo_data = json.at("hasLogoData").get<int>();
    channel.channel_type = json.at("channelType").get<std::string>();

    // optional field
    if (json.find("remoteControlKeyId") != json.end()) {
        channel.remote_control_key_id = json.at("remoteControlKeyId").get<int>();
    }

    if (json.find("channelTypeId") != json.end()) {
        channel.channel_type_id = json.at("channelTypeId").get<int>();
    }

    if (json.find("channel") != json.end()) {
        channel.channel = json.at("channel").get<std::string>();
    }

    if (json.find("type") != json.end()) {
        channel.type = json.at("type").get<int>();
    }
}

// Channels
void from_json(const nlohmann::json& json, Channels& channels) {
    assert(json.is_array());

    for (const nlohmann::json& element : json) {
        auto channel = element.get<Channel>();
        channels.channels.push_back(std::move(channel));
    }
}

void from_json(const nlohmann::json& json, Broadcasting& broadcasting) {
    assert(json.is_array());

    for (const nlohmann::json& element : json) {
        auto channel = element["channel"].get<Channel>();
        broadcasting.channels.push_back(std::move(channel));
    }
}

// Config
void from_json(const nlohmann::json& json, Config& config) {
    config.enable_live_streaming = json.at("enableLiveStreaming").get<bool>();
    config.broadcast.GR = json["broadcast"]["GR"].get<bool>();
    config.broadcast.BS = json["broadcast"]["BS"].get<bool>();
    config.broadcast.CS = json["broadcast"]["CS"].get<bool>();
    config.broadcast.SKY = json["broadcast"]["SKY"].get<bool>();
}

}

#endif // BONDRIVER_EPGSTATION_EPGSTATION_MODELS_DESERIALIZE_HPP
