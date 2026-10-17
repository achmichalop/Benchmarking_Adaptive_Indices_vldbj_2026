#include "../basics.h"

array<size_t, NUMDIMS> strides;
size_t totalCells = 0;

size_t getStartOfCell(ELEMTYPE start, ELEMTYPE extent)
{
    return (size_t) (start / extent + EPS);
}

size_t getCellIdx(ELEMTYPE value, float partitionExtent, size_t numPartitionsPerDimension)
{
    size_t idx = getStartOfCell(value + EPS, partitionExtent);

    if (idx >= numPartitionsPerDimension)
        idx = numPartitionsPerDimension - 1;

    return idx;
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

void getCellIdxOfPosition(size_t pos, array<size_t, NUMDIMS> &startCell, array<size_t, NUMDIMS> &endCell, array<size_t, NUMDIMS> &cellIdx)
{
    size_t iters = pos;
    for (size_t d = 0; d < NUMDIMS; d++)
    {
        size_t extent = endCell[d] - startCell[d] + 1;
        cellIdx[d] = startCell[d] + iters % extent;
        iters /= extent;
    }
}

size_t getCellRange(Rect &r, float partitionExtent, size_t numPartitionsPerDimension, array<size_t, NUMDIMS> &startCell, array<size_t, NUMDIMS> &endCell)
{
    size_t numCells = 1;
    for (size_t d = 0; d < NUMDIMS; d++)
    {
        startCell[d] = getCellIdx(r.m_min[d], partitionExtent, numPartitionsPerDimension);
        endCell[d] = getCellIdx(r.m_max[d], partitionExtent, numPartitionsPerDimension);
        numCells *= (endCell[d] - startCell[d] + 1);
    }
    return numCells;
}

void partitionGrid(size_t numPartitionsPerDimension, Rect** &index, size_t* &indexSize)
{
    array<size_t, NUMDIMS> startCell, endCell, cellIdx;
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
        size_t numCells = getCellRange(m_data_arr_rects[i], partitionExtent, numPartitionsPerDimension, startCell, endCell);

        for (size_t c = 0; c < numCells; c++)
        {
            getCellIdxOfPosition(c, startCell, endCell, cellIdx);
            indexSize[getCellID(cellIdx)]++;
        }
    }

    for (size_t i = 0; i < totalCells; i++)
    {
        index[i] = (Rect *) malloc(indexSize[i] * sizeof(Rect));
    }

    for (size_t i = 0; i < DATA_COUNT; i++)
    {
        size_t numCells = getCellRange(m_data_arr_rects[i], partitionExtent, numPartitionsPerDimension, startCell, endCell);

        for (size_t c = 0; c < numCells; c++)
        {
            getCellIdxOfPosition(c, startCell, endCell, cellIdx);
            size_t cellId = getCellID(cellIdx);
            index[cellId][cellFilledPointer[cellId]] = m_data_arr_rects[i];
            cellFilledPointer[cellId]++;
        }
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

bool isReferenceCell(Rect &rec, Rect &query, size_t cellId, float partitionExtent, size_t numPartitionsPerDimension)
{
    array<size_t, NUMDIMS> refCellIdx;
    for (size_t d = 0; d < NUMDIMS; d++)
        refCellIdx[d] = getCellIdx(max(rec.m_min[d], query.m_min[d]), partitionExtent, numPartitionsPerDimension);

    return getCellID(refCellIdx) == cellId;
}

size_t range_query(Rect** &index, size_t* &indexSize, Rect query, size_t numPartitionsPerDimension)
{
    size_t query_res = 0;
    float partitionExtent = 1.0f/numPartitionsPerDimension;

    array<size_t, NUMDIMS> startCell{}, endCell{};
    vector<array<size_t, NUMDIMS>> examinedCellsIds;
    vector<bool> examinedCellsFlags;

    size_t numExaminedCells = getCellRange(query, partitionExtent, numPartitionsPerDimension, startCell, endCell);

    examinedCellsIds.resize(numExaminedCells);
    examinedCellsFlags.resize(numExaminedCells, true);


    for (size_t i = 0; i < numExaminedCells; i++)
    {
        getCellIdxOfPosition(i, startCell, endCell, examinedCellsIds[i]);

        for (size_t d = 0; d < NUMDIMS; d++)
        {
            size_t idx = examinedCellsIds[i][d];
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
                if (isReferenceCell(index[cellId][ii], query, cellId, partitionExtent, numPartitionsPerDimension))
                    query_res++;
            }
        }
        else
        {
            for (size_t ii = 0; ii < indexSize[cellId]; ii++)
            {
                if (Overlap(query, index[cellId][ii]) &&
                    isReferenceCell(index[cellId][ii], query, cellId, partitionExtent, numPartitionsPerDimension))
                    query_res++;
            }

        }
    }

    return query_res;
}

int main(int argc, char **argv)
{
    string data_file_name = argv[1];
    size_t query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string output_file_name = argv[4];
    size_t numPartitionsPerDimension = stoi(argv[5]);
    Rect **index = nullptr;
    size_t *indexSize = nullptr;
    Rect *queries = nullptr;

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

    for (size_t i = 0; i < query_size; i++)
    {
        auto start_querying = clock();

        size_t query_res = range_query(index, indexSize, queries[i], numPartitionsPerDimension);

        auto end_querying = clock() - start_querying;

        output_file << query_res << "\t" << end_querying / (double)CLOCKS_PER_SEC << "\t" << end_indexing / (double)CLOCKS_PER_SEC << endl;
    }

    output_file.close();
    return 0;
}
