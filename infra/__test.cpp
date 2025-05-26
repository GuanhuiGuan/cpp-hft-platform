#include "thread_util.h"

#include <atomic>

int main() {

    std::atomic<int> res;

    auto t = hft::common::startThread(0, "testThread", [&res](int x, int y){res.store(x + y);}, 10, 20);
    t->join();
    std::cout << res.load() << '\n';

    return 0;
}