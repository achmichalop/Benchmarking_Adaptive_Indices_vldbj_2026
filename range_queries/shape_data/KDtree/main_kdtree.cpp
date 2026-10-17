#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

#include "nanoflann.hpp"
#include "../basics.h"

using namespace nanoflann;
using namespace std;



struct BoxCloud
{
    using ElementType = ELEMTYPE;

    std::vector<Rect> boxes;

    size_t kdtree_get_point_count() const
    {
        return boxes.size();
    }

    double kdtree_get_box_low(
        size_t idx,
        size_t dim) const
    {
        return boxes[idx].m_min[dim];
    }

    double kdtree_get_box_high(
        size_t idx,
        size_t dim) const
    {
        return boxes[idx].m_max[dim];
    }
};

void loadData(string data_file_name, BoxCloud &dataset)
{
    ifstream data_file(data_file_name.c_str());
    if (!data_file.is_open())
    {
        cout << "Cannot find data file\n";
        exit(1);
    }

    for (size_t i = 0; i < DATA_COUNT; i++)
    {
        Rect p;

        for (size_t d = 0; d < NUMDIMS; d++)
            data_file >> p.m_min[d];

        for (size_t d = 0; d < NUMDIMS; d++)
            data_file >> p.m_max[d];

        dataset.boxes.push_back(p);
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
        for(size_t d = 0; d < NUMDIMS; d++)
        {
            query_file >> queries[i].m_min[d];
            
            if (queries[i].m_min[d] < 0.0f)
                queries[i].m_min[d] = 0.0f;
        }
        for(size_t d = 0; d < NUMDIMS; d++) 
        {
            query_file >> queries[i].m_max[d];
            
            if (queries[i].m_max[d] > 1.0f)
                queries[i].m_max[d] = 1.0f;
        }    
    }

    query_file.close();
}

int main(int argc, char** argv)
{
    string data_file_name = argv[1];
    size_t query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string output_file_name = argv[4];
    size_t leaf_capacity = stoi(argv[5]);

    BoxCloud dataset;
    Rect *queries = nullptr;


    loadData(data_file_name, dataset);
    loadQueries(query_file_name, query_size, queries);
    
    using KDTree = KDTreeBoxIndexAdaptor<BoxCloud, NUMDIMS, size_t>;
    
    auto start_indexing = clock();
    KDTree index(NUMDIMS, dataset, KDTreeSingleIndexAdaptorParams(leaf_capacity));
    auto end_indexing = clock() - start_indexing;

    ofstream output_file;
    output_file.open(output_file_name.c_str());

    for (size_t i = 0; i < query_size; i++)
    {
        typename KDTree::BoundingBox query_box;

        for (size_t d = 0; d < NUMDIMS; ++d)
        {
            query_box[d].low  = queries[i].m_min[d];
            query_box[d].high = queries[i].m_max[d];
        }

        vector<size_t> ret_indexes;
        auto start_querying = clock();
        auto query_res = index.findWithinBox(query_box,ret_indexes);
        auto end_querying = clock() - start_querying;

        output_file << query_res << "\t" << end_querying / (double)CLOCKS_PER_SEC << "\t" << end_indexing / (double)CLOCKS_PER_SEC << endl;
    }

    output_file.close();
    
    return 0;
}