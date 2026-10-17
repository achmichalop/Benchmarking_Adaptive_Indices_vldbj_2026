#include "../basics.h"
#include "heap.hpp"

array<size_t, NUMDIMS> strides;
size_t totalCells = 0;

size_t getStartOfCell(ELEMTYPE start, ELEMTYPE extent)
{
    return (size_t) (start / extent + EPS);
}

size_t getCellID(array<size_t, NUMDIMS> &cellIdx)
{

    size_t cellId = 0;
    for (size_t d = 0; d < NUMDIMS; d++)
    {
        cellId += cellIdx[d] * strides[d];
    }

    return cellId;
}

void partitionGrid(size_t numPartitionsPerDimension, ELEMTYPE** &index, size_t* &indexSize)
{
    array<size_t, NUMDIMS> cellIdx;
    float partitionExtent = 1.0f/numPartitionsPerDimension;
    size_t cellFilledPointer[totalCells];

    index = (ELEMTYPE **) malloc(totalCells * sizeof(ELEMTYPE *));
    indexSize = (size_t *) malloc(totalCells * sizeof(size_t));


    memset(cellFilledPointer, 0, totalCells * sizeof(size_t));
    memset(indexSize, 0, totalCells * sizeof(size_t));

    strides[0] = 1;
    for (size_t d = 1; d < NUMDIMS; d++)
    {
        strides[d] = strides[d-1] * numPartitionsPerDimension;
    }

    for (size_t i = 0; i < DATA_COUNT; i++)
    {
        for (size_t d = 0; d < NUMDIMS; d++)
        {
            cellIdx[d] = getStartOfCell(m_data_arr[i][d] + EPS, partitionExtent);

            if (cellIdx[d] == numPartitionsPerDimension)
                cellIdx[d] = numPartitionsPerDimension - 1;
        }

        size_t cellId = getCellID(cellIdx);
        indexSize[cellId]++;
    }

    for (size_t i = 0; i < totalCells; i++)
    {
        index[i] = (ELEMTYPE *) malloc(indexSize[i] * NUMDIMS * sizeof(ELEMTYPE));
    }

    for (size_t i = 0; i < DATA_COUNT; i++)
    {
        for (size_t d = 0; d < NUMDIMS; d++)
        {
            cellIdx[d] = getStartOfCell(m_data_arr[i][d] + EPS, partitionExtent);

            if (cellIdx[d] == numPartitionsPerDimension)
                cellIdx[d] = numPartitionsPerDimension - 1;
        }

        size_t cellId = getCellID(cellIdx);

        for (size_t d = 0; d < NUMDIMS; d++)
            index[cellId][cellFilledPointer[cellId]*NUMDIMS+d] = m_data_arr[i][d];
        
        cellFilledPointer[cellId]++;
    }
}

void loadData(string data_file_name)
{
    ifstream data_file(data_file_name.c_str());
    if (!data_file.is_open())
    {
        cout << "Cannot find data file\n";
        exit(1);
    }

    for (size_t i = 0; i < DATA_COUNT; i++)
    {
        for (size_t d = 0; d < NUMDIMS; d++)
            data_file >> m_data_arr[i][d];
        m_data_ids[i] = i;
    }

    data_file.close();
}

void loadQueries(string query_file_name, size_t query_size, Point* &queries)
{
    queries = (Point *) malloc(query_size * sizeof(Point));
    
    ifstream query_file(query_file_name.c_str());
    if(!query_file.is_open())
    {
        cout << "Cannot find query file\n";
        exit(1);
    }

    for(size_t i = 0; i < query_size; i++){
        queries[i] = Point();
        for(size_t d = 0; d < NUMDIMS; d++)
        {
            query_file >> queries[i].m_values[d];
        }
    }

    query_file.close();
}

void initVisitedList(vector<uint64_t>& list, size_t totalCells)
{
    list.resize((totalCells+63)/64, 0);
}

void setVisitedCell(size_t idx, vector<uint64_t> &visitedList)
{
    visitedList[idx >> 6] |= (1ULL << (idx & 63));
}

bool isVisitedCell(size_t idx, vector<uint64_t> &visitedList)
{
    return (visitedList[idx >> 6] & (1ULL << (idx & 63))) != 0;
}

double PointToPointDistance(Point& q, ELEMTYPE* pt)
{
    double dist = 0.0;
    for (size_t d = 0; d < NUMDIMS; d++)
    {
        double diff = q.m_values[d] - pt[d];
        dist += diff * diff;
    }

    return dist;
}

double PointToCellDistance(Point& q, array<size_t, NUMDIMS> c, float extent)
{
    double dist = 0.0;
    
    for (size_t d = 0; d < NUMDIMS; d++)
    {
        ELEMTYPE minB = c[d] * extent;
        ELEMTYPE maxB = minB + extent;

        ELEMTYPE val = q.m_values[d];

        if (val < minB)
        {
            double diff = minB - val;
            dist += diff * diff;
        }
        else if (val > maxB)
        {
            double diff = val - maxB;
            dist += diff * diff;
        }
    }

    return dist;
}


void update_neighbors_heap(maxHeap<NUMDIMS> &Q, Point query, double &worstDist, ELEMTYPE* cell, size_t cellSize)
{
    for (size_t i = 0; i < cellSize; i++)
    {
        size_t offset = i * NUMDIMS;
        double dist = PointToPointDistance(query, &cell[offset]);
        Q.insert(maxHeapNode<NUMDIMS>(dist, &cell[offset]));
    }

    if (Q.size() == Q.capacity)
        worstDist = Q.top().dist;
}


maxHeap<NUMDIMS> kNN_query(ELEMTYPE** &index, size_t* &indexSize, Point query, size_t k, size_t numPartitionsPerDimension)
{
    minHeap<NUMDIMS> H;
    maxHeap<NUMDIMS> Q(k);
    vector<uint64_t> visitedList;
    array<size_t, NUMDIMS> cellIdx;
    double worstDist = numeric_limits<double>::max();
    float partitionExtent = 1.0f/numPartitionsPerDimension;
    size_t numExaminedCells = pow(3, NUMDIMS);
    vector<array<size_t, NUMDIMS>> examinedCellsIds;
    vector<bool> examinedCellsFlags;

    examinedCellsFlags.resize(numExaminedCells, true);
    examinedCellsIds.resize(numExaminedCells);
    
    initVisitedList(visitedList, totalCells);

    for (size_t d = 0; d < NUMDIMS; d++)
    {
        cellIdx[d] = getStartOfCell(query.m_values[d] + EPS, partitionExtent); 
    }

    size_t Cq = getCellID(cellIdx);
    setVisitedCell(Cq, visitedList);
    H.push({Cq, 0.0});

    while(!H.empty())
    {
        auto currCell = H.top();
        H.pop();

        if (Q.size() == Q.capacity && currCell.dist >= worstDist)
            break;

        size_t cellId = currCell.idx;
        
        update_neighbors_heap(Q, query, worstDist, index[cellId], indexSize[cellId]);

        array<int, NUMDIMS> startIdx;
        for (size_t d = 0; d < NUMDIMS; d++)
        {
            startIdx[d] = (int)((cellId / strides[d]) % numPartitionsPerDimension) - 1;
        }

        for (size_t i = 0; i < numExaminedCells; i++)
        {
            size_t iters = i;

            for (size_t d = 0; d < NUMDIMS; d++)
            {
                int idx = startIdx[d] + iters % 3;
                iters /= 3;

                if (idx < 0 || idx >= (int)numPartitionsPerDimension)
                {
                    examinedCellsFlags[i] = false;
                    continue;
                }

                examinedCellsIds[i][d] = (size_t)idx;
            }
        }

        examinedCellsFlags[numExaminedCells >> 1] = false;

        for (size_t i = 0; i < numExaminedCells; i++)
        {
            if (!examinedCellsFlags[i])
                continue;

            cellId = getCellID(examinedCellsIds[i]);

            if (isVisitedCell(cellId, visitedList))
                    continue;

            setVisitedCell(cellId, visitedList);
                
            double dist = PointToCellDistance(query, examinedCellsIds[i], partitionExtent);

            H.push({cellId, dist});
        }

        fill(examinedCellsFlags.begin(), examinedCellsFlags.end(), true);

    }

    return Q;
}

int main(int argc, char **argv)
{
    string data_file_name = argv[1];
    size_t query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string output_file_name = argv[4];
    size_t numPartitionsPerDimension = stoi(argv[5]);
    size_t k_neighbors = stoi(argv[6]);
    ELEMTYPE **index = nullptr;
    size_t *indexSize = nullptr;
    Point *queries = nullptr;

    loadData(data_file_name);
    loadQueries(query_file_name, query_size, queries);

    totalCells = numPartitionsPerDimension * numPartitionsPerDimension;

    if constexpr (NUMDIMS != 2)
    {
        numPartitionsPerDimension = ceil(pow(totalCells, 1 / (double)NUMDIMS));
        totalCells = pow(numPartitionsPerDimension,NUMDIMS);
    }

    auto start_indexing = clock();
    partitionGrid(numPartitionsPerDimension, index, indexSize);
    auto end_indexing = clock() - start_indexing;

    ofstream output_file;
    output_file.open(output_file_name.c_str());

    float partitionExtent = 1.0f/numPartitionsPerDimension;


    for (size_t i = 0; i < query_size; i++)
    {
        auto start_querying = clock();
        maxHeap<NUMDIMS> results = kNN_query(index, indexSize, queries[i], k_neighbors, numPartitionsPerDimension);
        auto end_querying = clock() - start_querying;

        output_file << results.size() << "\t" << end_querying / (double)CLOCKS_PER_SEC << "\t" << end_indexing / (double)CLOCKS_PER_SEC << endl;
    }

    output_file.close();
    return 0;
}