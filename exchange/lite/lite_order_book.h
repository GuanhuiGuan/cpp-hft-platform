#include <cstdint>
#include <unordered_map>
#include <map>
#include <list>
#include <iostream>
#include <cstdio>

namespace match {
    enum class Side {
        BID,
        ASK,
    };
    std::ostream& operator<<(std::ostream& os, Side side) {
        return os << ((Side::BID == side) ? "BID" : "ASK");
    }

    struct Order {
        uint64_t id_;
        uint64_t price_;
        uint64_t qty_;
        Side side_;
        Order(uint64_t id, uint64_t p,  uint64_t q, Side s) : id_{id}, price_{p}, qty_{q}, side_{s} {}
    };
    std::ostream& operator<<(std::ostream& os, const Order& order) {
        return os << "[id=" << order.id_ << " price=" << order.price_ << " qty=" << order.qty_ << " side=" << order.side_ << "]";
    }
    struct PriceLvl {
        uint64_t price_;
        std::list<Order*> orders_;
        PriceLvl(uint64_t p) : price_{p} {}
    };
    std::ostream& operator<<(std::ostream& os, const PriceLvl& pl) {
        os << pl.price_ << "{";
        for (const auto& o: pl.orders_) os << *o;
        return os << "}";
    }

    class OrderBook {
        std::unordered_map<uint64_t, Order*> orderMap_;
        std::list<PriceLvl> bids_; // desc
        std::list<PriceLvl> asks_; // asc
    public:
        ~OrderBook() {
            destroyBook(bids_);
            destroyBook(asks_);
        }
        
        auto addOrder(uint64_t id, Side side, uint64_t price, uint64_t qty) -> void {
            Order* order = new Order(id, price, qty, side);
            match(order);
            if (order->qty_ == 0) return;
            // add to corresponding side
            bool isBid = Side::BID == side;
            std::list<PriceLvl>& best = isBid ? bids_ : asks_; // NOTICE! use reference!!!
            auto it = best.begin();
            for (; it != best.end() && (isBid ? it->price_ > order->price_ : it->price_ < order->price_); ++it);
            if (it == best.end() || it->price_ != price) {
                it = best.emplace(it, price);
            }
            it->orders_.push_back(order);
            orderMap_.emplace(order->id_, order);
        }

        auto modOrder(uint64_t id, uint64_t qty) -> bool {
            auto it = orderMap_.find(id);
            if (it == orderMap_.end()) return false;
            it->second->qty_ = qty;
            return true;
        }

        auto cancelOrder(uint64_t id) -> bool {
            auto it = orderMap_.find(id);
            if (it != orderMap_.end()) {
                orderMap_.erase(it);
                return true;
            }
            return false;
        }

        auto printTopOfBook() const -> void {
            std::printf("#####\nBIDS\n");
            logBook(bids_);
            std::printf("ASKS\n");
            logBook(asks_);
        }
    private:
        auto match(Order* order) -> void {
            bool isBid = Side::BID == order->side_;
            std::list<PriceLvl>& oppo = isBid ? asks_ : bids_;
            while (order->qty_ > 0 && !oppo.empty()) {
                // only match with top price
                PriceLvl topPrice = oppo.front();
                if (isBid ? order->price_ < topPrice.price_ : order->price_ > topPrice.price_) break;
                // within 1 price lvl
                while (order->qty_ > 0 && !topPrice.orders_.empty()) {
                    Order* tpOrder = topPrice.orders_.front(); // always compare with first order
                    // if cancelled
                    if (orderMap_.find(tpOrder->id_) == orderMap_.end()) {
                        tpOrder->qty_ = 0;
                    }
                    auto qty = std::min(order->qty_, tpOrder->qty_);
                    if (qty > 0) {
                        order->qty_ -= qty;
                        tpOrder->qty_ -= qty;
                        logMatch(qty, order->price_, tpOrder->price_, order->side_);
                    }
                    if (tpOrder->qty_ == 0) {
                        orderMap_.erase(tpOrder->id_);
                        delete tpOrder;
                        topPrice.orders_.pop_front();
                    }
                }

                if (topPrice.orders_.empty()) {
                    // empty price lvl
                    oppo.pop_front();
                }
            }
        }
        auto logMatch(uint64_t qty, uint64_t orderPrice, uint64_t bookPrice, Side side) const -> void {
            std::printf("Matched event qty=%d orderPrice=%d bookPrice=%d side=%s\n", 
                qty, orderPrice, bookPrice, Side::BID == side ? "BID" : "ASK");
        }
        auto logBook(const std::list<PriceLvl>& book) const -> void {
            for (const auto& pl : book) {
                std::cout << pl << std::endl;
            }
        }
        auto destroyBook(const std::list<PriceLvl>& book) -> void {
            for (auto& pl: book) {
                for (auto& order: pl.orders_) {
                    delete order;
                }
            }
        }
    };
}
