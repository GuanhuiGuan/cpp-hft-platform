#pragma once

#include "../../infra/types.h"
#include "../../infra/log/logger.h"
#include "../../infra/alloc.h"
#include "../../infra/spsc_lfq.h"

#include <array>

using namespace infra;

namespace exchange {

    struct MEOrder {
        InsId insId_ {InsId_NULL}; // instrument id
        UserId clientId_ {UserId_NULL};
        UserOrderId clientOrderId_ {UserOrderId_NULL}; // ID sent from client
        MarketOrderId marketOrderId_ {MarketOrderId_NULL}; // unique ID among the market

        Side side_ {Side::NONE};
        Price price_ {Price_NULL};
        Qty qty_ {Qty_NULL};
        Priority priority_ {Priority_NULL};

        MEOrder* prev_ {nullptr};
        MEOrder* next_ {nullptr};

        MEOrder() {}
        MEOrder(InsId insId, UserId cid, UserOrderId coid, MarketOrderId mid, Side side, Price price, Qty qty)
        : MEOrder(insId, cid, coid, mid, side, price, qty, Priority_NULL)
        {}
        MEOrder(InsId insId, UserId cid, UserOrderId coid, MarketOrderId mid, Side side, Price price, Qty qty, Priority priority)
        : insId_{insId}, clientId_{cid}, clientOrderId_{coid}, marketOrderId_{mid}
        , side_{side}, price_{price}, qty_{qty}, priority_{priority} 
        {}

        auto toStr() const -> std::string {
            std::stringstream ss;
            ss << "MEOrder["
            << "insId=" << strInsId(insId_) << ' '
            << "clientId=" << strUserId(clientId_) << ' '
            << "clientOrderId=" << strUserOrderId(clientOrderId_) << ' '
            << "marketOrderId=" << strMarketOrderId(marketOrderId_) << ' '
            << "side=" << strSide(side_) << ' '
            << "price=" << strPrice(price_) << ' '
            << "qty=" << strQty(qty_) << ' '
            << "priority=" << strPriority(priority_) << ']'
            ;
            return ss.str();
        }
    };

    struct MEOrderAtPrice {
        Side side_ {Side::NONE};
        Price price_ {Price_NULL};

        MEOrder* orders_ {nullptr};
        MEOrderAtPrice* prev_ {nullptr};
        MEOrderAtPrice* next_ {nullptr};

        MEOrderAtPrice() {}
        MEOrderAtPrice(Side s, Price p, MEOrder* firstOrder) : side_{s}, price_{p}, orders_{firstOrder} {}
    };

    using MEOrderAtPriceMap = std::array<MEOrderAtPrice*, MAX_PRICE_LVL>;

    using OrderMap = std::array<MEOrder*, MAX_ORDER_IDS>;

    using CidOidMap = std::array<OrderMap*, MAX_NUM_CLIENTS>;
}