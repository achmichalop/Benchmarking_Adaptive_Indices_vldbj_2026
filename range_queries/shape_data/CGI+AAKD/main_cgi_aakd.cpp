#include "../basics.h"
#include "AAKD.h"

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

void partitionGrid(size_t numPartitionsPerDimension, Rect** &index, size_t* &indexSize)
{
    array<size_t, NUMDIMS> cellIdx;
    float partitionExtent = 1.0f/numPartitionsPerDimension;
    size_t cellFilledPointer[totalCells];

    index = (Rect **) malloc(totalCells * sizeof(Rect *));
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
            cellIdx[d] = getStartOfCell(m_data_arr_rects[i].m_min[d] + EPS, partitionExtent);

            if (cellIdx[d] == numPartitionsPerDimension)
                cellIdx[d] = numPartitionsPerDimension - 1;
        }

        size_t cellId = getCellID(cellIdx);
        indexSize[cellId]++;
    }

    for (size_t i = 0; i < totalCells; i++)
    {
        index[i] = (Rect *) malloc(indexSize[i] * sizeof(Rect));
    }

    for (size_t i = 0; i < DATA_COUNT; i++)
    {
        for (size_t d = 0; d < NUMDIMS; d++)
        {
            cellIdx[d] = getStartOfCell(m_data_arr_rects[i].m_min[d] + EPS, partitionExtent);

            if (cellIdx[d] == numPartitionsPerDimension)
                cellIdx[d] = numPartitionsPerDimension - 1;
        }

        size_t cellId = getCellID(cellIdx);

        index[cellId][cellFilledPointer[cellId]] = m_data_arr_rects[i];
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
        m_data_arr_rects[i] = Rect();
        for (size_t d = 0; d < NUMDIMS; d++)
            data_file >> m_data_arr_rects[i].m_min[d];

        for (size_t d = 0; d < NUMDIMS; d++)
            data_file >> m_data_arr_rects[i].m_max[d];
        m_data_ids[i] = i;
    }

    data_file.close();
}

void loadQueries(string query_file_name, size_t query_size, Rect* &queries)
{
    queries = (Rect *) malloc(query_size * sizeof(Rect));
    
    ifstream query_file(query_file_name.c_str());
    if(!query_file.is_open())
    {
        cout << "Cannot find query file\n";
        exit(1);
    }

    for(size_t i = 0; i < query_size; i++){
        queries[i] = Rect();
        for(size_t d = 0; d < NUMDIMS; d++) query_file >> queries[i].m_min[d];
        for(size_t d = 0; d < NUMDIMS; d++) 
        {
            query_file >> queries[i].m_max[d];

            if (queries[i].m_max[d] > 1.0f)
                queries[i].m_max[d] = 1.0f;
        }    
    }

    query_file.close();
}


size_t range_query(Rect** &index, size_t* &indexSize, AAKDTree* &cellIndices, Rect query, Rect extendedQuery, size_t numPartitionsPerDimension, Rect rootCover)
{
    size_t query_res = 0;
    size_t numExaminedCells = 1;
    float partitionExtent = 1.0f/numPartitionsPerDimension;

    array<size_t, NUMDIMS> startCell{}, endCell{};
    vector<array<size_t, NUMDIMS>> examinedCellsIds;
    vector<bool> examinedCellsFlags;

    for (size_t d = 0; d < NUMDIMS; d++)
    {
        startCell[d] = getStartOfCell(extendedQuery.m_min[d] + EPS, partitionExtent);

        if (startCell[d] == numPartitionsPerDimension)
            startCell[d] = numPartitionsPerDimension-1;
        
        endCell[d] = getStartOfCell(extendedQuery.m_max[d] + EPS, partitionExtent);
        
        if (endCell[d] == numPartitionsPerDimension)
            endCell[d] = numPartitionsPerDimension-1;

        numExaminedCells *= (endCell[d] - startCell[d] + 1);
    }

    examinedCellsIds.resize(numExaminedCells);
    examinedCellsFlags.resize(numExaminedCells, true);


    for (size_t i = 0; i < numExaminedCells; i++)
    {
        size_t iters = i;
        for (size_t d = 0; d < NUMDIMS; d++)
        {
            size_t extent = endCell[d] - startCell[d] + 1;
            size_t idx = startCell[d] + iters % extent;
            iters /= extent;
 
            examinedCellsIds[i][d] = idx;
            if (idx == startCell[d] || idx == endCell[d])
                examinedCellsFlags[i] = false;
        }
    }

    for (size_t i = 0; i < numExaminedCells; i++)
    {
        size_t cellId = getCellID(examinedCellsIds[i]);

        if (examinedCellsFlags[i])     // covered by
        {
            for (size_t ii = 0; ii < indexSize[cellId]; ii++)
            {
                if (Overlap(query, index[cellId][ii]))
                    query_res++;
            }
        }
        else
        {
            query_res += cellIndices[cellId].AdaptiveQuery(extendedQuery, query, rootCover);
        }
    }

    return query_res;
}

int main(int argc, char **argv)
{
    string data_file_name = argv[1];
    size_t query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string extended_query_file_name = argv[4];
    string output_file_name = argv[5];
    size_t numPartitionsPerDimension;
    Rect **gridIndex = nullptr;
    size_t *gridIndexSize = nullptr;
    Rect *queries = nullptr;
    Rect *extended_queries = nullptr;

    numPartitionsPerDimension = ceil(pow(ceil(DATA_COUNT / (double)CRACKTHRESHOLD), 1 / (double)NUMDIMS));

    totalCells = 1;
    for (size_t d = 0; d < NUMDIMS; d++)
        totalCells *= numPartitionsPerDimension;

    loadData(data_file_name);
    loadQueries(query_file_name, query_size, queries);
    loadQueries(extended_query_file_name, query_size, extended_queries);

    auto start_indexing = clock();
    partitionGrid(numPartitionsPerDimension, gridIndex, gridIndexSize);

    AAKDTree *cellIndices = (AAKDTree *) malloc(totalCells * sizeof(AAKDTree));
    for (size_t i = 0; i < totalCells; i++)
    {
        cellIndices[i] = AAKDTree(gridIndex[i], gridIndexSize[i]);
    }

    auto end_indexing = clock() - start_indexing;

    ofstream output_file;
    output_file.open(output_file_name.c_str());


    ELEMTYPE domainMin[NUMDIMS], domainMax[NUMDIMS];
    fill_n(domainMin, NUMDIMS, 0.0);
    fill_n(domainMax, NUMDIMS, 1.0);
    Rect rootCover = Rect(domainMin, domainMax);

    for (size_t i = 0; i < query_size; i++)
    {
        auto start_querying = clock();
        size_t query_res = range_query(gridIndex, gridIndexSize, cellIndices, queries[i], extended_queries[i], numPartitionsPerDimension, rootCover);
        auto end_querying = clock() - start_querying;

        output_file << query_res << "\t" << end_querying / (double)CLOCKS_PER_SEC << "\t" << end_indexing / (double)CLOCKS_PER_SEC << endl;
    }

    output_file.close();
    return 0;
}
