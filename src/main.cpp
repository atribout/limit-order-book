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
        std::cout << "[REJ] Order " << id << "rejected (Reason: " << (int)reason << ")\n";
    }

    void onTrade(uint64_t aggId, uint64_t passId, uint32_t qty, int32_t price) 
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

int main() 
{
    ConsoleListener l;

    OrderBook<ConsoleListener> book(l);

    std::cout << "Placing Sell Orders..." << std::endl;
    book.submitOrder(Order(1, 100, 10, Side::Sell)); // Sell 10 @ 100
    book.submitOrder(Order(2, 102, 20, Side::Sell)); // Sell 20 @ 102
    book.printBook();

    std::cout << "\n--- Incoming Aggressive Buy (25 @ 105) ---" << std::endl;
    book.submitOrder(Order(3, 105, 25, Side::Buy));

    std::cout << "\nFinal Book :" << std::endl;
    book.printBook();


    return 0;
}