#pragma once

#include "me_order.h"
#include "infra/log/logger.h"
#include "infra/alloc.h"

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

        auto priceToIdx(Price p) const {return p % orderAtPriceMap_.max_size();}
    };
}
