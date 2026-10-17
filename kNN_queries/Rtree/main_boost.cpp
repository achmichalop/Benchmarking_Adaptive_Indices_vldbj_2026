#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/adapted/std_array.hpp>
#include <boost/geometry.hpp>
BOOST_GEOMETRY_REGISTER_STD_ARRAY_CS(cs::cartesian)

#include <vector>
#include <iostream>
#include <boost/foreach.hpp>

#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <fstream>
#include <string>
#include <sstream>
#include "../basics.h"
using namespace std;

namespace bg = boost::geometry;
namespace bgm = bg::model;
namespace bgi = boost::geometry::index;

#ifdef nodeSize16
    #define nodeSize 16
#elif nodeSize32
    #define nodeSize 32
#elif nodeSize64
    #define nodeSize 64
#elif nodeSize128
    #define nodeSize 128
#elif nodeSize256
    #define nodeSize 256
#elif nodeSize512
    #define nodeSize 512
#elif nodeSize1024
    #define nodeSize 1024
#elif nodeSize2048
    #define nodeSize 2048
#elif nodeSize4096
    #define nodeSize 4096
#elif nodeSize8192
    #define nodeSize 8192
#endif



using point = array<ELEMTYPE, NUMDIMS>;
using box = bgm::box<point>;
using record = pair<point, size_t>;


void loadData(string data_file_name, vector<record> &dataset)
{
    ifstream data_file(data_file_name.c_str());
    if (!data_file.is_open())
    {
        cout << "Cannot find data file\n";
        exit(1);
    }

    for (size_t i = 0; i < DATA_COUNT; i++)
    {
        point rec;

        for (size_t d = 0; d < NUMDIMS; d++)
            data_file >> rec[d];

        dataset.push_back(make_pair(rec, i));
    }

    data_file.close();
}

void loadQueries(string query_file_name, size_t query_size, vector<point> &queries)
{    
    ifstream query_file(query_file_name.c_str());
    if(!query_file.is_open())
    {
        cout << "Cannot find query file\n";
        exit(1);
    }

    for(size_t i = 0; i < query_size; i++){
        point rec;

        for(size_t d = 0; d < NUMDIMS; d++)
        {
            query_file >> rec[d];
        }
        
        queries.push_back(rec);
    }

    query_file.close();
}

int main(int argc, char **argv)
{

    string data_file_name = argv[1];
    int query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string time_file_name = argv[4];
    size_t k_neighbors = stoi(argv[5]);
    std::vector<record> dataset;
    std::vector<point> queries;
    
    loadData(data_file_name, dataset);
    loadQueries(query_file_name, query_size, queries);
    auto start_indexing = clock();
    bgi::rtree<record, bgi::quadratic<nodeSize>> rtree(dataset);
    auto end_indexing = clock() - start_indexing;

    ofstream times_file;
    times_file.open(time_file_name.c_str());

    for(int i = 0; i < query_size; i++){
        vector<record> results;
        auto start_querying = clock();
        rtree.query(bgi::nearest(queries[i], k_neighbors), std::back_inserter(results));
        auto end_querying = clock() - start_querying;
        times_file << results.size() << "\t" << (double)end_querying/CLOCKS_PER_SEC << "\t" << end_indexing / (double)CLOCKS_PER_SEC << "\n";
    }

    times_file.close();

    return 0;
}
