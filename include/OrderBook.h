#pragma once
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <memory>
#include <concepts>
#include "Order.h"

template<typename T>
concept TradeListenerConcept = requires(T t, uint64_t id, int32_t p, uint32_t q, Side s, RejectReason r) {
    { t.onTrade(id, id, p, q)};
    { t.onOrderAdded(id, p, q, s)};
    { t.onOrderCancelled(id)};
    { t.onOrderRejected(id, r)};
    { t.onOrderBookUpdate(p, q, s)};
};


template<TradeListenerConcept ListenerT>
class OrderBook{
private:

    struct Level
    {
        std::list<Order> orders;
        uint32_t totalVolume = 0;
    };

    std::map<int32_t, Level, std::greater<int32_t>> bids;
    std::map<int32_t, Level> asks;

    struct OrderLocation
    {
        typename std::list<Order>::iterator orderIt;
        Level* levelPtr;
        Side side;
        int32_t price;
        uint32_t quantity;
    };

    std::unordered_map<uint64_t, OrderLocation> orderLookup;

    ListenerT& listener;

public:

    OrderBook(ListenerT& l): listener(l) {};

    void submitOrder(Order order)
    {
        if (order.quantity <= 0)
        {
            listener.onOrderRejected(order.id, RejectReason::InvalidQuantity);
            return;
        }
        if (order.price <= 0)
        {
            listener.onOrderRejected(order.id, RejectReason::InvalidPrice);
            return;
        }

        if(orderLookup.find(order.id) != orderLookup.end())
        {
            listener.onOrderRejected(order.id, RejectReason::DuplicateId);
            return;
        }

        matchOrder(order);
        if(order.quantity > 0)
        {
            addOrder(order);
        }
    }

    void cancelOrder(uint64_t orderId)
    {
        auto it = orderLookup.find(orderId);
        if(it == orderLookup.end())
        {
            listener.onOrderRejected(orderId, RejectReason::OrderNotFound);
            return;
        }

        OrderLocation& loc = it->second;
        Level& level = *loc.levelPtr;

        level.totalVolume -= loc.quantity;
        listener.onOrderBookUpdate(loc.price, level.totalVolume, loc.side);
        level.orders.erase(loc.orderIt);

        if(level.orders.empty())
        {
            if(loc.side == Side::Buy)
            {
                bids.erase(loc.price);
            }
            else
            {
                asks.erase(loc.price);
            }
        }
        orderLookup.erase(orderId);
        listener.onOrderCancelled(orderId);
    }

    void printBook()
    {
        std::cout << "--- ASKS ---" << std::endl;
        for (auto it = asks.rbegin(); it != asks.rend(); ++it)
        {
            std::cout << it->first << "\t|" << it->second.totalVolume << std::endl;
        }

        std::cout << "--- BIDS ---"<< std::endl;
        for (const auto& [price, level] : bids)
        {
            std::cout << price << "\t|" << level.totalVolume << std::endl;
        }
        std::cout << "------------" << std::endl;
    }

private:
    
    void matchOrder(Order& incomingOrder)
    {
        while(incomingOrder.quantity>0)
        {
            if(incomingOrder.side == Side::Buy)
            {
                if(asks.empty()) break;

                auto bestAskIt = asks.begin();
                int32_t bestPrice = bestAskIt->first;

                if(incomingOrder.price >= bestPrice)
                {
                    Level& level = bestAskIt->second;
                    executeTrade(incomingOrder, level, bestPrice);
                }
                else
                {
                    break;
                }
            }
            else
            {
                if(bids.empty()) break;

                auto bestBidIt = bids.begin();
                int32_t bestPrice = bestBidIt->first;

                if(incomingOrder.price <= bestPrice)
                {
                    Level& level = bestBidIt->second;
                    executeTrade(incomingOrder, level, bestPrice);
                }
                else
                {
                    break;
                }
            }
            
        }

    }

    void executeTrade(Order& incoming, Level& level, int32_t price)
    {
        Order& order = *level.orders.begin();
        uint32_t qtyTraded = std::min(incoming.quantity, order.quantity);

        listener.onTrade(incoming.id, order.id, price, qtyTraded);

        incoming.quantity -= qtyTraded;

        if (qtyTraded == order.quantity)
        {
            cancelOrder(order.id);
        }
        else
        {
            order.quantity -= qtyTraded;
            orderLookup[order.id].quantity = order.quantity;
            level.totalVolume -= qtyTraded;
            listener.onOrderBookUpdate(price, level.totalVolume, order.side);
        }
    }

    void addOrder(const Order& order)
    {   
        Level& level = (order.side == Side::Buy) ? bids[order.price] : asks[order.price];

        level.orders.push_back(order);
        level.totalVolume += order.quantity;
        listener.onOrderBookUpdate(order.price, level.totalVolume, order.side);

        OrderLocation loc;
        loc.levelPtr = &level;
        loc.orderIt = std::prev(level.orders.end());
        loc.price = order.price;
        loc.side = order.side;
        loc.quantity = order.quantity;

        orderLookup[order.id] = loc;

        listener.onOrderAdded(order.id, order.price, order.quantity, order.side);
    }
};