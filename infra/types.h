#pragma once

#include <string>
#include <sstream>
#include <limits>

namespace infra {

    constexpr size_t MAX_PRICE_LVL = 256;
    constexpr size_t MAX_ORDER_IDS = 1024 * 1024;
    constexpr size_t MAX_NUM_CLIENTS = 256;

    enum class Side : int8_t {
        NONE = 0,
        BID = 1,
        ASK = -1,
    };
    inline auto strSide(Side s) -> std::string {
        switch (s) {
            case Side::BID: return "BID";
            case Side::ASK: return "ASK";
            default: return "NONE";
        }
    }

    using Price = int64_t;
    constexpr auto Price_NULL = std::numeric_limits<Price>::max();
    inline auto strPrice(Price x) -> std::string {
        [[unlikely]] if (Price_NULL == x) return "NULL";
        return std::to_string(x);
    }

    using Qty = uint64_t;
    constexpr auto Qty_NULL = std::numeric_limits<Qty>::max();
    inline auto strQty(Qty x) -> std::string {
        [[unlikely]] if (Qty_NULL == x) return "NULL";
        return std::to_string(x);
    }

    using Priority = uint64_t;
    constexpr auto Priority_NULL = std::numeric_limits<Priority>::max();
    inline auto strPriority(Priority x) -> std::string {
        [[unlikely]] if (Priority_NULL == x) return "NULL";
        return std::to_string(x);
    }

    using InsId = uint64_t;
    constexpr auto InsId_NULL = std::numeric_limits<InsId>::max();
    inline auto strInsId(InsId x) -> std::string {
        [[unlikely]] if (InsId_NULL == x) return "NULL";
        return std::to_string(x);
    }

    using UserId = uint64_t;
    constexpr auto UserId_NULL = std::numeric_limits<UserId>::max();
    inline auto strUserId(UserId x) -> std::string {
        [[unlikely]] if (UserId_NULL == x) return "NULL";
        return std::to_string(x);
    }

    using UserOrderId = uint64_t;
    constexpr auto UserOrderId_NULL = std::numeric_limits<UserOrderId>::max();
    inline auto strUserOrderId(UserOrderId x) -> std::string {
        [[unlikely]] if (UserOrderId_NULL == x) return "NULL";
        return std::to_string(x);
    }

    using MarketOrderId = uint64_t;
    constexpr auto MarketOrderId_NULL = std::numeric_limits<MarketOrderId>::max();
    inline auto strMarketOrderId(MarketOrderId x) -> std::string {
        [[unlikely]] if (MarketOrderId_NULL == x) return "NULL";
        return std::to_string(x);
    }

}