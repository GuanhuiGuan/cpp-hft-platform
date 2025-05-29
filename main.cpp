#include "exchange/matcher/me_order_book.h"
#include "exchange/matcher/match_engine.h"

#include <csignal>

using namespace infra;

Logger* logger = new Logger("../logs/app.log");

void sigHandler(int sig) {
    std::cerr << "start of sigHandler: signal=" << sig << std::endl;
    std::this_thread::sleep_for(5s);
    if (logger) {
        delete logger;
        logger = nullptr;
    }
    std::this_thread::sleep_for(5s);
    std::cerr << "end of sigHandler: signal=" << sig << std::endl;
    std::exit(EXIT_SUCCESS);
}

int main() {
    // graceful shutdown
    std::signal(SIGINT, sigHandler);
    
    exchange::MatchEngine me;
    exchange::MEOrderBook ob(InsId(0), logger, &me);

    std::this_thread::sleep_for(2s);

    return 0;
}