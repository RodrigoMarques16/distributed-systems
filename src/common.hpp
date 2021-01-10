#pragma once

#include <moa.pb.h>

#include <array>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

const std::unordered_map<std::string, moa::tag_t> str_to_tag = {
    {"TRIAL", moa::tag_t::TRIAL},
    {"LICENSE", moa::tag_t::LICENSE},
    {"SUPPORT", moa::tag_t::SUPPORT},
    {"BUG", moa::tag_t::BUG},
};

const std::array<std::string, 4> message_texts = {
    "Trial downloaded",
    "License purchased",
    "Support request received",
    "Bug report received"
};

const std::array<std::string, 4> tag_to_str = {
    "TRIAL",
    "LICENSE",
    "SUPPORT",
    "BUG"
};

std::optional<moa::tag_t> parse_tag(const std::string& s) {
    auto result = str_to_tag.find(s);
    if (result == str_to_tag.end()) {
        std::cout << "Failed to parse provided tag" << std::endl;
        return std::nullopt;
    }
    return result->second;
}

// https://www.fluentcpp.com/2017/04/21/how-to-split-a-string-in-c/
std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

inline std::string message_to_text(moa::Message message) {
    std::stringstream ss;
    ss << "Received message:\n"
       << message.id() << '\n'
       << message.tag() << '\n'
       << message.timestamp() << '\n'
       << message.msg() << '\n';
    return ss.str();
}

inline void print_message(moa::Message message) {
    std::cout << message_to_text(message);
}