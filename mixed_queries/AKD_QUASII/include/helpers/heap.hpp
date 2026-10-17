#ifndef HEAP_H
#define HEAP_H

#include <cstdint>
#include <vector>
#include <memory>

class maxHeapNode
{
public:
    double dist;
    std::unique_ptr<float[]> record;

    maxHeapNode();
    maxHeapNode(double dist, std::unique_ptr<float[]> record);
};


class maxHeap
{
public:
    std::vector<maxHeapNode> data;
    uint32_t capacity;

    maxHeap(size_t k);

    maxHeapNode& top();
    size_t size();
    void siftUp(size_t i);
    void siftDown(size_t i);
    void insert(maxHeapNode&& node);   
};

#endif // HEAP_H
