#include "me_order_book.h"

using namespace infra;

namespace exchange {

    MEOrderBook::MEOrderBook(InsId insId, Logger* logger, MatchEngine* me)
    : insId_{insId}, log_{logger}, me_{me} 
    , orderAtPriceMemPool_{MAX_PRICE_LVL}, orderMemPool_{MAX_ORDER_IDS}
    {
        log_->log(LogType::INFO, "new MEOrderBook(insId=%)", insId_);
    }

    MEOrderBook::~MEOrderBook() noexcept {
        log_->log(LogType::INFO, "del MEOrderBook(insId=%)", insId_);
    }

    auto MEOrderBook::add(UserId clientId, UserOrderId clientOrderId, Side side, Price price, Qty qty) noexcept -> void {
        MarketOrderId mid = nextMarketOrderId_++;
        MEOrder* order = orderMemPool_.alloc(insId_, clientId, clientOrderId, mid, side, price, qty);
        
        // TODO priority
        // TODO dispatch market update

        addOrder(order);
    }

    auto MEOrderBook::cancel(UserId clientId, UserOrderId clientOrderId) noexcept -> void {
        // TODO
    }

    void MEOrderBook::addOrder(MEOrder* order) {
        cidOidMap_.at(order->clientId_).at(order->clientOrderId_) = order;

        MEOrderAtPrice* op = orderAtPriceMap_.at(priceToIdx(order->price_));
        if (!op) {
            op = orderAtPriceMemPool_.alloc(order->side_, order->price_, order);
            addOrderAtPrice(op);
        }
        if (op->orders_) {
            order->next_ = op->orders_;
            op->orders_ = order;
        }
    }

    // TODO implement match
    void MEOrderBook::addOrderAtPrice(MEOrderAtPrice* op) {
        orderAtPriceMap_[priceToIdx(op->price_)] = op;

        bool isBuy = Side::BID == op->side_;
        MEOrderAtPrice*& best = isBuy ? bids_ : asks_;
        if (!best) {
            best = op;
        } else {
            MEOrderAtPrice* prev {nullptr};
            MEOrderAtPrice* curr = best;
            while (curr) {
                if (isBuy ? curr->price_ < op->price_ : curr->price_ > op->price_) {
                    break;
                }
                prev = curr;
                curr = curr->next_;
            }
            if (prev) prev->next_ = op;
            op->prev_ = prev;
            if (curr) curr->prev_ = op;
            op->next_ = curr;
        }
    }
}