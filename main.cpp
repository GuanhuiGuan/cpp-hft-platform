#include "exchange/matcher/me_order_book.h"
#include "exchange/matcher/match_engine.h"

#include <csignal>

using namespace infra;

Logger* logger {nullptr};

void sigHandler(int sig) {
    std::cerr << "start of sigHandler: signal=" << sig << std::endl;
    std::this_thread::sleep_for(3s);
    if (logger) {
        delete logger;
        logger = nullptr;
    }
    std::this_thread::sleep_for(2s);
    std::cerr << "end of sigHandler: signal=" << sig << std::endl;
    std::exit(EXIT_SUCCESS);
}

int main() {
    // graceful shutdown
    std::signal(SIGINT, sigHandler);

    logger = new Logger("logs/app.log");
    
    exchange::MatchEngine me;
    exchange::MEOrderBook ob(InsId(0), logger, &me);

    ob.add(1, 100, Side::BID, 10000, 10);
    ob.add(1, 101, Side::BID, 10001, 10);
    ob.add(2, 102, Side::ASK, 10002, 5);
    ob.add(2, 103, Side::ASK, 10000, 15);

    while (true) {
        std::this_thread::sleep_for(1s);
    }

    return 0;
}