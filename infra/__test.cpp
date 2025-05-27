#include "thread_util.h"
#include "alloc.h"
#include "spsc_lfq.h"
#include "mpsc_lfq.h"

#include <atomic>
#include <cstring>
#include <memory>

struct Foo {
    char ca_[3];
    Foo() {
        // std::cout << "creating Foo()\n";
    }
    Foo(const char* ca) {
        // std::cout << "creating Foo " << ca << '\n';
        strcpy(ca_, ca);
    }
    ~Foo() {
        // std::cout << "destroying Foo " << ca_ << '\n';
    }
};
std::ostream& operator<<(std::ostream& os, const Foo& f) {
    return os << "Foo{" << f.ca_ << '}';
}

void testMpsc(int numProducer, int loop) {
    auto start = std::chrono::steady_clock::now();
    std::cout << "\n--- Start of MPSC Test ---\n";
    std::atomic_long sum {0LL};
    infra::MpscQueue<long> msq(65536);
    
    bool quit {false};
    auto runConsumer = [&msq, &sum, &quit]() {
        long x;
        while (!quit) {
            if (msq.dequeue(x)) {
                sum += x;
            } else {
                // std::this_thread::sleep_for(1ms);
            }
        }
    };
    std::unique_ptr<std::thread> consumer(infra::startThread(0, "consumer", runConsumer));

    auto runProducer = [&msq, loop](long i) {
        for (int l {0}; l < loop; ++l) {
            msq.enqueue(std::move(i));
        }
    };
    std::vector<std::thread> producers;
    for (int i {0}; i < numProducer; ++i) {
        producers.emplace_back(runProducer, i);
    }

    for (auto& p : producers) p.join();
    quit = true;
    consumer->join();
    
    auto duration = std::chrono::steady_clock::now() - start;
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    std::cout << "MPSC res: " << sum << ", elapsed: " << time << "\n"; // expected: np * (np-1) / 2 * loop
    std::cout << "--- End of MPSC Test ---\n";
}

int main() {

    std::cout << std::boolalpha;

    std::atomic<int> res;
    auto t = infra::startThread(0, "testThread", [&res](int x, int y){res.store(x + y);}, 10, 20);
    t->join();
    std::cout << res.load() << '\n';

    infra::Allocator<Foo> fooAlloc(2);
    Foo* it = fooAlloc.alloc(std::move("IT"));
    std::cout << fooAlloc.nextIdx() << '\n';
    Foo* es = fooAlloc.alloc(std::move("ES"));
    std::cout << fooAlloc.nextIdx() << '\n';
    Foo* pt = fooAlloc.alloc(std::move("PT"));
    std::cout << fooAlloc.nextIdx() << '\n';
    std::cout << fooAlloc.capacity() << '\n';
    std::cout << *it << ',' << *es << ',' << !pt << '\n';
    fooAlloc.free(it, true);
    pt = fooAlloc.alloc(std::move("DE"));
    std::cout << *pt << '\n';
    es->~Foo();
    fooAlloc.free(es);
    fooAlloc.free(pt, true);

    infra::SpscQueue<Foo> ssq(4);
    for (size_t i = 0; i < 6; ++i) ssq.enqueue({std::to_string(i).c_str()});
    std::cout << *ssq.atRead() << '\n';
    Foo x;
    while (ssq.size() > 0) std::cout << ssq.dequeue(x) << ':' << x;
    std::cout << std::endl;

    testMpsc(20, 100);

    return 0;
}