#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include "RTree_723oct21s_stochastic_14dec22.h"


using namespace std;


typedef int ValueType;
typedef float ELEMTYPE;
typedef double ELEMTYPEREAL;


#ifdef fo8
    const int fo = 8;
#elif fo16
    const int fo = 16;
#elif fo32
    const int fo = 32;
#endif


#ifdef maxt32
    const int maxt = 32;
#elif maxt64
    const int maxt = 64;
#elif maxt128
    const int maxt = 128;
#elif maxt256
    const int maxt = 256;
#elif maxt512
    const int maxt = 512;
#elif maxt1024
    const int maxt = 1024;
#elif maxt2048
    const int maxt = 2048;
#elif maxt4096
    const int maxt = 4096;
#endif


#ifdef mint16
    const int mint = 16;
#elif mint32
    const int mint = 32;
#elif mint64
    const int mint = 64;
#elif mint128
    const int mint = 128;
#elif mint256
    const int mint = 256;
#elif mint512
    const int mint = 512;
#elif mint1024
    const int mint = 1024;
#elif mint2048
    const int mint = 2048;
#endif

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
        for(size_t d = 0; d < NUMDIMS; d++) query_file >> queries[i].m_values[d];    
    }

    query_file.close();
}


int main(int argc, char **argv){
    string data_file_name = argv[1];
    int query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string time_file_name = argv[4];
    size_t k_neighbors = stoi(argv[5]);
    size_t nn_sampling_factor = stoi(argv[6]);
    
    typedef RTree<ValueType, fo, fo/2, maxt, mint> AIR;
    AIR tree(data_file_name, k_neighbors, nn_sampling_factor);

    Point *queries = nullptr;
    loadQueries(query_file_name, query_size, queries);

    ofstream times_file;
    times_file.open(time_file_name.c_str());

    clock_t startTime, endTime;

    for(int i = 0; i < query_size; i++){
 
        startTime = clock();

        maxHeap<NUMDIMS> results = tree.kNN_query(queries[i], k_neighbors);
        endTime = clock() - startTime;

        times_file << results.size() << "\t" << (double)endTime/CLOCKS_PER_SEC <<  "\n";
    }

    times_file.close();
    return 0;
}

