#pragma once
#include <cstdint>
#include <iostream>

enum class Side {
    Buy,
    Sell
};

enum class OrderType {
    GoodTillCancel,
    FillAndKill
};

struct Order {
    uint64_t id;
    int32_t price;
    uint32_t quantity;
    Side side;

    Order(uint64_t id, int32_t price, uint32_t quantity, Side side)
        : id(id), price(price), quantity(quantity), side(side) {}

    friend std::ostream& operator<<(std::ostream& os, const Order& order) {
        return os << "Order[" << order.id << "] " 
                  << (order.side == Side::Buy ? "BUY" : "SELL") << " "
                  << order.quantity << " @ " << order.price;
    }
};