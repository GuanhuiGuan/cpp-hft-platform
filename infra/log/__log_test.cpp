#include "logger.h"
#include <csignal>

infra::Logger* logger = nullptr;

void sigHandler(int) {
    std::this_thread::sleep_for(2s);
    if (logger) {
        delete logger;
        logger = nullptr;
    }
    std::this_thread::sleep_for(2s);
    std::exit(EXIT_SUCCESS);
}

int main() {
    // graceful shutdown
    std::signal(SIGINT, sigHandler);

    logger = new infra::Logger("logs/test.log");

    while (true) {
        logger->log(infra::LogType::INFO, "Hello there % % % %%", 100, "DV", 66.666);
        std::this_thread::sleep_for(1s);
    }

    return 0;
}