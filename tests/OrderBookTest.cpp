#include <gtest/gtest.h>
#include "OrderBook.h"
#include "Listeners.h"

class OrderBookTest : public ::testing::Test 
{
protected:
    VectorListener listener;
    std::unique_ptr<OrderBook<VectorListener>> book;

    void SetUp() override 
    {
        book = std::make_unique<OrderBook<VectorListener>>(listener);
    }
};

TEST_F(OrderBookTest, FullMatchExecution)
{
    book->submitOrder(Order(1, 100, 10, Side::Sell));
    book->submitOrder(Order(2, 100, 10, Side::Buy));

    ASSERT_EQ(listener.trades.size(), 1);

    auto trade = listener.trades[0];
    EXPECT_EQ(trade.qty, 10);
    EXPECT_EQ(trade.price, 100);
    EXPECT_EQ(trade.aggId, 2);
    EXPECT_EQ(trade.passId, 1);
}

TEST_F(OrderBookTest, PartialMatch)
{
    book->submitOrder(Order(1, 100, 20, Side::Buy));
    book->submitOrder(Order(2, 100, 5, Side::Sell));

    ASSERT_EQ(listener.trades.size(), 1);
    EXPECT_EQ(listener.trades[0].qty, 5);

    book->submitOrder(Order(3, 100, 10, Side::Sell));

    ASSERT_EQ(listener.trades.size(), 2);
    EXPECT_EQ(listener.trades[1].qty, 10);
    EXPECT_EQ(listener.trades[1].passId, 1);
}

TEST_F(OrderBookTest, SweepMultiLevel) 
{
    book->submitOrder(Order(1, 100, 10, Side::Sell));
    book->submitOrder(Order(2, 101, 10, Side::Sell));

    book->submitOrder(Order(3, 102, 15, Side::Buy));

    ASSERT_EQ(listener.trades.size(), 2);
    
    EXPECT_EQ(listener.trades[0].price, 100);
    EXPECT_EQ(listener.trades[0].qty, 10);
    
    EXPECT_EQ(listener.trades[1].price, 101);
    EXPECT_EQ(listener.trades[1].qty, 5);
}

TEST_F(OrderBookTest, RejectsInvalidOrders) 
{
    book->submitOrder(Order(10, 100, 0, Side::Buy));
    ASSERT_EQ(listener.rejectedIds.size(), 1);
    EXPECT_EQ(listener.rejectReasons[0], RejectReason::InvalidQuantity);

    book->submitOrder(Order(11, -50, 10, Side::Buy));
    ASSERT_EQ(listener.rejectedIds.size(), 2);
    EXPECT_EQ(listener.rejectReasons[1], RejectReason::InvalidPrice);
}

TEST_F(OrderBookTest, RejectsUnknownCancellation) 
{
    book->cancelOrder(999);

    ASSERT_EQ(listener.rejectedIds.size(), 1);
    EXPECT_EQ(listener.rejectedIds[0], 999);
    EXPECT_EQ(listener.rejectReasons[0], RejectReason::OrderNotFound);
}

TEST_F(OrderBookTest, TimePriorityFIFO) {
    book->submitOrder(Order(1, 100, 10, Side::Sell));
    book->submitOrder(Order(2, 100, 10, Side::Sell));

    book->submitOrder(Order(3, 100, 10, Side::Buy));

    ASSERT_EQ(listener.trades.size(), 1);
    EXPECT_EQ(listener.trades[0].passId, 1);
}