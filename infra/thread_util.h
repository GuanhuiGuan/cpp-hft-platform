#pragma once

#include <sched.h>
#include <pthread.h>
#include <string>
#include <thread>
#include <iostream>

namespace hft::common {

    inline auto setThreadAffinity(int coreId) noexcept {
        cpu_set_t cpuSet; // flag sets of CPUs
        CPU_ZERO(&cpuSet); // clear all
        CPU_SET(coreId, &cpuSet);
        return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet) == 0);
    }

    template<typename F, typename... Args>
    inline auto startThread(int coreId, const std::string& name, F&& func, Args&&... args) {
        auto t = [&]() {
            if (coreId >= 0 && !setThreadAffinity(coreId)) {
                std::cerr << "Failed to set affinity for " << name << " " << pthread_self() << " to " << coreId << std::endl;
                exit(EXIT_FAILURE);
            }
            std::cerr << "Affinity set for " << name << " " << pthread_self() << " to " << coreId << std::endl;
            std::forward<F>(func)(std::forward<Args>(args)...);
        };
        return t;
    }
}