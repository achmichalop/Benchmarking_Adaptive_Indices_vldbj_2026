#include "AAKD.h"

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

int main(int argc, char **argv) {
    string data_file_name = argv[1];
    size_t query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string output_file_name = argv[4];
    size_t k_neighbors = stoi(argv[5]);
    size_t nn_sampling_factor = stoi(argv[6]);
    AAKDTree aakd = AAKDTree(k_neighbors, nn_sampling_factor);
    Point *queries = nullptr;

    loadData(data_file_name);
    loadQueries(query_file_name, query_size, queries);

    ofstream output_file;
    output_file.open(output_file_name.c_str());

    ELEMTYPE domainMin[NUMDIMS], domainMax[NUMDIMS];
    fill_n(domainMin, NUMDIMS, 0.0);
    fill_n(domainMax, NUMDIMS, 1.0);

    Rect rootCover = Rect(domainMin, domainMax);
    for (size_t i = 0; i < query_size; i++)
    {   
        auto start_querying = clock();
        maxHeap<NUMDIMS> results = aakd.kNN_query(queries[i], k_neighbors, rootCover);
        auto end_querying = clock() - start_querying;

        output_file << results.size() << "\t" << end_querying / (double)CLOCKS_PER_SEC << endl;
    }



    output_file.close();
    return 0;
}