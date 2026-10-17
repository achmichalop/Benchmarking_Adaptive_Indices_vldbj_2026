#include "index_factory.hpp"
#include <string>
#include <iostream>
#include <map>
#include <algorithm>
// For the command line parsing
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>

#include "table.hpp"
#include "workload.hpp"

int main(int argc, char** argv){
    string workload_path = "queries";
    string data_path = "data";
    string index_algorithm = "Full Scan";
    string results_file;
    size_t algorithm_id = 0;
    size_t k_neighbors = 0;
    size_t nn_sampling_factor = 0;
    int number_of_repetitions = 3;
    map<string, string> config;
    int dimensions, dataSize, workload_size;

    int c;
    while ((c = getopt (argc, argv, "w:d:i:r:s:p:a:t:cz:o:y:x:k:l:n:f:")) != -1)
        switch (c)
        {
        case 'x':
            dimensions = atoi(optarg);
            break;
        case 'k':
            dataSize = atoi(optarg);
            break;
        case 'l':
            workload_size = atoi(optarg);
            break;
        case 'w':
            workload_path = optarg;
            break;
        case 'd':
            data_path = optarg;
            break;
        case 'i':
            algorithm_id = atoi(optarg);
            break;
        case 'r':
            number_of_repetitions = atoi(optarg);
            break;
        case 's':
            results_file = optarg;
            break;
        case 'p':
            config["minimum_partition_size"] = optarg;
            break;
        case 'a':
            config["delta"] = optarg;
            break;
        case 'c':
            config["interactivity_threshold"] = '1';
            break;
        case 'z':
            config["interactivity_threshold_is_time"] = '1';
            config["interactivity_threshold"] = optarg;
            break;
        case 'o':
            config["interactivity_threshold_is_time"] = '1';
            config["num_queries_over"] = optarg;
            break;
        case 'y':
            config["is_delta_fixed"] = "1";
            config["interactivity_threshold"] = optarg;
            break;
        case 'n':
            k_neighbors = atoi(optarg);
            break;
        case 'f':
            nn_sampling_factor = atoi(optarg);
            break;
        default:
            cout << "Usage:\n";
            cout << "-w <workload_path>\n";
            cout << "-d <data_path>\n";
            cout << "-i <algorithm id>\n";
            cout << "-r <number_of_repetitions>\n";
            cout << "-s <file_to_save_results>\n";
            cout << "-p <minimum_partition_size>\n";
            cout << "-a <delta>\n";
            cout << "-c <flag for progressive index to use costmodels>\n";
            cout << "-z <flag for progressive index interactive threshold time>\n";
            cout << "-o <flag for progressive index number of queries over the interactive thrshold>\n";
            return -1;
        }

    for(auto repetition = 0; repetition < number_of_repetitions; repetition++){
        auto index = IndexFactory::getIndex(algorithm_id, config);

        cout << endl;
        cout << index->name() << endl;
        cout << "Repetition: " << repetition << endl;
        cout << "Workload: " << workload_path << endl;
        cout << "Data: " << data_path << endl;
        cout << "Dimensions: " << dimensions << endl;
        cout << "Datasize: " << dataSize << endl;
        cout << "Num Queries " << workload_size << endl;
        cout << "Configurations:" << endl;

        for(auto& element : config){
            cout << '\t' << element.first << ": " << element.second << endl;
        }

        auto table = Table::read_file(data_path, dataSize, dimensions);
        auto workload = PointWorkload::read_file(workload_path, dimensions);

        index->initialize(table.get());

        for(size_t i = 0; i < workload_size; ++i){
            index->adapt_index(workload.queries.at(i), k_neighbors, nn_sampling_factor);
            maxHeap results = index->kNN_query(workload.queries.at(i), k_neighbors);
        }

        index->measurements->save(
                results_file,
                repetition
                );

    }
    return 0;
}
