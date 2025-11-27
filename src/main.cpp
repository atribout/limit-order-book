#include <iostream>
#include "Order.h"
#include "OrderBook.h"

int main() {
    OrderBook book;

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