#include "../basics.h"

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

size_t range_query(ELEMTYPE** &index, size_t* &indexSize, Rect query, size_t numPartitionsPerDimension)
{
    size_t query_res = 0;
    size_t numExaminedCells = 1;
    float partitionExtent = 1.0f/numPartitionsPerDimension;

    array<size_t, NUMDIMS> startCell{}, endCell{};
    vector<array<size_t, NUMDIMS>> examinedCellsIds;
    vector<bool> examinedCellsFlags;

    for (size_t d = 0; d < NUMDIMS; d++)
    {
        startCell[d] = getStartOfCell(query.m_min[d] + EPS, partitionExtent);

        if (startCell[d] == numPartitionsPerDimension)
            startCell[d] = numPartitionsPerDimension-1;
        
        endCell[d] = getStartOfCell(query.m_max[d] + EPS, partitionExtent);
        
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
                query_res++;
        }
        else
        {
            for (size_t ii = 0; ii < indexSize[cellId]; ii++)
            {
                size_t offset = ii * NUMDIMS;
                
                if (Overlap(query.m_min, query.m_max, &index[cellId][offset]))
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
    ELEMTYPE **index = nullptr;
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

    float partitionExtent = 1.0f/numPartitionsPerDimension;

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