#include "heap.hpp"
#include <cstdint>
#include <vector>
#include <memory>

using namespace std;

maxHeapNode::maxHeapNode(){}
maxHeapNode::maxHeapNode(double dist, unique_ptr<float[]> record) 
                        : dist(dist), record(move(record)){}

maxHeap::maxHeap(size_t k) : capacity(k)
{
    data.reserve(k);
}
maxHeapNode& maxHeap::top()
{
    return data[0];
}

size_t maxHeap::size()
{
    return data.size();
}

void maxHeap::siftUp(size_t i)
{
    auto node = move(data[i]);

    while (i > 0)
    {
        size_t p = (i - 1) >> 1;

        if (data[p].dist >= node.dist)
            break;

        data[i] = move(data[p]);
        i = p;
    }

    data[i] = move(node);
}

void maxHeap::siftDown(size_t i)
{
    size_t n = data.size();

    auto node = move(data[i]);

    while (true)
    {
        size_t l = (i << 1) + 1;

        if (l >= n)
            break;

        size_t r = l + 1;

        size_t best = (r < n && data[r].dist > data[l].dist) ? r : l;

        if (data[best].dist <= node.dist)
            break;

        data[i] = move(data[best]);
        i = best;
    }

    data[i] = move(node);
}

void maxHeap::insert(maxHeapNode&& node)
{
    if (data.size() < capacity)
    {
        data.push_back(move(node));
        siftUp(data.size() - 1);
    }
    else if (node.dist < data[0].dist)
    {
        data[0] = move(node);
        siftDown(0);
    }
}

