#ifndef FULL_SCAN_CANDIDATE_LIST_H
#define FULL_SCAN_CANDIDATE_LIST_H

#include "abstract_index.hpp"
#include <string>
#include <map>
#include "candidate_list.hpp"

class FullScanCandidateList : public AbstractIndex
{
public:
    const static size_t ID = 111;
    FullScanCandidateList(std::map<std::string, std::string> config);
    ~FullScanCandidateList();

    std::string name() override{
        return "FullScan (Candidate List)";
    }

    void initialize(Table *table_to_copy) override;

    void adapt_index(Query &query) override;

    std::unique_ptr<Table> range_query(Query &query, Query &original_query) override;

    static std::pair<double, size_t> scan_partition(
            Table *t,
            Query& query,
            std::vector<std::pair<size_t, size_t> >& partitions,
            std::vector<std::vector<bool>> &per_partition_attribute_skip
            );
    static std::pair<double, size_t> scan_partition_with_refinement(
            Table *t,
            Query& query,
            std::vector<std::pair<size_t, size_t> >& partitions,
            std::vector<std::vector<bool>> &per_partition_attribute_skip,
            Query& original
            );
    static int refine_window_query_extension(
            Table *t, Table *t_max, 
            Query& original_query, 
            std::vector<std::pair<size_t, size_t>> &partitions);
};

inline std::pair<double, size_t> FullScanCandidateList::scan_partition(
    Table *t,
    Query& query,
    std::vector<std::pair<size_t, size_t>> &partitions,
    std::vector<std::vector<bool>> &per_partition_attribute_skip
){
    assert(partitions.size() == per_partition_attribute_skip.size());
    double sum = 0.0;
    size_t tuples_summed = 0.0;
    for(size_t partition_index = 0; partition_index < partitions.size(); ++partition_index){
        auto low = partitions[partition_index].first;
        auto high = partitions[partition_index].second;

        std::unique_ptr<CandidateList> qualifying_rows = std::make_unique<CandidateList>(0);

        // Handle the first predicate
        // If we should skip the first attribute
        auto column = query.predicates[0].column;
        if(per_partition_attribute_skip[partition_index][column]){
            for(size_t row_id = low; row_id < high; row_id++){
                qualifying_rows->push_back(row_id);
            }
        }else{
            // First we fill the qualifying rows
            auto low_pred = query.predicates[0].low;
            auto high_pred = query.predicates[0].high;

            for(size_t row_id = low; row_id < high; row_id++){
                auto value = t->columns[column]->data[row_id];
                qualifying_rows->maybe_push_back(row_id, low_pred <= value && value <= high_pred);
            }
        }


        // Skip the first predicate as we already processed it
        for(size_t predicate_index = 1; predicate_index < query.predicate_count(); ++predicate_index){

            if(qualifying_rows->size == 0)
                break;
            column = query.predicates[predicate_index].column;

            if(per_partition_attribute_skip[partition_index][column]){
                /* Do Nothing */
            }else{
                auto low_pred = query.predicates[predicate_index].low;
                auto high_pred = query.predicates[predicate_index].high;

                auto qualifying_index = 0;
                for(size_t i = 0; i < qualifying_rows->size; ++i){
                    auto value = t->columns[column]->data[qualifying_rows->get(i)];
                    int matching = low_pred <= value && value <= high_pred;
                    qualifying_rows->data[qualifying_index] = qualifying_rows->get(i);
                    qualifying_index+= matching;
                }
                qualifying_rows->size = qualifying_index;
            }

        }

        for(size_t i = 0; i < qualifying_rows->size; ++i){
            sum += (t->columns[0]->data[qualifying_rows->get(i)]);
        }
        tuples_summed += qualifying_rows->size;
    }
    return std::make_pair(sum, tuples_summed);

}

inline std::pair<double, size_t> FullScanCandidateList::scan_partition_with_refinement(
    Table *t,
    Query& query,
    std::vector<std::pair<size_t, size_t>> &partitions,
    std::vector<std::vector<bool>> &per_partition_attribute_skip,
    Query& original_query
){
    assert(partitions.size() == per_partition_attribute_skip.size());
    double sum = 0.0;
    int realResults = 0;
    size_t tuples_summed = 0.0;
    for(size_t partition_index = 0; partition_index < partitions.size(); ++partition_index){
        auto low = partitions[partition_index].first;
        auto high = partitions[partition_index].second;

        std::unique_ptr<CandidateList> qualifying_rows = std::make_unique<CandidateList>(0);

        // Handle the first predicate
        // If we should skip the first attribute
        auto column = query.predicates[0].column;
        if(per_partition_attribute_skip[partition_index][column]){
            for(size_t row_id = low; row_id < high; row_id++){
                qualifying_rows->push_back(row_id);
            }
        }else{
            // First we fill the qualifying rows
            auto low_pred = query.predicates[0].low;
            auto high_pred = query.predicates[0].high;

            for(size_t row_id = low; row_id < high; row_id++){
                auto min_value = t->columns[column]->data[row_id];
                auto max_value = t->max_columns[column]->data[row_id];
                // std::cout << "low, high, value " << low_pred << " " << high_pred << " " << max_value << " " << min_value << std::endl; 
                qualifying_rows->maybe_push_back(row_id, (low_pred <= min_value && min_value <= high_pred) );
            }
        }


        // Skip the first predicate as we already processed it
        for(size_t predicate_index = 1; predicate_index < query.predicate_count(); ++predicate_index){

            if(qualifying_rows->size == 0)
                break;
            column = query.predicates[predicate_index].column;

            if(per_partition_attribute_skip[partition_index][column]){
                /* Do Nothing */
            }else{
                auto low_pred = query.predicates[predicate_index].low;
                auto high_pred = query.predicates[predicate_index].high;

                // std::cout << "low, high 2 " << low_pred << " " << high_pred << std::endl; 

                auto qualifying_index = 0;
                for(size_t i = 0; i < qualifying_rows->size; ++i){
                    auto min_value = t->columns[column]->data[qualifying_rows->get(i)];
                    auto max_value = t->max_columns[column]->data[qualifying_rows->get(i)];
                    int matching = (low_pred <= min_value && min_value <= high_pred);
                    qualifying_rows->data[qualifying_index] = qualifying_rows->get(i);
                    qualifying_index+= matching;
                }
                qualifying_rows->size = qualifying_index;
            }

        }

        // std::cout << " ---------------------------------------------------------------------------------------  " << qualifying_rows->size << std::endl ;
        for(size_t i = 0; i < qualifying_rows->size; ++i){
            // std::cout << " - " << t->columns[0]->data[qualifying_rows->get(i)] << " "  << t->columns[1]->data[qualifying_rows->get(i)]<< std::endl;
            auto dataXStart = t->columns[0]->data[qualifying_rows->get(i)];
            auto dataYStart = t->columns[1]->data[qualifying_rows->get(i)];
            auto dataXEnd = t->max_columns[0]->data[qualifying_rows->get(i)];
            auto dataYEnd = t->max_columns[1]->data[qualifying_rows->get(i)];
            // std::cout << dataXStart << " " << dataYStart << " " << dataXEnd << " " << dataYEnd << std::endl;
            // std::cout << query.predicates[0].low << " " << query.predicates[1].low << " " << query.predicates[0].high << " " << query.predicates[1].high << std::endl;
            // std::cout << original_query.predicates[0].low << " " << original_query.predicates[1].low << " " << original_query.predicates[0].high << " " << original_query.predicates[1].high << std::endl;
            // std::cout << "\n";
            // if(dataXEnd >= original_query.predicates[0].low && dataXStart <= original_query.predicates[0].high && dataYEnd >= original_query.predicates[1].low && dataYStart <= original_query.predicates[1].high)
            // if (a_rectA->m_min[index] > a_rectB->m_max[index] || a_rectB->m_min[index] > a_rectA->m_max[index])
            
            if (dataXStart > original_query.predicates[0].high || original_query.predicates[0].low > dataXEnd || dataYStart > original_query.predicates[1].high || original_query.predicates[1].low > dataYEnd)
                continue;
            realResults++;

            sum += (t->columns[0]->data[qualifying_rows->get(i)]);
        }
        tuples_summed += qualifying_rows->size;
    }
    // std::cout << "Res " << tuples_summed <<" real " << realResults << std::endl;
    return std::make_pair(sum, realResults);

}


// inline std::pair<double, size_t> FullScanCandidateList::scan_partition_with_refinement(
//     Table *t,
//     Query& query,
//     std::vector<std::pair<size_t, size_t>> &partitions,
//     std::vector<std::vector<bool>> &per_partition_attribute_skip,
//     Query& original_query
// ){
//     assert(partitions.size() == per_partition_attribute_skip.size());
//     double sum = 0.0;
//     size_t tuples_summed = 0.0;
//     for(size_t partition_index = 0; partition_index < partitions.size(); ++partition_index){
//         auto low = partitions[partition_index].first;
//         auto high = partitions[partition_index].second;

//         std::unique_ptr<CandidateList> qualifying_rows = std::make_unique<CandidateList>(0);

//         // Handle the first predicate
//         // If we should skip the first attribute
//         auto column = query.predicates[0].column;
//         if(per_partition_attribute_skip[partition_index][column]){
//             for(size_t row_id = low; row_id < high; row_id++){
//                 auto data_x = t->columns[0]->data[row_id];
//                 auto data_y = t->columns[1]->data[row_id];
//                 auto data_x_max = t->max_columns[0]->data[row_id];
//                 auto data_y_max = t->max_columns[1]->data[row_id];

//                 std::cout << "Comparing " << data_x_max << " > " << original_query.predicates[0].low << std::endl;
//                 std::cout << "Comparing " << data_x << " < " << original_query.predicates[0].high << std::endl;
//                 std::cout << "Comparing " << data_y_max << " > " << original_query.predicates[1].low << std::endl;
//                 std::cout << "Comparing " << data_y << " < " << original_query.predicates[1].high << "\n" << std::endl;

//                 if(data_x_max > original_query.predicates[0].low && data_x < original_query.predicates[0].high && data_y_max > original_query.predicates[1].low && data_y < original_query.predicates[1].high){
//                     qualifying_rows->push_back(row_id);
//                 }
//             }
//         }else{
//             // First we fill the qualifying rows
//             auto low_pred = query.predicates[0].low;
//             auto high_pred = query.predicates[0].high;

//             auto original_low_pred = original_query.predicates[0].low;
//             auto original_high_pred = original_query.predicates[0].high;

//             for(size_t row_id = low; row_id < high; row_id++){
//                 auto value = (t->columns[column]->data[row_id] + t->max_columns[column]->data[row_id])/2;
//                 auto data_x = t->columns[0]->data[row_id];
//                 auto data_y = t->columns[1]->data[row_id];
//                 auto data_x_max = t->max_columns[0]->data[row_id];
//                 auto data_y_max = t->max_columns[1]->data[row_id];

//                 std::cout << "Comparing 2 " << data_x_max << " > " << original_query.predicates[0].low << std::endl;
//                 std::cout << "Comparing 2 " << data_x << " < " << original_query.predicates[0].high << std::endl;
//                 std::cout << "Comparing 2 " << data_y_max << " > " << original_query.predicates[1].low << std::endl;
//                 std::cout << "Comparing 2 " << data_y << " < " << original_query.predicates[1].high << "\n" << std::endl;
//                 std::cout << original_low_pred << " <= " <<  value << std::endl;
//                 std::cout << value << " <= " <<  original_high_pred << std::endl;

//                 if(data_x_max > original_query.predicates[0].low && data_x < original_query.predicates[0].high && data_y_max > original_query.predicates[1].low && data_y < original_query.predicates[1].high){
//                     qualifying_rows->maybe_push_back(row_id, original_low_pred <= value && value <= original_high_pred);
//                 }
                
//             }
//         }


//         // Skip the first predicate as we already processed it
//         for(size_t predicate_index = 1; predicate_index < query.predicate_count(); ++predicate_index){

//             if(qualifying_rows->size == 0)
//                 break;
//             column = query.predicates[predicate_index].column;

//             if(per_partition_attribute_skip[partition_index][column]){
//                 /* Do Nothing */
//             }else{
//                 auto low_pred = query.predicates[predicate_index].low;
//                 auto high_pred = query.predicates[predicate_index].high;

//                 auto original_low_pred = original_query.predicates[0].low;
//                 auto original_high_pred = original_query.predicates[0].high;

//                 auto qualifying_index = 0;
//                 for(size_t i = 0; i < qualifying_rows->size; ++i){
//                     auto value = t->columns[column]->data[qualifying_rows->get(i)];
//                     std::cout << "Matching " << original_low_pred << " <= " << value << " and " << " <= " << original_high_pred << std::endl;
//                     int matching = original_low_pred <= value && value <= original_high_pred;
//                     qualifying_rows->data[qualifying_index] = qualifying_rows->get(i);
//                     qualifying_index+= matching;
//                 }
//                 qualifying_rows->size = qualifying_index;
//             }

//         }

//         for(size_t i = 0; i < qualifying_rows->size; ++i){
//             sum += (t->columns[0]->data[qualifying_rows->get(i)]);
//         }
//         tuples_summed += qualifying_rows->size;
//     }
//     return std::make_pair(sum, tuples_summed);

// }

inline int FullScanCandidateList::refine_window_query_extension(Table *t, Table *t_max, Query& original_query, std::vector<std::pair<size_t, size_t>> &partitions){

        int useless_result = 0;
        for(size_t partition_index = 0; partition_index < partitions.size(); ++partition_index){
            auto low = partitions[partition_index].first;
            auto high = partitions[partition_index].second;
            // std::cout << " In refinement " << partitions.size() << " " << low << " " << high << std::endl;
            
            for(size_t row_id = low; row_id <= high; row_id++){
                auto data_x = t->columns[0]->data[row_id];
                auto data_y = t->columns[1]->data[row_id];


                // what I am about to do is not right, but it does simulate the same amount of operations it takes
                // to refine the results
                // to really get the correct results the table (table) and (table_max) should move together
                // but that is way too complicated, so I just did this

                // std::cout << "before tmax" << std::endl;
                auto data_x_max = t->max_columns[0]->data[row_id];
                // std::cout << "after 1 " << std::endl;
                auto data_y_max = t->max_columns[1]->data[row_id];
                // std::cout << "after 2 " << std::endl;

                // std::cout << "Comparing " << data_x << " <= " << original_query.predicates[0].low << std::endl;
                // std::cout << "Comparing " << data_x_max << " >= " << original_query.predicates[0].high << std::endl;
                // std::cout << "Comparing " << data_y << " <= " << original_query.predicates[1].low << std::endl;
                // std::cout << "Comparing " << data_y_max << " >= " << original_query.predicates[1].high << "\n" << std::endl;


                // check whether the shape overlaps the original query
                if(data_x < original_query.predicates[0].low && data_x_max > original_query.predicates[0].high
                    && data_y < original_query.predicates[1].low && data_y_max > original_query.predicates[1].high){
                       continue;
                }
                useless_result++;

            }

        }

        // std::cout << "RESSS " << useless_result << std::endl;
        return useless_result;
    }
#endif // FULL_SCAN_CANDIDATE_LIST_H
