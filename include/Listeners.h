#include <cstdint>
#include <iostream>
#include <vector>
#include "Order.h"

struct ConsoleListener 
{
    void onOrderAdded(uint64_t id, int32_t price, uint32_t qty, Side side) 
    {
        std::cout << "[ORDER] New " << (side == Side::Buy ? "Buy" : "Sell") 
                  << " " << qty << " @ " << price << " (ID: " << id << ")\n";
    }

    void onOrderCancelled(uint64_t id) 
    {
        std::cout << "[CANCEL] Order " << id << " removed\n";
    }

    void onOrderRejected(uint64_t id, RejectReason reason)
    {
        std::cout << "[REJ] Order " << id << " rejected (Reason: " << (int)reason << ")\n";
    }

    void onTrade(uint64_t aggId, uint64_t passId, int32_t price, uint32_t qty) 
    {
        std::cout << ">>> TRADE EXECUTE: " << qty << "@" << price
            << " (Aggressor: " << aggId<< ", Passive: " << passId << ")\n";
    }

    // --- PUBLIC FLOW ---
    void onOrderBookUpdate(int32_t price, uint32_t volume, Side side)
    {
        std::cout << "[MKT DATA] Price Level " << price << " ("
                  << (side == Side::Buy ? "Bid" : "Ask")
                  << ") is now " << volume << "\n";
    }
};

struct EmptyListener
{
    void onOrderAdded(uint64_t id, int32_t price, uint32_t qty, Side side) {}

    void onOrderCancelled(uint64_t id) {}

    void onOrderRejected(uint64_t id, RejectReason reason) {}

    void onTrade(uint64_t aggId, uint64_t passId, int32_t price, uint32_t qty) {}

    void onOrderBookUpdate(int32_t price, uint32_t volume, Side side) {}
};

struct VectorListener {
    struct TradeInfo 
    { 
        uint64_t aggId;
        uint64_t passId;
        int32_t price;
        uint32_t qty; 
    };

    std::vector<TradeInfo> trades;
    std::vector<uint64_t> cancelledIds;
    std::vector<uint64_t> rejectedIds;
    std::vector<RejectReason> rejectReasons;

    void onTrade(uint64_t aggId, uint64_t passId, int32_t price, uint32_t qty) 
    {
        trades.push_back({aggId, passId, price, qty});
    }

    void onOrderCancelled(uint64_t id) 
    {
        cancelledIds.push_back(id);
    }

    void onOrderRejected(uint64_t id, RejectReason reason) 
    {
        rejectedIds.push_back(id);
        rejectReasons.push_back(reason);
    }
    
    void onOrderAdded(uint64_t, int32_t, uint32_t, Side) {}
    void onOrderBookUpdate(int32_t, uint32_t, Side) {}

    void clear() 
    {
        trades.clear();
        cancelledIds.clear();
        rejectedIds.clear();
        rejectReasons.clear();
    }
};