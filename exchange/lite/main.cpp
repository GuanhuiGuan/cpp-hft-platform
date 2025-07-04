#include "lite_order_book.h"

int main() {
    match::OrderBook book;
    book.printTopOfBook();
    book.addOrder(1, match::Side::BID, 1000, 10);
    book.addOrder(2, match::Side::ASK, 1010, 10);
    book.printTopOfBook();
    book.modOrder(1, 12);
    book.printTopOfBook();
    book.addOrder(3, match::Side::BID, 1020, 15);
    book.printTopOfBook();
    book.addOrder(4, match::Side::ASK, 900, 20);
    book.printTopOfBook();
    book.cancelOrder(4);
    book.addOrder(5, match::Side::BID, 999, 3);
    book.printTopOfBook();

    return 0;
}