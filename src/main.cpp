#include <iostream>
#include "Order.h"
#include "OrderBook.h"

struct consoleListener 
{
    void onTrade(uint64_t aggId, uint64_t passId, uint32_t qty, int32_t price) 
    {
                std::cout << ">>> TRADE EXECUTE: " << qty << "@" << price
                  << " (Aggressor: " << aggId<< ", Passive: " << passId << ")"
                  << std::endl;
    }

    void onOrderAdded(uint64_t id, int32_t price, uint32_t qty, Side side) 
    {
        std::cout << "[ORDER] New " << (side == Side::Buy ? "Buy" : "Sell") 
                  << " " << qty << " @ " << price << " (ID: " << id << ")\n";
    }

    void onOrderCancelled(uint64_t id) 
    {
        std::cout << "[CANCEL] Order " << id << " removed\n";
    }
};

int main() 
{
    consoleListener logger;

    OrderBook book(logger);

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