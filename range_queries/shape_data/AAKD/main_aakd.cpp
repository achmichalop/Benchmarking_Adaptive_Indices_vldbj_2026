#include "../basics.h"
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

int main(int argc, char **argv) {
    string data_file_name = argv[1];
    size_t query_size = stoi(argv[2]);
    string query_file_name = argv[3];
    string extended_query_file_name = argv[4];
    string output_file_name = argv[5];
    AAKDTree aakd = AAKDTree();
    Rect *queries = nullptr;
    Rect *extended_queries = nullptr;

    loadData(data_file_name);
    loadQueries(query_file_name, query_size, queries);
    loadQueries(extended_query_file_name, query_size, extended_queries);

    ofstream output_file;
    output_file.open(output_file_name.c_str());

    ELEMTYPE domainMin[NUMDIMS], domainMax[NUMDIMS];
    fill_n(domainMin, NUMDIMS, 0.0);
    fill_n(domainMax, NUMDIMS, 1.0);

    Rect rootCover = Rect(domainMin, domainMax);
    for (size_t i = 0; i < query_size; i++)
    {   
        auto start_querying = clock();
        int query_res = aakd.AdaptiveQuery(extended_queries[i], queries[i], rootCover);
        auto end_querying = clock() - start_querying;

        output_file << query_res << "\t" << end_querying / (double)CLOCKS_PER_SEC << endl;
    }

    output_file.close();
    return 0;
}