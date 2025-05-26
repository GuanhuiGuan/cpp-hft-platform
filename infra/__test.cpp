#include "thread_util.h"
#include "alloc.h"

#include <atomic>
#include <cstring>

struct Foo {
    char ca_[3];
    Foo() {std::cout << "creating Foo()\n";}
    Foo(const char* ca) {
        std::cout << "creating Foo " << ca << '\n';
        strcpy(ca_, ca);
    }
    ~Foo() {std::cout << "destroying Foo " << ca_ << '\n';}
};

int main() {

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
    std::cout << it->ca_ << ',' << es->ca_ << ',' << pt->ca_ << '\n';
    fooAlloc.free(it, true);
    es->~Foo();
    fooAlloc.free(es);
    fooAlloc.free(pt, true);

    return 0;
}