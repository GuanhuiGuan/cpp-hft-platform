#include "mpsc_lfq.h"
#include "thread_util.h"

void testMpsc(int numProducer, int loop) {
    auto start = std::chrono::steady_clock::now();
    std::cout << "\n--- Start of MPSC Test ---\n";
    std::atomic_long sum {0LL};
    infra::MpscQueue<long> msq(1024 * 64);
    
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
    std::unique_ptr<std::thread> consumer (infra::startThreadUptr(0, "consumer", runConsumer));

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
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    std::cout << "MPSC res: " << sum << ", elapsed: " << time << "\n"; // expected: np * (np-1) / 2 * loop
    std::cout << "--- End of MPSC Test ---\n";
}

int main() {
    for (auto i = 0; i < 5; ++i) {
        testMpsc(20, 100);
    }
    
    return 0;
}
