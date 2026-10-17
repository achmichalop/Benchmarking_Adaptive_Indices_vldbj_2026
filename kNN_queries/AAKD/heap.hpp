#ifndef HEAP_H
#define HEAP_H

#include <cstdint>
#include <vector>
// #include <memory>


template <size_t Dim>
class maxHeapNode
{
public:
    double dist;
    float point[Dim];
    
    maxHeapNode(){}
    maxHeapNode(double dist, float *rec)
    {
        this->dist = dist;
        std::copy(rec, rec + Dim, point);
    }
};

template <size_t Dim>
class maxHeap
{
public:
    std::vector<maxHeapNode<Dim>> data;
    uint32_t capacity;

    maxHeap(size_t k)
    {
        capacity = k;
        data.reserve(k);
    }

    maxHeapNode<Dim>& top()
    {
        return data[0];
    }

    size_t size()
    {
        return data.size();
    }

    void siftUp(size_t i)
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

    void siftDown(size_t i)
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

    void insert(maxHeapNode<Dim> node)
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
};

#endif // HEAP_H
