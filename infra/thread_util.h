#pragma once

#include <sched.h>
#include <pthread.h>
#include <string>
#include <thread>
#include <iostream>
#include <memory>

namespace infra {

    inline auto setThreadAffinity(int coreId) noexcept {
        #ifdef __linux__
            cpu_set_t cpuSet; // flag sets of CPUs
            CPU_ZERO(&cpuSet); // clear all
            CPU_SET(coreId, &cpuSet);
            return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuSet) == 0);
        #else
            return true;
        #endif
    }

    template<typename F, typename... Args>
    inline auto startThread(int coreId, const std::string& name, F&& func, Args&&... args) {
        auto t = std::make_unique<std::thread>([&]() {
            if (coreId >= 0 && !setThreadAffinity(coreId)) {
                std::cerr << "Failed to set affinity for " << name << " " << pthread_self() << " to " << coreId << std::endl;
                exit(EXIT_FAILURE);
            }
            std::cerr << "Affinity set for " << name << " " << pthread_self() << " to " << coreId << std::endl;
            func(std::forward<Args>(args)...);
        });
        std::this_thread::sleep_for(100ms); // just in case referred objects in other threads aren't ready
        return t;
    }
}