#include "lite_order_book.h"

int main() {
    match::OrderBook book;
    book.printTopOfBook();
    book.addOrder(1, match::Side::BID, match::OrderType::GFD, 1000, 10);
    book.addOrder(111, match::Side::BID, match::OrderType::GFD, 1000, 2);
    book.addOrder(2, match::Side::ASK, match::OrderType::GFD, 1010, 10);
    book.printTopOfBook();
    book.modOrder(111, match::Side::ASK, 1001, 3);
    book.printTopOfBook();
    book.addOrder(3, match::Side::BID, match::OrderType::GFD, 1020, 15);
    book.printTopOfBook();
    book.addOrder(4, match::Side::ASK, match::OrderType::GFD, 900, 20);
    book.printTopOfBook();
    book.cancelOrder(4);
    book.addOrder(5, match::Side::BID, match::OrderType::IOC, 999, 3);
    book.printTopOfBook();

    return 0;
}