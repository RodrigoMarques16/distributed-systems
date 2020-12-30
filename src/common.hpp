#pragma once

#include <array>
#include <string>
#include <unordered_map>

#include <moa.pb.h>

const std::unordered_map<std::string, moa::tag_t> str_to_tag = {
    {"TRIAL",   moa::tag_t::TRIAL},
    {"LICENSE", moa::tag_t::LICENSE},
    {"SUPPORT", moa::tag_t::SUPPORT},
    {"BUG",     moa::tag_t::BUG},
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

moa::tag_t parse_tag(const std::string& s) {
    auto result = str_to_tag.find(s);
    if (result == str_to_tag.end()) {
        std::cout << "Failed to parse provided tag" << std::endl;
        exit(EXIT_FAILURE);
    } 
    return result->second;
}