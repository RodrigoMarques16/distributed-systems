#pragma once

#include <vector>
#include <mutex>

#include "common.hpp"

template<typename T>
struct Database {
    using Container = std::vector<T>;

    std::array<Container, 4> db;
    std::array<std::mutex, 4> mutexes;

    time_t TTL; // how long to keep data around for, in seconds

    Database(time_t t) : TTL(t) {}

    void write(const tag_t tag, const T& s) {
        std::unique_lock lk(mutexes[tag]);
        db[tag].push_back(s);
    }

    Container read(const tag_t tag) {
        if (db[tag].empty()) 
            return {};
        std::unique_lock lk(mutexes[tag]);
        clear_expired(tag);
        return Container(db[tag].begin(), db[tag].end());
    }

    void clear_expired(const tag_t tag) {
        auto it = db[tag].begin();
        int count = 0;
        while(has_expired(*it) && it != db[tag].end()) {
            ++it;
            ++count;
        }
        std::cout << count << " messages have expired" << std::endl;
        db[tag] = Container(it, db[tag].end());
    }

    inline bool has_expired(const T& msg) {
        auto current_time = get_current_time();
        std::cout << "|" << current_time << "-" << msg.timestamp() << "=" << current_time - msg.timestamp() <<  "|\n";
        return current_time - msg.timestamp() >= TTL;
    }

    inline size_t size(const tag_t tag) {
        return db[tag].size();
    }

};