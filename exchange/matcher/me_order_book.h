#pragma once

#include "me_order.h"

namespace exchange {

    class MatchEngine;

    class MEOrderBook {
        InsId insId_;
        Logger* log_;
        MatchEngine* me_;

        MEOrderAtPrice* bids_;
        MEOrderAtPrice* asks_;

        Allocator<MEOrderAtPrice> orderAtPriceMemPool_;
        MEOrderAtPriceMap orderAtPriceMap_;

        Allocator<MEOrder> orderMemPool_;
        CidOidMap cidOidMap_;
        MarketOrderId nextMarketOrderId_ {0};
    public:
        MEOrderBook(InsId insId, Logger* logger, MatchEngine* me);
        ~MEOrderBook() noexcept;
        MEOrderBook(const MEOrderBook&) = delete;
        MEOrderBook(MEOrderBook&&) = delete;
        MEOrderBook& operator=(const MEOrderBook&) = delete;
        MEOrderBook& operator=(MEOrderBook&&) = delete;

        auto add(UserId clientId, UserOrderId clientOrderId, Side side, Price price, Qty qty) noexcept -> void;

        auto cancel(UserId clientId, UserOrderId clientOrderId) noexcept -> void;

    private:
        void addOrder(MEOrder* order);
        void addOrderAtPrice(MEOrderAtPrice* op);

        void match(MEOrder* order);

        auto priceToIdx(Price p) const {return p % orderAtPriceMap_.max_size();}
    };


/*************** implementation ***************/

    // MEOrderBook::MEOrderBook(InsId insId, Logger* logger, MatchEngine* me) : insId_{insId}, log_{logger}, me_{me} {

    // }

    MEOrderBook::MEOrderBook(InsId insId, Logger* logger, MatchEngine* me)
    : insId_{insId}, log_{logger}, me_{me} 
    , orderAtPriceMemPool_{MAX_PRICE_LVL}, orderMemPool_{MAX_ORDER_IDS}
    {
        log_->log(LogType::INFO, "new MEOrderBook(insId=%)", insId_);

        for (size_t i = 0; i < cidOidMap_.size(); ++i) cidOidMap_[i] = new OrderMap();
    }

    MEOrderBook::~MEOrderBook() noexcept {
        log_->log(LogType::INFO, "del MEOrderBook(insId=%)", insId_);

        for (size_t i = 0; i < cidOidMap_.size(); ++i) delete cidOidMap_[i];
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
        match(order);
        if (!order->qty_) {
            orderMemPool_.free(order);
            return;
        }

        cidOidMap_.at(order->clientId_)->at(order->clientOrderId_) = order;

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

    void MEOrderBook::match(MEOrder* order) {
        bool isBuy = Side::BID == order->side_;
        MEOrderAtPrice* bestOppo = isBuy ? asks_ : bids_;
        while (order->qty_ > 0) {
            if (bestOppo && isBuy ? order->price_ < bestOppo->price_ : order->price_ > bestOppo->price_) {
                break;
            }
            while (bestOppo && !bestOppo->orders_) {
                orderAtPriceMap_[bestOppo->price_] = nullptr;
                auto next = bestOppo->next_;
                orderAtPriceMemPool_.free(bestOppo);
                bestOppo = next;
            }
            if (!bestOppo) break;
            auto qty = std::min(order->qty_, bestOppo->orders_->qty_);
            bestOppo->orders_->qty_ -= qty;
            order->qty_ -= qty;

            std::cout << "matched " << (isBuy ? "BID" : "ASK") << " qty=" << qty 
                << " orderPrice=" << order->price_ << " bookPrice=" << bestOppo->price_ << std::endl;
        }
    }
}
