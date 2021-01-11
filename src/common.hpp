#pragma once

#include <moa.pb.h>

#include <array>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <ostream>

enum tag_t {
    TRIAL,
    LICENSE,
    SUPPORT,
    BUG
};

const std::unordered_map<std::string, tag_t> str_to_tag = {
    {"TRIAL", tag_t::TRIAL},
    {"LICENSE", tag_t::LICENSE},
    {"SUPPORT", tag_t::SUPPORT},
    {"BUG", tag_t::BUG},
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

std::optional<tag_t> parse_tag(const std::string& s) {
    auto result = str_to_tag.find(s);
    if (result == str_to_tag.end()) {
        std::cout << "Failed to parse provided tag" << std::endl;
        return std::nullopt;
    }
    return result->second;
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

inline void print_message(moa::Message message) {
    std::cout << message.id() << '\n'
              << message.tag() << '\n'
              << message.timestamp() << '\n'
              << message.msg() << std::endl;
}

std::ostream& operator<<(std::ostream& os, const moa::Message& message){
    os << message.id() << '\n'
       << message.tag() << '\n'
       << message.timestamp() << '\n'
       << message.msg();
    return os;
}

time_t get_current_time() {
    using namespace std::chrono;
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}