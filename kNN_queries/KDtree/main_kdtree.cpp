#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

#include "nanoflann.hpp"
#include "../basics.h"

using namespace nanoflann;
using namespace std;

struct PointCloud
{
    std::vector<std::array<ELEMTYPE, NUMDIMS>> pts;

    inline size_t kdtree_get_point_count() const
    {
        return pts.size();
    }

    inline ELEMTYPE kdtree_get_pt(
        const size_t idx,
        const size_t dim) const
    {
        return pts[idx][dim];
    }
    template <class BBOX>
    bool kdtree_get_bbox(BBOX&) const
    {
        return false;
    }
};


void loadData(string data_file_name, PointCloud& dataset)
{
    ifstream data_file(data_file_name.c_str());
    if (!data_file.is_open())
    {
        cout << "Cannot find data file\n";
        exit(1);
    }

    for (size_t i = 0; i < DATA_COUNT; i++)
    {
        array<ELEMTYPE, NUMDIMS> p;

        for (size_t d = 0; d < NUMDIMS; d++)
            data_file >> p[d];
        
        dataset.pts.push_back(p);
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

int main(int argc, char** argv)
{
    string data_file_name = argv[1];
    size_t query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string output_file_name = argv[4];
    size_t leaf_capacity = stoi(argv[5]);
    size_t k_neighbors = stoi(argv[6]);
    PointCloud dataset;
    Point *queries = nullptr;


    loadData(data_file_name, dataset);
    loadQueries(query_file_name, query_size, queries);

    
    using KDTree = KDTreeSingleIndexAdaptor< L2_Simple_Adaptor<ELEMTYPE, PointCloud>, PointCloud, NUMDIMS>;
    
    auto start_indexing = clock();
    KDTree index(NUMDIMS, dataset, KDTreeSingleIndexAdaptorParams(leaf_capacity));
    auto end_indexing = clock() - start_indexing;

    ofstream output_file;
    output_file.open(output_file_name.c_str());

    for (size_t i = 0; i < query_size; i++)
    {
        std::vector<size_t> ret_index(k_neighbors);
        std::vector<double> out_dist_sqr(k_neighbors);

        KNNResultSet<double, size_t> resultSet(k_neighbors);

        resultSet.init(ret_index.data(),out_dist_sqr.data());

        auto start_querying = clock();
        index.findNeighbors(resultSet, queries[i].m_values, SearchParameters());
        auto end_querying = clock() - start_querying;

        output_file << k_neighbors << "\t" << end_querying / (double)CLOCKS_PER_SEC << "\t" << end_indexing / (double)CLOCKS_PER_SEC << endl;
    }

    output_file.close();

    return 0;
}