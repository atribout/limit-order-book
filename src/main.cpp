#include "Order.h"
#include "OrderBook.h"
#include "Listeners.h"

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