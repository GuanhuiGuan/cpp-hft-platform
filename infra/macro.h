#pragma once

#include <string>
#include <iostream>

namespace infra {

    inline auto ASSERT(bool expr, const std::string& msg) {
        [[unlikely]] if (!expr) {
            std::cerr << "ASSERT: " << msg << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}