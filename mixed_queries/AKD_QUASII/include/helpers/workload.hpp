#ifndef WORKLOAD_H
#define WORKLOAD_H

#include "query.hpp"
#include <cstdint>
#include <vector>
#include <string>

using namespace std;

class Workload 
{
public:
    vector<Query> queries;
    Workload();

    void append(Query query);

    static Workload read_file(string path, int dimensions);

    void save_file(string path); 

    size_t query_count() const;
};


class PointWorkload 
{
public:
    vector<PointQuery> queries;
    PointWorkload();

    void append(PointQuery query);

    static PointWorkload read_file(string path, int dimensions);
    size_t query_count() const;
};


class MixedWorkload 
{
public:
    vector<MixedQuery> queries;
    MixedWorkload();

    void append(MixedQuery query);

    static MixedWorkload read_file(string path, int dimensions);
    size_t query_count() const;
};



#endif // WORKLOAD_H
