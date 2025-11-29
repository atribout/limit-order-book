#include <iostream>
#include "Order.h"
#include "OrderBook.h"

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
    struct TradeInfo { uint32_t qty; int32_t price; };
    std::vector<TradeInfo> trades;
    std::vector<uint64_t> cancelledIds;

    void onTrade(uint64_t, uint64_t, int32_t price, uint32_t qty) {
        trades.push_back({qty, price});
    }

    void onOrderCancelled(uint64_t id) {
        cancelledIds.push_back(id);
    }
    
    void onOrderAdded(uint64_t, int32_t, uint32_t, Side) {}
    void onOrderRejected(uint64_t, RejectReason) {}
    void onOrderBookUpdate(int32_t, uint32_t, Side) {}
};

int main() {
    ConsoleListener l;
    OrderBook<ConsoleListener> book(l);

    std::cout << "=== 1. SIMPLE MATCHING TEST ===\n";
    book.submitOrder(Order(1, 100, 10, Side::Sell)); // Sell 10 @ 100
    book.submitOrder(Order(2, 100, 10, Side::Buy));  // Buy 10 @ 100 -> FULL FILL
    // Expected: Trade 10@100, Empty book.

    std::cout << "\n=== 2. PARTIAL FILL TEST ===\n";
    book.submitOrder(Order(3, 100, 20, Side::Sell)); // Sell 20 @ 100
    book.submitOrder(Order(4, 100, 10, Side::Buy));  // Buy 10 @ 100 -> PARTIAL FILL
    // Expected: Trade 10@100, 10 remaining on Sell order #3.

    std::cout << "\n=== 3. MULTI-LEVEL SWEEP TEST ===\n";
    book.submitOrder(Order(5, 101, 10, Side::Sell)); // Add liquidity higher up
    // Book state: Sell 10@100 (remaining #3), Sell 10@101 (#5)
    
    book.submitOrder(Order(6, 102, 30, Side::Buy)); 
    // Expected: 
    // - Fills the 10 @ 100 (#3)
    // - Fills the 10 @ 101 (#5)
    // - 10 remaining @ 102 on Buy side (#6)

    std::cout << "\n=== 4. ERROR & DUPLICATE TEST ===\n";
    book.submitOrder(Order(6, 99, 10, Side::Buy)); // ID 6 already exists! -> REJECT DUPLICATE
    book.submitOrder(Order(7, -50, 10, Side::Buy)); // Negative price -> REJECT INVALID PRICE
    book.submitOrder(Order(8, 100, 0, Side::Sell)); // Null quantity -> REJECT INVALID QUANTITY
    
    std::cout << "\n=== 5. CANCELLATION TEST ===\n";
    book.cancelOrder(6); // Cancel the remainder of order #6
    book.cancelOrder(999); // Unknown ID -> REJECT ORDER NOT FOUND

    std::cout << "\n=== FINAL STATE ===\n";
    book.printBook(); 
    // Expected: Empty book (everything filled or cancelled).

    return 0;
}