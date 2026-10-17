#include "cracking_kd_tree.hpp"
#include "kd_node.hpp"
#include "full_scan.hpp"
#include <algorithm> // to check if all elements of a vector are true
#include <limits>
#include <full_scan_candidate_list.hpp>

using namespace std;

CrackingKDTree::CrackingKDTree(std::map<std::string, std::string> config){

    if(config.find("minimum_partition_size") == config.end())
        minimum_partition_size = 100;
    else
        minimum_partition_size = std::stoi(config["minimum_partition_size"]);
}
CrackingKDTree::~CrackingKDTree(){}

void CrackingKDTree::initialize(Table *table_to_copy){
        //! Check partition size, we change it to guarantee it always partitions all dimensions at least once
    while (minimum_partition_size > table_to_copy->row_count()/pow(2,table_to_copy->col_count())){
        minimum_partition_size /=2;
    }
    if (minimum_partition_size < 100){
        minimum_partition_size = 100;
    }
    // ******************
    auto start = measurements->time();

    // Copy the entire table
    table = make_unique<Table>(table_to_copy);

    index = make_unique<KDTree>(table->row_count());

    auto end = measurements->time();

    measurements->append(
            "initialization_time",
            std::to_string(Measurements::difference(end, start))
            );
    // ******************
}

void CrackingKDTree::adapt_index(Query &query) {
    // Before adapting calculate the scan overhead to measure how much the previous
    // queries helped this one
    auto search_results= index->search(query);
    auto partitions = search_results.first;
    n_tuples_scanned_before_adapting = 0;
    for(auto &partition : partitions)
        n_tuples_scanned_before_adapting += partition.second - partition.first;
    
    if(should_adapt){
        // If the index has converged then we stop adaptation
        if(index->has_converged(table.get(), minimum_partition_size)){
            should_adapt = false;
            // Add 0 to adaptation_time
            auto start = measurements->time();
            auto end = start;
            measurements->append(
                    "adaptation_time",
                    std::to_string(Measurements::difference(end, start))
                    );
        }else{


            // Adapt the KDTree 
            auto start = measurements->time();
            adapt(query);
            auto end = measurements->time();
            // ******************
            measurements->append(
                    "adaptation_time",
                    std::to_string(Measurements::difference(end, start))
                    );
        }
    }else{
        // Add 0 to adaptation_time
        auto start = measurements->time();
        auto end = start;
        measurements->append(
            "adaptation_time",
            std::to_string(Measurements::difference(end, start))
            );
    }
}


void CrackingKDTree::adapt_index(PointQuery &query, size_t k, size_t factor) {
    
    sample_size = max((size_t)20, factor * k);

    if(should_adapt){
        // If the index has converged then we stop adaptation
        if(index->has_converged(table.get(), minimum_partition_size)){
            should_adapt = false;
            // Add 0 to adaptation_time
            auto start = measurements->time();
            auto end = start;
            measurements->append(
                    "adaptation_time",
                    std::to_string(Measurements::difference(end, start))
                    );
        }else{
            // Adapt the KDTree 
            auto start = measurements->time();
            adapt(query);
            auto end = measurements->time();
            // ******************
            measurements->append(
                    "adaptation_time",
                    std::to_string(Measurements::difference(end, start))
                    );
        }
    }else{
        // Add 0 to adaptation_time
        auto start = measurements->time();
        auto end = start;
        measurements->append(
            "adaptation_time",
            std::to_string(Measurements::difference(end, start))
            );
    }
}


unique_ptr<Table> CrackingKDTree::range_query(Query &query) {
    // ******************
    auto start = measurements->time();

    // Search on the index the correct partitions
    auto search_results= index->search(query);
    auto partitions = search_results.first;
    auto partition_skip = search_results.second;

    auto end = measurements->time();
    measurements->append(
            "index_search_time",
            std::to_string(Measurements::difference(end, start))
            );

    start = measurements->time();
    // Scan the table and returns the row ids 

    auto result = FullScanCandidateList::scan_partition(table.get(), query,partitions, partition_skip);
    end = measurements->time();
        
    // ******************
    measurements->append(
        "scan_time",
        std::to_string(Measurements::difference(end, start))
    );

    int64_t n_tuples_scanned = 0;
    for(auto &partition : partitions)
        n_tuples_scanned += partition.second - partition.first;

    //std::cout << "--------------------- RESULTS " << result.second << std::endl;        
    //std::cerr << result.second << std::endl;        

    measurements->append("result", std::to_string(result.second));

    // Before returning the result, update the statistics.
    measurements->append("number_of_nodes", std::to_string(index->get_node_count()));
    measurements->append("max_height", std::to_string(index->get_max_height()));
    measurements->append("min_height", std::to_string(index->get_min_height()));
    measurements->append("memory_footprint", std::to_string(index->get_node_count() * sizeof(KDNode)));
    measurements->append("tuples_scanned", std::to_string(n_tuples_scanned));
    measurements->append("partitions_scanned", std::to_string(partitions.size()));

    auto skips = 0;
    for(size_t i = 0; i < partition_skip.size(); ++i){
        if(all_of(partition_skip.at(i).begin(), partition_skip.at(i).end(), [](bool v) { return v; })){
            skips += 1;
        }
    }
    measurements->append("partitions_skipped", std::to_string(skips));


    measurements->append(
        "scan_overhead_before_adapt",
        std::to_string(
            n_tuples_scanned_before_adapting/static_cast<float>(result.second)
        )
    );

    measurements->append(
        "scan_overhead_after_adapt",
        std::to_string(
            n_tuples_scanned/static_cast<float>(result.second)
        )
    );

    auto t = make_unique<Table>(2);
    float row[2] = {static_cast<float>(result.first), static_cast<float>(result.second)};
    t->append(row);
    
    return t;
}

void CrackingKDTree::adapt(Query& query){

    if(index->root == nullptr){
        auto position = table->CrackTable(
                0, table->row_count(),
                query.predicates[0].low, 0 
                );
        index->root = index->create_node(0, query.predicates[0].low, position);
    }

    for(size_t dim = 0; dim < query.predicate_count(); ++dim){
        if(query.predicates[dim].low >= std::numeric_limits<float>::lowest())
            adapt_recursion(
                    index->root.get(),
                    query,
                    dim, query.predicates[dim].low,
                    0, table->row_count()
                    );
    }

    for(size_t dim = 0; dim < query.predicate_count(); ++dim){
        if(query.predicates[dim].high <= std::numeric_limits<float>::max())
            adapt_recursion(
                    index->root.get(),
                    query,
                    dim, query.predicates[dim].high,
                    0, table->row_count()
                    );
    }
}

void CrackingKDTree::adapt(PointQuery& query){


    Table sample_table(table->col_count(), sample_size);

    index->get_sample_recursion(index->root.get(), 0, table->row_count(), query, 
                                    table.get(), sample_table, sample_size);

    // Query boxQuery = make_box_query(query, sample_table);   // quantile-side with p_low = p_high = 0.5
    Query boxQuery = make_box_query_quantile_based(query, sample_table, 3.0/4.0, 1.0/4.0);   
    

    if(index->root == nullptr){
        auto position = table->CrackTable(
                0, table->row_count(),
                boxQuery.predicates[0].low, 0 
                );
        index->root = index->create_node(0, boxQuery.predicates[0].low, position);
    }

    for(size_t dim = 0; dim < boxQuery.predicate_count(); ++dim){
        if(boxQuery.predicates[dim].low >= std::numeric_limits<float>::lowest())
            adapt_recursion(
                    index->root.get(),
                    boxQuery,
                    dim, boxQuery.predicates[dim].low,
                    0, table->row_count()
                    );
    }

    for(size_t dim = 0; dim < boxQuery.predicate_count(); ++dim){
        if(boxQuery.predicates[dim].high <= std::numeric_limits<float>::max())
            adapt_recursion(
                    index->root.get(),
                    boxQuery,
                    dim, boxQuery.predicates[dim].high,
                    0, table->row_count()
                    );
    }
}

Query CrackingKDTree::make_box_query(PointQuery query, Table &sample_table)
{
    size_t dimensions = sample_table.col_count(); 
    vector<vector<float>> minValsPerDim(dimensions), maxValsPerDim(dimensions);
    
    for (size_t i = 0; i < sample_table.row_count(); i++)
    {
        auto row = sample_table.materialize_row(i);
        for (size_t d = 0; d < dimensions; d++)
        {
            if (row[d] < query.coordinates[d])
            {
                minValsPerDim[d].push_back(row[d]);
            }
            else
            {
                maxValsPerDim[d].push_back(row[d]);
            }
        }
    }
    vector<pair<float,float>> lows_highs(dimensions);

    for (size_t d = 0; d < dimensions; d++)
    {
        if (!minValsPerDim[d].empty() && !maxValsPerDim[d].empty())
        {
            size_t medianPosInMins = minValsPerDim[d].size() >> 1;
            size_t medianPosInMaxes = maxValsPerDim[d].size() >> 1;

            nth_element(minValsPerDim[d].begin(), minValsPerDim[d].begin() + medianPosInMins, minValsPerDim[d].end());
            nth_element(maxValsPerDim[d].begin(), maxValsPerDim[d].begin() + medianPosInMaxes, maxValsPerDim[d].end());

            lows_highs[d].first = minValsPerDim[d][medianPosInMins];
            lows_highs[d].second = maxValsPerDim[d][medianPosInMaxes];
        }
        else if (minValsPerDim[d].empty() && !maxValsPerDim[d].empty())
        {
            size_t medianPosInMaxes = maxValsPerDim[d].size() >> 1;
            nth_element(maxValsPerDim[d].begin(), maxValsPerDim[d].begin() + medianPosInMaxes, maxValsPerDim[d].end());
            float medMax = maxValsPerDim[d][medianPosInMaxes];
            lows_highs[d].first = max(0.0f, query.coordinates[d] - (medMax - query.coordinates[d]));
            lows_highs[d].second = medMax;
        }
        else if (!minValsPerDim[d].empty() && maxValsPerDim[d].empty())
        {
            size_t medianPosInMins = minValsPerDim[d].size() >> 1;
            nth_element(minValsPerDim[d].begin(), minValsPerDim[d].begin() + medianPosInMins, minValsPerDim[d].end());
            float medMin = minValsPerDim[d][medianPosInMins];
            lows_highs[d].first = medMin;
            lows_highs[d].second = min(1.0f, query.coordinates[d] + (query.coordinates[d] - medMin));

        }
        else
        {
            float offset = query.coordinates[d] / 2;
            lows_highs[d].first = max(0.0f, query.coordinates[d] - offset);
            lows_highs[d].second = min(1.0f, query.coordinates[d] + offset);
        }
    }

    Query boxQuery(lows_highs);

    return boxQuery;
}


Query CrackingKDTree::make_box_query_quantile_based(PointQuery query, Table &sample_table, 
                                                double p_low, double p_high)
{
    size_t dimensions = sample_table.col_count(); 
    vector<vector<float>> lowVals(dimensions), highVals(dimensions);
    
    for (size_t i = 0; i < sample_table.row_count(); i++)
    {
        auto row = sample_table.materialize_row(i);
        for (size_t d = 0; d < dimensions; d++)
        {
            if (row[d] < query.coordinates[d])
            {
                lowVals[d].push_back(row[d]);
            }
            else
            {
                highVals[d].push_back(row[d]);
            }
        }
    }
    vector<pair<float,float>> lows_highs(dimensions);

    for (size_t d = 0; d < dimensions; d++)
    {
        if (!lowVals[d].empty() && !highVals[d].empty())
        {
            size_t lowPos = static_cast<size_t>(lowVals[d].size() * p_low);
            size_t highPos = static_cast<size_t>(highVals[d].size() * p_high);

            lowPos = min(lowPos, lowVals[d].size()-1);
            highPos = min(highPos, highVals[d].size()-1);


            nth_element(lowVals[d].begin(), lowVals[d].begin() + lowPos, lowVals[d].end());
            nth_element(highVals[d].begin(), highVals[d].begin() + highPos, highVals[d].end());

            lows_highs[d].first = lowVals[d][lowPos];
            lows_highs[d].second = highVals[d][highPos];
        }
        else if (lowVals[d].empty() && !highVals[d].empty())
        {
            size_t highPos = static_cast<size_t>(p_high * highVals[d].size());
            highPos = min(highPos, highVals[d].size()-1);
            nth_element(highVals[d].begin(), highVals[d].begin() + highPos, highVals[d].end());
            float selectedVal = highVals[d][highPos];
            lows_highs[d].first = max(0.0f, query.coordinates[d] - (selectedVal - query.coordinates[d]));
            lows_highs[d].second = selectedVal;
        }
        else if (!lowVals[d].empty() && highVals[d].empty())
        {
            size_t lowPos = static_cast<size_t>(p_low * lowVals[d].size());
            lowPos = min(lowPos, lowVals[d].size()-1);

            nth_element(lowVals[d].begin(), lowVals[d].begin() + lowPos, lowVals[d].end());
            float selectedVal = lowVals[d][lowPos];
            lows_highs[d].first = selectedVal;
            lows_highs[d].second = min(1.0f, query.coordinates[d] + (query.coordinates[d] - selectedVal));

        }
        else
        {
            float offset = query.coordinates[d] / 2;
            lows_highs[d].first = max(0.0f, query.coordinates[d] - offset);
            lows_highs[d].second = min(1.0f, query.coordinates[d] + offset);
        }
    }

    Query boxQuery(lows_highs);

    return boxQuery;
}


void CrackingKDTree::adapt_recursion(
        KDNode *current,
        Query& query,
        size_t pivot_dim,
        float pivot,
        size_t lower_limit,
        size_t upper_limit
        ){
    
    // If the size of the partition is already too small then stop exploring it
    if(upper_limit - lower_limit + 1 < minimum_partition_size)
        return;

    // If that pivot has already been inserted then we don't need to
    // keep looking in this branch.
    if(current->column == pivot_dim && current->key == pivot){
        return;
    }

    
    // If the current node has the same column as the pivot dim
    if(current->column == pivot_dim){
        if(pivot < current->key){
            if(current->left_child == nullptr){
                auto position = table->CrackTable(
                        lower_limit, current->position,
                        pivot, pivot_dim
                        );
                current->left_child = index->create_node(
                        pivot_dim, pivot, position
                        );
                // if(position == lower_limit || position == current->position )
                //     cout << "Trying to crack " << lower_limit << " " << upper_limit << " in " << pivot << " ---> DIDN'T crack 1\n";
            }else{
                adapt_recursion(
                        current->left_child.get(), query,
                        pivot_dim, pivot,
                        lower_limit, current->position
                        );
                return;
            }
        }else{
            if(current->right_child == nullptr){
                auto position = table->CrackTable(
                        current->position, upper_limit,
                        pivot, pivot_dim
                        );
                current->right_child = index->create_node(
                        pivot_dim, pivot, position
                        );
                // if(position == upper_limit || position == current->position )
                //     cout << "Trying to crack " << lower_limit << " " << upper_limit << " in " << pivot << " ---> DIDN'T crack 2\n";
            }else{
                adapt_recursion(
                        current->right_child.get(), query,
                        pivot_dim, pivot,
                        current->position, upper_limit
                        );
                return;
            }
        }
    }else{
        switch(current->compare(query)){
            case -1:
                //                  Key
                // Data:  |----------!--------|
                // Query:            |-----|
                //                  low   high
                if(current->right_child == nullptr){
                    auto position = table->CrackTable(
                            current->position, upper_limit,
                            pivot, pivot_dim
                            );
                    current->right_child = index->create_node(
                            pivot_dim, pivot, position
                            );
                    // if(position == upper_limit || position == current->position )
                    //     cout << "Trying to crack " << lower_limit << " " << upper_limit << " in " << pivot <<  " ---> DIDN'T crack 3\n";
                }else{
                    adapt_recursion(
                            current->right_child.get(), query,
                            pivot_dim, pivot,
                            current->position, upper_limit
                            );
                    return;
                }
                break;
            case +1:
                //                  Key
                // Data:  |----------!--------|
                // Query:      |-----|
                //            low   high
                if(current->left_child == nullptr){
                    auto position = table->CrackTable(
                            lower_limit, current->position,
                            pivot, pivot_dim
                            );
                    current->left_child = index->create_node(
                            pivot_dim, pivot, position
                            );
                    // if(position == lower_limit || position == current->position )
                    //     cout << "Trying to crack " << lower_limit << " " << upper_limit << " in " << pivot << " ---> DIDN'T crack 4\n";
                }else{
                    adapt_recursion(
                            current->left_child.get(), query,
                            pivot_dim, pivot,
                            lower_limit, current->position
                            );
                    return;
                }
                break;
            case 0:
                if(current->left_child == nullptr){
                    auto position = table->CrackTable(
                            lower_limit, current->position,
                            pivot, pivot_dim
                            );
                    current->left_child = index->create_node(
                            pivot_dim, pivot, position
                            );
                    // if(position == lower_limit || position == current->position )
                    //     cout << "Trying to crack " << lower_limit << " " << upper_limit << " in " << pivot <<  " ---> DIDN'T crack 5\n";
                }else{
                    adapt_recursion(
                            current->left_child.get(), query,
                            pivot_dim, pivot,
                            lower_limit, current->position
                            );
                }
                if(current->right_child == nullptr){
                    auto position = table->CrackTable(
                            current->position, upper_limit,
                            pivot, pivot_dim
                            );
                    current->right_child = index->create_node(
                            pivot_dim, pivot, position
                            );
                    // if(position == upper_limit || position == current->position )
                    //     cout << "Trying to crack " << lower_limit << " " << upper_limit << " in " << pivot <<  " ---> DIDN'T crack 6\n";
                }else{
                    adapt_recursion(
                            current->right_child.get(), query,
                            pivot_dim, pivot,
                            current->position, upper_limit
                            );
                }
                break;
            default:
                assert(false);
        }
    }
}

maxHeap CrackingKDTree::kNN_query(PointQuery query, size_t k)
{
    auto start = measurements->time();

    maxHeap Q(k);
    double worstDist = numeric_limits<double>::max();
    vector<double> proj_dists(query.coordinates.size(), 0);
    double dist = 0;

    find_neighbors(Q, query, index->root.get(), dist, worstDist, proj_dists, 0, table->row_count());
    
    auto end = measurements->time();
    
    measurements->append(
            "index_search_time",
            std::to_string(Measurements::difference(end, start))
            );

    start = measurements->time();
    end = start;
    measurements->append(
        "scan_time",
        std::to_string(Measurements::difference(end, start))
    );

    measurements->append("result", std::to_string(k));

    return Q;
}

void CrackingKDTree::find_neighbors(maxHeap &Q, PointQuery query, KDNode *current, double dist, 
                    double &worstDist, vector<double> &proj_dists, size_t lower_limit, size_t upper_limit)
{


    // Leaf node
    if (current == nullptr)
    {
        update_neighbors_heap(Q, query, worstDist, lower_limit, upper_limit);
        return;
    }

    size_t dim = current->column;
    float val = query.coordinates[dim];
    double diff = val - current->key;
    double new_proj = diff * diff;
    size_t best_lower, best_upper;
    size_t other_lower, other_upper;

    KDNode *bestChild;
    KDNode *otherChild;

    if (diff < 0)
    {
        bestChild = current->left_child.get();
        best_lower = lower_limit;
        best_upper = current->position;

        otherChild = current->right_child.get();
        other_lower = current->position;
        other_upper = upper_limit;
    }
    else
    {
        bestChild = current->right_child.get();
        best_lower = current->position;
        best_upper = upper_limit;   

        otherChild = current->left_child.get();
        other_lower = lower_limit;
        other_upper = current->position;
    }

    find_neighbors(Q, query, bestChild, dist, worstDist, proj_dists, best_lower, best_upper);

    double old_proj = proj_dists[dim];
    dist = dist + new_proj - old_proj;
    proj_dists[dim] = new_proj;

    if (Q.size() == Q.capacity && dist >= worstDist)
    {
        proj_dists[dim] = old_proj;
        return;
    }

    find_neighbors(Q, query, otherChild, dist, worstDist, proj_dists, other_lower, other_upper);
    proj_dists[dim] = old_proj;
}

void CrackingKDTree::update_neighbors_heap(maxHeap &Q, PointQuery query, double &worstDist, 
                                        size_t lower_limit, size_t upper_limit)
{
    for (size_t index = lower_limit; index < upper_limit; index++)
    {
        auto record = table->materialize_row(index);
        double dist = calculate_distance(query, record);
        Q.insert(maxHeapNode(dist, move(record)));
    }

    if (Q.size() == Q.capacity)
        worstDist = Q.top().dist;
}

double CrackingKDTree::calculate_distance(PointQuery query, std::unique_ptr<float[]>& record)
{
    double dist = 0.0;
    double diff = 0.0;
    size_t dimensions = query.coordinates.size();
    for (size_t d = 0; d < dimensions; d++)
    {
        diff = query.coordinates[d] - record[d];
        dist += diff * diff;
    }

    return dist;
}


