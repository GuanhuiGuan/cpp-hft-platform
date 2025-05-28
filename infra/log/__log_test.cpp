#include "logger.h"

int main() {
    infra::Logger logger("logs/test.log");
    logger.log(infra::LogType::INFO, "Hello there % % % %%", 100, "DV", 66.666);

    // logger.flushBuffer();

    return 0;
}