#pragma once

#include <deque>
#include <mutex>
#include <condition_variable>

template<typename T>
class ConcurrentVector {
    using Container = std::vector<T>;
    using Mutex     = std::mutex;
    using Lock      = std::lock_guard<Mutex>;

    Container vec;
    Mutex mut;

public:

    ConcurrentVector() : vec(), mut() {}

};

template<typename T>
struct Database {
    using namespace moa;
    
    using Container = std::vector<T>;
    using Lock      = std::lock_guard<std::mutex>;

    std::array<Container, 4> db;
    std::array<std::mutex, 4> mutexes;
    std::array<std::condition_variable, 4> condition_vars;
    std::array<bool, 4> notify = {false, false, false, false};

    long TTL; // how long to keep data around for, in seconds

    Database(int t) : TTL(t) {}

    void write(const tag_t tag, const T& s) {o
        {
            Lock lk(mutexes[tag]);
            db[tag].push_back(s);
        }
        condition_vars[tag].notify_all();
    }

    T wait_for_new(const tag_t tag) {
        conditional_vars[tag].wait();
        return *(--db.end());
    }

    std::vector<T> read_all(const tag_t tag) {
        Lock lk(mutexes[tag]);
        if (db[tag].empty()) 
            return {};
        clear_expired(tag);
        return std::vector<T>(db[tag].begin(), db[tag].end());
    }

    void clear_expired(const tag_t tag) {
        auto it = db[tag].begin();
        while(has_expired(*it))
            ++it;
        db[tag] = Container(it, db[tag].end());
    }

    inline bool has_expired(const T& msg) {
        using namespace std::chrono;
        auto current_time = system_clock::now().time_since_epoch().count();
        return current_time - msg.timestamp() >= TTL;
    }

};