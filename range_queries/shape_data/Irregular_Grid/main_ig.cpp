#include "../basics.h"

array<size_t, NUMDIMS> producedPartitionsPerDim;
array<ELEMTYPE*, NUMDIMS> partitionValues;
array<size_t, NUMDIMS> strides;
size_t totalCells = 0;

size_t getStartOfCell(ELEMTYPE start, size_t currDim)
{
    size_t low = 0;
    size_t high = producedPartitionsPerDim[currDim];
    size_t mid;

    while (low < high)
    {
        mid = (low + high) >> 1;
        if (start > partitionValues[currDim][mid]) 
            low = mid + 1;
        else
            high = mid;
    }   
    
    return high;
}

size_t getCellIdx(ELEMTYPE value, size_t currDim)
{
    size_t idx = getStartOfCell(value, currDim);

    if (idx == producedPartitionsPerDim[currDim])
        idx = producedPartitionsPerDim[currDim] - 1;

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

size_t getCellRange(Rect &r, array<size_t, NUMDIMS> &startCell, array<size_t, NUMDIMS> &endCell)
{
    size_t numCells = 1;
    for (size_t d = 0; d < NUMDIMS; d++)
    {
        startCell[d] = getCellIdx(r.m_min[d], d);
        endCell[d] = getCellIdx(r.m_max[d], d);
        numCells *= (endCell[d] - startCell[d] + 1);
    }
    return numCells;
}

void getSample(vector<Point>&sample, float sample_ratio, size_t numPartitionsPerDimension)
{
    size_t total = DATA_COUNT;
    size_t sample_size = total * sample_ratio;

    if (sample_size <= numPartitionsPerDimension)
        sample_size *= numPartitionsPerDimension;

    sample.reserve(sample_size);

    std::vector<size_t> sampleIdx;
    sampleIdx.reserve(sample_size);
    size_t step = std::max<size_t>(1, total / sample_size);
    for (size_t i = 0; i < total && sampleIdx.size() < sample_size; i += step)
        sampleIdx.push_back(i);
    
    
    for (size_t i = 0; i < sample_size; i++)
    {
        ELEMTYPE centroid[NUMDIMS];

        for (size_t d = 0; d < NUMDIMS; d++)
            centroid[d] = (m_data_arr_rects[sampleIdx[i]].m_min[d] + m_data_arr_rects[sampleIdx[i]].m_max[d]) / 2;

        Point rec(centroid);
        sample.emplace_back(rec);
    }
}

void findPartitionsPerDim(vector<Point>& points, size_t currDim, size_t numPartitionsPerDimension)
{
    size_t lowerBoundPos = 0;
    size_t upperBoundPos = points.size();
    size_t depth = ceil(log2((double) numPartitionsPerDimension));

    uint32_t step = upperBoundPos / (double) numPartitionsPerDimension;

    std::vector<size_t> bounds;
    std::vector<ELEMTYPE> temp_partitions;
    bounds.resize(1 << depth);
    bounds[0] = lowerBoundPos;
    fill(bounds.begin() + numPartitionsPerDimension - 1, bounds.end(), upperBoundPos);

    temp_partitions.resize(1 << depth, 1.0f);

    for (size_t d = depth; d > 0; d--)
    {
        size_t low = 0;
        size_t stride = 1 << d;
        size_t high = stride - 1;
        size_t numIterations = 1 << (depth - d);
        for (size_t j = 0; j < numIterations; j++)
        {
            lowerBoundPos = bounds[low];
            upperBoundPos = bounds[high];
            size_t examinedPartPos = low + ((high - low) >> 1);
            if (examinedPartPos >= (numPartitionsPerDimension - 1))
                break;

            size_t medianElemPos = (examinedPartPos + 1) * step;
            
            if (!temp_partitions[high])
            {
                temp_partitions[examinedPartPos] = 0.0f;
                bounds[examinedPartPos] = medianElemPos;
                low = high;
                high += stride;
                continue;
            }

            auto lowIt = points.begin() + lowerBoundPos;
            auto medianIt = points.begin() + medianElemPos;
            auto highIt = points.begin() + upperBoundPos;

            nth_element(lowIt, medianIt, highIt, [currDim](auto& a, auto& b){return a.m_values[currDim] < b.m_values[currDim];});
    
            double examinedSeparator = points[medianElemPos].m_values[currDim];

            auto partitionIt = partition(lowIt, highIt, [examinedSeparator, currDim](auto& p){return p.m_values[currDim] < examinedSeparator;});

            uint32_t counter = partitionIt - points.begin();

            if (counter == lowerBoundPos)
            {
                examinedSeparator = 0;
                counter = medianElemPos;
            }

            temp_partitions[examinedPartPos] = examinedSeparator;
            bounds[examinedPartPos] = counter;

            low = high;
            high += stride;
        }
    }

    fill(temp_partitions.begin() + numPartitionsPerDimension, temp_partitions.end(), 0.0f);

    producedPartitionsPerDim[currDim] = temp_partitions.size() - std::count(temp_partitions.begin(), temp_partitions.end(), 0.0f);
    partitionValues[currDim] = (ELEMTYPE *) malloc(producedPartitionsPerDim[currDim] * sizeof(ELEMTYPE));
    for (size_t i = 0, k = 0; i < temp_partitions.size(); i++)
    {
        if (temp_partitions[i] != 0.0f)
            partitionValues[currDim][k++] = temp_partitions[i];
    }

}

void partitionGrid(size_t numPartitionsPerDimension, float sample_ratio, Rect** &index, size_t* &indexSize)
{
    array<size_t, NUMDIMS> startCell, endCell, cellIdx;
    totalCells = 1;

    vector<Point> sample;
    getSample(sample, sample_ratio, numPartitionsPerDimension);


    for (size_t d = 0; d < NUMDIMS; d++)
    {
        findPartitionsPerDim(sample, d, numPartitionsPerDimension);
        totalCells *= producedPartitionsPerDim[d];

        if (d == 0)
            strides[d] = 1;
        else
        strides[d] = strides[d-1] * producedPartitionsPerDim[d-1];
    }    

    index = (Rect **) malloc(totalCells * sizeof(Rect *));
    indexSize = (size_t *) malloc(totalCells * sizeof(size_t));

    size_t cellFilledPointer[totalCells];
    memset(cellFilledPointer, 0, totalCells * sizeof(size_t));
    memset(indexSize, 0, totalCells * sizeof(size_t));

    for (size_t i = 0; i < DATA_COUNT; i++)
    {   
        size_t numCells = getCellRange(m_data_arr_rects[i], startCell, endCell);

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
        size_t numCells = getCellRange(m_data_arr_rects[i], startCell, endCell);

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

bool isReferenceCell(Rect &rec, Rect &query, size_t cellId)
{
    array<size_t, NUMDIMS> refCellIdx;
    for (size_t d = 0; d < NUMDIMS; d++)
        refCellIdx[d] = getCellIdx(max(rec.m_min[d], query.m_min[d]), d);

    return getCellID(refCellIdx) == cellId;
}

size_t range_query(Rect** &index, size_t* &indexSize, Rect query)
{
    size_t query_res = 0;

    array<size_t, NUMDIMS> startCell{}, endCell{};
    vector<array<size_t, NUMDIMS>> examinedCellsIds;
    vector<bool> examinedCellsFlags;

    size_t numExaminedCells = getCellRange(query, startCell, endCell);

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
                if (isReferenceCell(index[cellId][ii], query, cellId))
                    query_res++;
            }
        }
        else
        {
            for (size_t ii = 0; ii < indexSize[cellId]; ii++)
            {
                if (Overlap(query, index[cellId][ii]) &&
                    isReferenceCell(index[cellId][ii], query, cellId))
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
    float sample_ratio = stof(argv[5]);
    size_t numPartitionsPerDimension = stoi(argv[6]);
    Rect **index = nullptr;
    size_t *indexSize = nullptr;
    Rect *queries = nullptr;

    loadData(data_file_name);
    loadQueries(query_file_name, query_size, queries);

    if constexpr (NUMDIMS != 2)
    {
        numPartitionsPerDimension = ceil(pow(numPartitionsPerDimension * numPartitionsPerDimension, 1 / (double)NUMDIMS));
    }

    auto start_indexing = clock();
    partitionGrid(numPartitionsPerDimension, sample_ratio, index, indexSize);
    auto end_indexing = clock() - start_indexing;

    ofstream output_file;
    output_file.open(output_file_name.c_str());


    for (size_t i = 0; i < query_size; i++)
    {
        auto start_querying = clock();
        size_t query_res = range_query(index, indexSize, queries[i]);
        auto end_querying = clock() - start_querying;
        output_file << query_res << "\t" << end_querying / (double)CLOCKS_PER_SEC << "\t" << end_indexing / (double)CLOCKS_PER_SEC << endl;
    }

    output_file.close();

    return 0;
}
