#include "measurements.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>

Measurements::Measurements(){}
Measurements::~Measurements(){}

double Measurements::time(){
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec + time.tv_usec/1e6;
}

double Measurements::difference(double end, double start){
    return end - start;
}

void Measurements::append(std::string key, std::string value){
    measurements[key].push_back(value);
}

std::vector<std::string> Measurements::get_headers(
    std::map<std::string, std::vector<std::string>> m
){
    std::vector<std::string> headers;
    for(auto const &element : m){
        headers.push_back(element.first);
    }
    std::sort(headers.begin(), headers.end());
    return headers;
}

void Measurements::save(std::string csv_name, int repetition){
    bool file_exists = exists(csv_name);
    // std::fstream csv_file(csv_name, std::ios::out | std::ios::app);
    std::fstream csv_file(csv_name, std::ios::out); 
    if( !csv_file.is_open() ) {
        std::cout << "Can't open file: " << csv_name << std::endl;
        return;
    }

    auto headers = get_headers(measurements);

    if(!file_exists){
        // Create header of CSV
        // for(auto const &header : headers)
        //     csv_file << header << ",";
        // csv_file << "repetition";
        // csv_file << std::endl;
    }

    // Append the results
    // for(size_t i = 0; i < measurements[headers[0]].size(); ++i){
    //     for(auto const& header : headers){
    //         if(measurements[header].size() <= i)
    //             csv_file << "0,";
    //         else
    //             csv_file << measurements[header][i] << ",";
    //     }
    //     csv_file << std::to_string(repetition);
    //     csv_file << std::endl;
    // }
    // Append the results DINOS

    // std::cout << " =============================== HEADER SIZE " << headers.size() << std::endl;
    float allWorkloadTime = 0;

    // if(headers.size() == 6)             // LINEAR SCAN
    // {
    //     auto resultIndex = std::find(headers.begin(), headers.end(), "result")- headers.begin();
    //     auto scantimeIndex = std::find(headers.begin(), headers.end(), "scan_time")- headers.begin();
    //     for(size_t i = 0; i < measurements[headers[0]].size(); ++i){
    //         for(size_t k = 0; k < headers.size();++k)
    //         {
    //             // std::cout << headers[k] << std::endl;
    //             if(headers[k] == "scan_time")
    //             {
                    
    //                 // float totalTime = atof(measurements[headers[k]][i].c_str())+atof(measurements[headers[k+1]][i].c_str());
    //                 csv_file << measurements[headers[resultIndex]][i].c_str() << "\t" << measurements[headers[scantimeIndex]][i].c_str() << std::endl;
    //             }
                    
    //         }
    //     }
    // }
    // else                                // AKD TREE
    // {
        float totalTime = 0.0;
        auto adaptationtimeIndex = std::find(headers.begin(), headers.end(), "adaptation_time")- headers.begin();
        auto scantimeIndex = std::find(headers.begin(), headers.end(), "scan_time")- headers.begin();
        auto indexsearchIndex = std::find(headers.begin(), headers.end(), "index_search_time")- headers.begin();
        auto inittimeIndex = std::find(headers.begin(), headers.end(), "initialization_time")- headers.begin();
        auto resultIndex = std::find(headers.begin(), headers.end(), "result")- headers.begin();
        auto nodeCountIndex = std::find(headers.begin(), headers.end(), "number_of_nodes")- headers.begin();

        
        // for(int ii = 0; ii < 8; ii++)
        //  csv_file << "dummy line " << ii <<"\n";

        // std::cout << "Size " << measurements[headers[0]].size() << std::endl;
        for(size_t i = 0; i < measurements[headers[0]].size(); ++i)
        {
            if(i == 0)
            {
                totalTime = atof(measurements[headers[inittimeIndex]][0].c_str())+atof(measurements[headers[indexsearchIndex]][i].c_str())+atof(measurements[headers[scantimeIndex]][i].c_str())+atof(measurements[headers[adaptationtimeIndex]][i].c_str());
            }
            else
            {
                totalTime = atof(measurements[headers[indexsearchIndex]][i].c_str())+atof(measurements[headers[scantimeIndex]][i].c_str())+atof(measurements[headers[adaptationtimeIndex]][i].c_str());
            }
            // std::cout << atof(measurements[headers[adaptationtimeIndex]][i].c_str()) << " " << atof(measurements[headers[indexsearchIndex]][i].c_str()) << " " << +atof(measurements[headers[scantimeIndex]][i].c_str()) << " " << totalTime << std::endl;
            allWorkloadTime += totalTime;
            // std::cout << "------------- " <<  measurements[headers[resultIndex]][i] << std::endl;
            csv_file << measurements[headers[resultIndex]][i] << "\t" << totalTime << std::endl;
        }
    // csv_file << "Total time " << allWorkloadTime << "\n";
    csv_file.close();
}

inline bool Measurements::exists (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}
