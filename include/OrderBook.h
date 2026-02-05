#pragma once
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <memory>
#include <concepts>
#include "Order.h"
#include "OrderPool.h"

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
        int32_t head = -1;
        int32_t tail = -1;
        uint32_t totalVolume = 0;
    };

    OrderPool pool;

    // We assume prices between 0 and 100,000.
    static constexpr size_t MAX_PRICE = 100000;
    std::vector<Level> bids;
    std::vector<Level> asks;

    std::vector<int32_t> orderIndexLookup;

    int32_t minAskPrice = MAX_PRICE;
    int32_t maxBidPrice = 0;

    ListenerT& listener;

public:

    OrderBook(ListenerT& l):
          listener(l), 
          pool(1000000),
          bids(MAX_PRICE + 1),
          asks(MAX_PRICE + 1),
          orderIndexLookup(10000000, -1)
    {};

    void submitOrder(Order order)
    {
        if (order.quantity <= 0) [[unlikely]]
        {
            listener.onOrderRejected(order.id, RejectReason::InvalidQuantity);
            return;
        }
        if (order.price <= 0 || order.price > MAX_PRICE) [[unlikely]]
        {
            listener.onOrderRejected(order.id, RejectReason::InvalidPrice);
            return;
        }

        if (order.id >= orderIndexLookup.size()) [[unlikely]]
        {
            return;
        }

        if(orderIndexLookup[order.id] != -1) [[unlikely]]
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
        if (orderId >= orderIndexLookup.size()) [[unlikely]]
        {
            return;
        }
        int32_t idx = orderIndexLookup[orderId];
        if (idx == -1) [[unlikely]]
        {
            listener.onOrderRejected(orderId, RejectReason::OrderNotFound);
            return;
        }

        Order& order = pool.get(idx);
        std::vector<Level>& bookSide = (order.side == Side::Buy) ? bids: asks;
        Level& level = bookSide[order.price];

        level.totalVolume -= order.quantity;
        listener.onOrderBookUpdate(order.price, level.totalVolume, order.side);

        if(order.prev != -1)
        {
            pool.get(order.prev).next = order.next;
        }
        else
        {
            level.head = order.next;
        }
        
        if(order.next != -1)
        {
            pool.get(order.next).prev = order.prev;
        }
        else
        {
            level.tail = order.prev;
        }

        pool.deallocate(idx);
        orderIndexLookup[orderId] = -1;
        listener.onOrderCancelled(orderId);
    }

    void printBook()
    {
        std::cout << "--- ASKS ---" << std::endl;
        for (int32_t p = MAX_PRICE; p >= 0; --p)
        {
            if (asks[p].totalVolume > 0)
            {
                std::cout << p << "\t|" << asks[p].totalVolume << std::endl;
            }
        }

        std::cout << "--- BIDS ---"<< std::endl;
        for (int32_t p = MAX_PRICE; p>=0; --p)
        {
            if (bids[p].totalVolume > 0)
            {
                std::cout << p << "\t|" << bids[p].totalVolume << std::endl;
            }
        }
        std::cout << "------------" << std::endl;
    }

private:
    
    void matchOrder(Order& incomingOrder)
    {
        if(incomingOrder.side == Side::Buy)
        {
            while(incomingOrder.quantity > 0 && incomingOrder.price >= minAskPrice)
            {
                if (minAskPrice > (int32_t)MAX_PRICE) [[unlikely]] break;
                Level& level = asks[minAskPrice];

                if(level.head == -1)
                {
                    minAskPrice++;
                    continue;
                }
                matchWithLevel(incomingOrder, level, minAskPrice);

                if(level.head == -1)
                {
                    minAskPrice++;
                }
            }
        }
        else
        {
            while(incomingOrder.quantity > 0 && incomingOrder.price <= maxBidPrice)
            {
                if (maxBidPrice < 0) [[unlikely]] break;
                Level& level = bids[maxBidPrice];

                if(level.head == -1)
                {
                    maxBidPrice--;
                    continue;
                }
                matchWithLevel(incomingOrder, level, maxBidPrice);

                if(level.head == -1)
                {
                    maxBidPrice--;
                }
            }
        }
    }

    void matchWithLevel(Order& incoming, Level& level, int32_t price)
    {
        uint32_t qtyTraded;
        while (level.head != -1 && incoming.quantity > 0)
        {
            int32_t currIdx = level.head;
            Order& order = pool.get(currIdx);

            qtyTraded = std::min(incoming.quantity, order.quantity);

            incoming.quantity -= qtyTraded;
            order.quantity -= qtyTraded;
            level.totalVolume -= qtyTraded;

            listener.onTrade(incoming.id, order.id, price, qtyTraded);
            listener.onOrderBookUpdate(price, level.totalVolume, order.side);
        
            if (order.quantity == 0)
            {
                int32_t nextIdx = order.next;
                level.head = nextIdx;

                if (nextIdx != -1)
                {
                    pool.get(nextIdx).prev = -1;
                }
                else
                {
                    level.tail = -1;
                }
                orderIndexLookup[order.id] = -1;
                pool.deallocate(currIdx);
            }
        }
    }

    void addOrder(const Order& order)
    {   
        int32_t idx = pool.allocate(order.id, order.price, order.quantity, order.side);

        if (idx == -1) [[unlikely]]
        {
            listener.onOrderRejected(order.id, RejectReason::SystemFull); 
            return;
        }
        
        std::vector<Level>& bookSide = (order.side == Side::Buy) ? bids: asks;
        Level& level = bookSide[order.price];

        if (level.head == -1)
        {
            level.head = idx;
            level.tail = idx;
        }
        else
        {
            pool.get(level.tail).next = idx;
            pool.get(idx).prev = level.tail;
            level.tail = idx;
        }

        level.totalVolume += order.quantity;
        orderIndexLookup[order.id] = idx;

        if(order.side == Side::Buy)
        {
            if(order.price > maxBidPrice)
            {
                maxBidPrice = order.price;
            }
        }
        else
        {
            if (order.price < minAskPrice)
            {
                minAskPrice = order.price;
            }
        }

        listener.onOrderAdded(order.id, order.price, order.quantity, order.side);
        listener.onOrderBookUpdate(order.price, level.totalVolume, order.side);
    }
};