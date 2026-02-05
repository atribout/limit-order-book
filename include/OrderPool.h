#pragma once
#include <vector>
#include <cstdint>
#include "Order.h"

class OrderPool {
private:
    std::vector<Order> store;
    std::vector<int32_t> freeList;

public:
    explicit OrderPool(size_t size)
    {
        store.resize(size);
        for(int32_t i = size-1; i>=0; --i)
        {
            freeList.push_back(i);
        }
    }

    template<typename... Args>
    int32_t allocate(Args&&... args)
    {
        if (freeList.empty()) return -1;

        int32_t idx = freeList.back();
        freeList.pop_back();

        new (&store[idx]) Order(std::forward<Args>(args)...);

        store[idx].prev = -1;
        store[idx].next = -1;

        return idx;
    }

    void deallocate(int32_t idx)
    {
        freeList.push_back(idx);
    }

    Order& get(int32_t idx)
    {
        return store[idx];
    }
};