#include "quasii.hpp"
#include "full_scan.hpp"
#include <iterator>
#include <math.h>
#include <algorithm>
#include <stack>
#include <fstream>
#include <limits>
#include <full_scan_candidate_list.hpp>

using namespace std;

Quasii::Quasii(std::map<std::string, std::string> config){
    if(config.find("minimum_partition_size") == config.end())
        last_level_threshold  = 100;
    else
        last_level_threshold  = std::stoi(config["minimum_partition_size"]);
}
Quasii::~Quasii(){}

void Quasii::initialize(Table *table_to_copy){
    //! Check partition size, we change it to guarantee it always partitions all dimensions at least once
    while (last_level_threshold > table_to_copy->row_count()/pow(2,table_to_copy->col_count())){
        last_level_threshold /=2;
    }
    if (last_level_threshold < 100){
        last_level_threshold = 100;
    }
    // ******************
    auto start = measurements->time();

    // Copy the entire table
    table = make_unique<Table>(table_to_copy);

    // Initialize with one Slice that covers all the data
    first_level_slices.push_back(
        createDefaultChild(0, 0, table->row_count())
    );

    // Calculate the thresholds
    calculate_level_thresholds();

    auto end = measurements->time();

        measurements->append(
        "initialization_time",
        std::to_string(Measurements::difference(end, start))
    );
    // ******************
}

void Quasii::adapt_index(Query &query) {
    // Before adapting calculate the scan overhead to measure how much the previous
    // queries helped this one
    auto search_results= search(query);
    auto partitions = search_results.first;
    n_tuples_scanned_before_adapting = 0;
    for(auto &partition : partitions)
        n_tuples_scanned_before_adapting += partition.second - partition.first;

    if(should_adapt){
        if(has_converged()){
            should_adapt = false;
            auto start = measurements->time();
            auto end = start;
            // ******************
            measurements->append(
                    "adaptation_time",
                    std::to_string(Measurements::difference(end, start))
                    );

        }else{
            // ******************
            auto start = measurements->time();
            build(first_level_slices, query);
            auto end = measurements->time();
            // ******************
            measurements->append(
                    "adaptation_time",
                    std::to_string(Measurements::difference(end, start))
                    );

        }

    }else{
        // ******************
        auto start = measurements->time();
        auto end = start;
        // ******************
        measurements->append(
                "adaptation_time",
                std::to_string(Measurements::difference(end, start))
                );
    }

}

void Quasii::adapt_index(PointQuery &query, size_t k, size_t factor) {

    sample_size = max((size_t)20, factor * k);
    if(should_adapt){
        if(has_converged()){
            should_adapt = false;
            auto start = measurements->time();
            auto end = start;
            // ******************
            measurements->append(
                    "adaptation_time",
                    std::to_string(Measurements::difference(end, start))
                    );

        }else{
            // ******************
            auto start = measurements->time();
            build(first_level_slices, query);

            auto end = measurements->time();
            // ******************
            measurements->append(
                    "adaptation_time",
                    std::to_string(Measurements::difference(end, start))
                    );

        }

    }else{
        // ******************
        auto start = measurements->time();
        auto end = start;
        // ******************
        measurements->append(
                "adaptation_time",
                std::to_string(Measurements::difference(end, start))
                );
    }

}

Query Quasii::make_box_query(PointQuery query, Table &sample_table)
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

Query Quasii::make_box_query_quantile_based(PointQuery query, Table &sample_table, 
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

void Quasii::get_sample_recursion(Slice& slice, PointQuery& query, Table& sample_table, size_t sample_size)
{
    if (sample_table.row_count() >= sample_size)
        return;
    
    if (slice.children.empty())
    {
        fill_sample_table(slice.offset_begin, slice.offset_end, sample_table, sample_size);
        return;
    }

    size_t dim = slice.children[0].column;
    float val = query.coordinates[dim];
    size_t child_index = binarySearch(slice.children, val);
    
    get_sample_recursion(slice.children[child_index], query, sample_table, sample_size);

    int left  = static_cast<int>(child_index) - 1;
    int right = static_cast<int>(child_index) + 1;

    while (left >= 0 || right < static_cast<int>(slice.children.size()))
    {
        double left_dist = std::numeric_limits<double>::max();
        double right_dist = std::numeric_limits<double>::max();

        if (left >= 0)
        {
            left_dist = val - slice.children[left].right_value;
        }

        if (right < static_cast<int>(slice.children.size()))
        {
            right_dist = slice.children[right].left_value - val;
        }

        if (left_dist <= right_dist)
        {
            get_sample_recursion(slice.children[left],query,sample_table,sample_size);
            --left;
        }
        else
        {
            get_sample_recursion(slice.children[right],query,sample_table,sample_size);
            ++right;
        }
    }
}

void Quasii::fill_sample_table(size_t lower_limit, size_t upper_limit, Table& sample_table, size_t sample_size)
{
    size_t partition_size = upper_limit - lower_limit;
    size_t residue = sample_size - sample_table.row_count();
    size_t step = (partition_size <= residue) ? 1 : ceil(partition_size / (double)residue);
    size_t numIters = (partition_size < residue) ? partition_size : residue;
    // cout << "Partition Size: " << partition_size << " -- " 
    //      << "Lower: " << lower_limit << " -- "
    //      << "Upper: " << upper_limit << " -- "
    //      << "Filled: " << sample_table.row_count() << " -- "
    //      << "Residue: " << residue << endl;

    for (size_t i = 0, index = lower_limit; i < numIters; i++, index+=step)
    {
        sample_table.append(table->materialize_row(index).get());
    }
}

unique_ptr<Table> Quasii::range_query(Query &query) {
    // ******************
    auto start = measurements->time();

    // Search on the index the correct partitions
    auto search_results= search(query);
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

    measurements->append("result", std::to_string(result.second));

    // ******************
    measurements->append(
            "scan_time",
            std::to_string(Measurements::difference(end, start))
            );

    int64_t n_tuples_scanned = 0;
    for(auto &partition : partitions)
        n_tuples_scanned += partition.second - partition.first;

    // Before returning the result, update the statistics.
    measurements->append(
            "number_of_nodes",
            std::to_string(count_slices(first_level_slices))
            );
    measurements->append("max_height", std::to_string(table->col_count()));
    measurements->append("min_height", std::to_string(table->col_count()));
    measurements->append(
            "memory_footprint",
            std::to_string(count_slices(first_level_slices) * sizeof(Slice))
            );
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


// size_t quasii_examined_regions = 0;
// size_t quasii_examined_points = 0;

maxHeap Quasii::kNN_query(PointQuery query, size_t k)
{

    auto start = measurements->time();

    maxHeap Q(k);
    double worstDist = numeric_limits<double>::max();
    vector<double> proj_dists(query.coordinates.size(), 0);
    double dist = 0;

    auto dim = first_level_slices[0].column;
    float val = query.coordinates[dim];

    size_t slice_index = binarySearch(first_level_slices, val);
    find_neighbors(Q, query, first_level_slices[slice_index], dist, worstDist, proj_dists);
    
    int left  = static_cast<int>(slice_index) - 1;
    int right = static_cast<int>(slice_index) + 1;

    while (left >= 0 || right < static_cast<int>(first_level_slices.size()))
    {
        double left_proj = numeric_limits<double>::max();
        double right_proj = numeric_limits<double>::max();

        if (left >= 0)
        {
            auto& left_slice = first_level_slices[left];
            left_proj = 0.0;

            if (val < left_slice.left_value)
            {
                double diff = val - left_slice.left_value;
                left_proj = diff * diff;
            }
            else if (val > left_slice.right_value)
            {
                double diff = val - left_slice.right_value;
                left_proj = diff * diff;
            }
        }

        if (right < static_cast<int>(first_level_slices.size()))
        {
            auto& right_slice = first_level_slices[right];
            right_proj = 0.0;
            if (val < right_slice.left_value)
            {
                double diff = right_slice.left_value - val;
                right_proj = diff * diff;
            }
            else if (val > right_slice.right_value)
            {
                double diff = right_slice.right_value - val;
                right_proj = diff * diff;
            }
        }

        if (left_proj <= right_proj)
        {
            if (left < 0)
                break;

            if (Q.size() == Q.capacity && left_proj >= worstDist)
                break;

            size_t next_slice = static_cast<size_t>(left);
            fill(proj_dists.begin(), proj_dists.end(), 0.0);
            proj_dists[dim] = left_proj;

            find_neighbors(Q,query,first_level_slices[next_slice],left_proj,worstDist,proj_dists);
            --left;
        }
        else
        {
         if (right >= static_cast<int>(first_level_slices.size()))
                break;

            if (Q.size() == Q.capacity && right_proj >= worstDist)
                break;

            size_t next_slice = static_cast<size_t>(right);
            fill(proj_dists.begin(), proj_dists.end(), 0.0);
            proj_dists[dim] = right_proj;

            find_neighbors(Q,query,first_level_slices[next_slice],right_proj,worstDist,proj_dists);
            ++right;
        }
    }

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

void Quasii::find_neighbors(maxHeap &Q, PointQuery query, Slice& slice, double dist, 
                    double &worstDist, vector<double> &proj_dists)
{
    if (slice.children.empty())
    {
        update_neighbors_heap(Q,query,worstDist,slice.offset_begin,slice.offset_end);
        return;
    }

    size_t dim = slice.children[0].column;
    float val = query.coordinates[dim];
    size_t child_index = binarySearch(slice.children, val);
    double old_proj = proj_dists[dim];

    find_neighbors(Q,query,slice.children[child_index],dist,worstDist,proj_dists);
    
    int left  = static_cast<int>(child_index) - 1;
    int right = static_cast<int>(child_index) + 1;


    while (left >= 0 || right < static_cast<int>(slice.children.size()))
    {
        double left_proj = std::numeric_limits<double>::max();
        double right_proj = std::numeric_limits<double>::max();

        if (left >= 0)
        {
            left_proj = 0.0;

            if (val < slice.children[left].left_value)
            {
                double diff = val - slice.children[left].left_value;
                left_proj = diff * diff;
            }
            else if (val > slice.children[left].right_value)
            {
                double diff = val - slice.children[left].right_value;
                left_proj = diff * diff;
            }
        }

        if (right < static_cast<int>(slice.children.size()))
        {
            right_proj = 0.0;

            if (val < slice.children[right].left_value)
            {
                double diff = slice.children[right].left_value - val;
                right_proj = diff * diff;
            }
            else if (val > slice.children[right].right_value)
            {
                double diff = slice.children[right].right_value - val;
                right_proj = diff * diff;
            }
        }

        int next_child;
        double new_proj;

        if (left_proj <= right_proj)
        {
            next_child = left;
            new_proj = left_proj;
            --left;
        }
        else
        {
            next_child = right;
            new_proj = right_proj;
            ++right;
        }

        double new_dist = dist + new_proj - old_proj;


        if (Q.size() == Q.capacity && new_dist >= worstDist)
        {
            break;
        }

        proj_dists[dim] = new_proj;
        find_neighbors(Q,query, slice.children[next_child],new_dist,worstDist,proj_dists);

        proj_dists[dim] = old_proj;
    }

    proj_dists[dim] = old_proj;
}


void Quasii::update_neighbors_heap(maxHeap &Q, PointQuery query, double &worstDist, 
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

double Quasii::calculate_distance(PointQuery query, std::unique_ptr<float[]>& record)
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


size_t Quasii::count_slices(std::vector<Slice> &slices){
    int64_t number_of_slices = slices.size();
    for(auto& slice : slices){
        number_of_slices += count_slices(slice.children);
    }
    return number_of_slices;
}

void Quasii::search_recursion(
        Slice &slice,
        Query &query,
        vector<pair<size_t, size_t>> &partitions,
        vector<vector<bool>> &partition_skip,
        vector<pair<float, float>> partition_borders
        ){
    if(slice.children.empty()){
        partitions.push_back(
                make_pair(slice.offset_begin, slice.offset_end)
                );
        partition_borders.at(slice.column).first = slice.left_value;
        partition_borders.at(slice.column).second = slice.right_value;
        partition_skip.push_back(
                query.covers(partition_borders)
                );
    }else{
        auto predicate = query.predicates[slice.children[0].column];
        auto i = binarySearch(slice.children, predicate.low);

        while(i < static_cast<int64_t>(slice.children.size()) && slice.children[i].left_value <= predicate.high){
            partition_borders.at(slice.column).first = slice.left_value;
            partition_borders.at(slice.column).second = slice.right_value;
            search_recursion(slice.children[i], query, partitions, partition_skip, partition_borders);
            ++i;
        }
    }
}

pair<vector<pair<size_t, size_t>>, vector<vector<bool>>> Quasii::search(Query& query){
    std::vector<pair<size_t, size_t>> partitions;
    std::vector<std::vector<bool>> partition_skip;

    auto predicate = query.predicates[first_level_slices[0].column];
    auto i = binarySearch(first_level_slices, predicate.low);

    vector<pair<float, float>> partition_borders(query.predicate_count());
    for(size_t i = 0; i < query.predicate_count(); ++i){
        partition_borders.at(i) = make_pair(
                numeric_limits<float>::lowest(),
                numeric_limits<float>::max()
                );
    }
    while(i < static_cast<int64_t>(first_level_slices.size()) && first_level_slices[i].left_value <= predicate.high){
        search_recursion(first_level_slices[i], query, partitions, partition_skip, partition_borders);
        ++i;
    }
    return make_pair(partitions, partition_skip);
}

// biggest who is less or equal to the key
int64_t Quasii::binarySearch(const std::vector<Slice> &slice, float key){
    auto  min = (int64_t) 0;
    auto  max = static_cast<int64_t>(slice.size() - 1);

    if(min == max)
        return min;

    while (max >= min && max > 0) {
        int64_t mid = ((max+min)/2.0) + 0.5;

        if(slice[mid].left_value ==  key){
            return mid;
        }else if(slice[mid].left_value < key){
            min = mid +1;
        }else if(slice[mid].left_value > key){
            max = mid -1;
        }

    }
    if(min < max)
        return min;
    return max;
}

void Quasii::calculate_level_thresholds(){
    if(table->col_count() == 1){
        dimensions_threshold.push_back(last_level_threshold);
        return;
    }
    dimensions_threshold.push_back(last_level_threshold);
    auto number_of_columns = static_cast<double>(table->col_count());
    auto number_of_rows = static_cast<double>(table->row_count());
    auto r = ceil(
            pow(
                static_cast<double>(number_of_rows/last_level_threshold),
                static_cast<double>(1.0/number_of_columns)
               )
            );

    auto cur_thr = r * last_level_threshold;
    dimensions_threshold.push_back(cur_thr);
    for (auto i = 2; i  < number_of_columns; i ++){
        cur_thr = r * cur_thr;
        dimensions_threshold.push_back(cur_thr);
    }
    reverse(dimensions_threshold.begin(),dimensions_threshold.end());
}

struct less_than_offset
{
    inline bool operator() (const Slice& s1, const Slice& s2)
    {
        return (s1.offset_begin < s2.offset_begin);
    }
};

void Quasii::build(std::vector<Slice> &slices, Query &query){
    std::vector<Slice> S1; // S'
    auto dim = slices[0].column;
    auto predicate = query.predicates[dim];
    auto low = predicate.low;
    auto high = predicate.high;
    vector<size_t> indexes_to_remove;
    for(auto i = binarySearch(slices, low); i < static_cast<int64_t>(slices.size()) && slices[i].left_value <= high; ++i){
        std::vector<Slice> S2 = refine(slices[i], predicate); // S''
        if(S2.size() == 0){
            auto& r_s = slices[i];
            if(r_s.column != table->col_count() - 1){
                if(r_s.children.size() == 0){
                    r_s.children.push_back(
                            createDefaultChild(r_s.column + 1, r_s.offset_begin,r_s.offset_end)
                            );
                }
                build(r_s.children, query);
            }
        }else{
            indexes_to_remove.push_back(i);
            for (auto &r_s : S2){
                if(r_s.intersects(low, high)){
                    if(r_s.column != table->col_count() - 1){
                        if(r_s.children.size() == 0){
                            r_s.children.push_back(
                                    createDefaultChild(r_s.column + 1, r_s.offset_begin,r_s.offset_end)
                                    );
                        }
                        build(r_s.children, query);
                    }
                }
            }
            S1.insert(
                    S1.end(),
                    std::make_move_iterator(S2.begin()),
                    std::make_move_iterator(S2.end())
                    );
        }
    }

    for(int64_t j = indexes_to_remove.size() - 1; j >= 0; --j){
        slices[indexes_to_remove[j]] = slices.back();
        slices.pop_back();
    }

    slices.insert(
            slices.end(),
            std::make_move_iterator(S1.begin()),
            std::make_move_iterator(S1.end())
            );

    sort(slices.begin(), slices.end(), less_than_offset());
}

void Quasii::build(std::vector<Slice> &slices, PointQuery &query){
    

    Table sample_table(table->col_count(), sample_size);

    auto dim = slices[0].column;
    float val = query.coordinates[dim];

    size_t slice_index = binarySearch(slices, val);
    get_sample_recursion(slices[slice_index], query, sample_table, sample_size);

    Query boxQuery = make_box_query_quantile_based(query, sample_table, 1.0/4.0, 3.0/4.0);   // p_low = 1/4,  p_high = 3/4

    build(slices, boxQuery);


}


std::vector<Slice> Quasii::refine(Slice &slice, Predicate &predicate){
    auto low = predicate.low;
    auto high = predicate.high;
    std::vector<Slice> result_slices;
    std::vector<Slice> refined_slices;
    // If the slice size is below the threshold then dont refine it
    if (((slice.offset_end - slice.offset_begin) <= dimensions_threshold[slice.column]) || (slice.children.size() > 0)){
        //refined_slices.push_back(slice);
        return refined_slices;
    }

    // lower and high are within box
    if (slice.left_value <= low && high <= slice.right_value){
        refined_slices = sliceThreeWay(slice, low, high);
    }
    else if (slice.left_value <= low && low < slice.right_value){
        refined_slices = sliceTwoWay(slice, low);
    }
    else if (slice.left_value < high && high <= slice.right_value ){
        refined_slices = sliceTwoWay(slice, high);
    }
    else{
        refined_slices = sliceArtificial(slice);
    }

    for (auto &r_s : refined_slices){
        if(r_s.size() > dimensions_threshold[r_s.column] && r_s.intersects(low, high)){
            std::vector<Slice> refined_slice_aux = sliceArtificial(r_s);
            result_slices.insert(
                    result_slices.end(),
                    refined_slice_aux.begin(),
                    refined_slice_aux.end()
                    );
        }
        else{
            result_slices.push_back(r_s);
        }
    }
    return result_slices;

}

// This case there is no pivot in the slice, but it is inside the query
// Slice:
//          |===|
//          |===|
// Query: |---------|
//
// So it is necessary to crack using artificial pivots.
// Choose the middle one based on the left and right values of the slice.
// Keep cracking the new slices until the threshold is achieved.
// Crack the child of the new slices also.


void Quasii::sliceArtificialRecursion(Slice& slice, std::vector<Slice>& result){
    auto threshold = dimensions_threshold[slice.column];
    if(slice.size() < threshold){
        result.push_back(slice);
        return;
    }

    std::vector<Slice> slices_refined = sliceTwoWay(
            slice, (slice.right_value + slice.left_value)/2.0
            );

    // The slicing had no effect, so stop slicing
    if(slices_refined.size() < 2){
        result.push_back(slices_refined[0]);
        return;
    }

    for(auto& s : slices_refined){
        sliceArtificialRecursion(s, result);
    }
}
std::vector<Slice> Quasii::sliceArtificial(Slice &slice){
    std::vector<Slice> result;
    auto threshold = dimensions_threshold[slice.column];

    if(slice.size() < threshold){
        result.push_back(slice);
        return result;
    }

    sliceArtificialRecursion(slice, result);

    return result;
}

// Same as Cracking-in-Two
//         |==================|
//         |                  |
//         |                  |
//         |                  |
//         |                  |
//         |==================|
//                  ||
//                Pivot
//
// Results into two slices:
//         |========||==========|
//         |        ||          |
//         |        ||          |
//         |        ||          |
//         |        ||          |
//         |========||==========|

std::vector<Slice> Quasii::sliceTwoWay(Slice &slice, float key){
    std::vector<Slice> result;

    auto pivot_index = table->CrackTable(
            slice.offset_begin, slice.offset_end, key, slice.column
            );

    // It might be the case that the pivot_index is equal to the offset_begin
    // Then instead of creating a new slice, just extend the other one
    if(slice.offset_begin >= pivot_index){
        pivot_index = slice.offset_begin;
    }else{
        result.push_back(
                Slice(
                    slice.column, slice.offset_begin, pivot_index, slice.left_value, key
                    )
                );
    }
    // If the pivot index is greater or equal to the offset_end
    // then we shouldn't insert it
    if(!(pivot_index >= slice.offset_end)){
        result.push_back(
                Slice(
                    slice.column, pivot_index, slice.offset_end, key, slice.right_value
                    )
                );
    }

    return result;
}

// Same as Cracking-in-Three
//         |==================|
//         |                  |
//         |                  |
//         |                  |
//         |                  |
//         |==================|
//            ||          ||
//          Pivot1      Pivot2
//
// Results into three slices:
//         |========||===||=======|
//         |        ||   ||       |
//         |        ||   ||       |
//         |        ||   ||       |
//         |        ||   ||       |
//         |========||===||=======|

std::vector<Slice> Quasii::sliceThreeWay(Slice &slice, float low, float high){
    std::vector<Slice> result;

    auto crack_result = table->CrackTableInThree(
            slice.offset_begin, slice.offset_end, low, high, slice.column
            );
    auto first_crack = crack_result.first;
    auto second_crack = crack_result.second;

    if(first_crack - slice.offset_begin > 0){
        result.push_back(
                Slice(slice.column, slice.offset_begin, first_crack, slice.left_value, low)
                );
    }

    if(second_crack - first_crack > 0){
        result.push_back(
                Slice(slice.column, first_crack, second_crack, low, high)
                );
    }

    if(slice.offset_end - second_crack > 0){
        result.push_back(
                Slice(slice.column, second_crack, slice.offset_end, high, slice.right_value)
                );
    }
    return result;
}

void Quasii::draw_index(std::string path){
    std::ofstream myfile(path.c_str());

    myfile << "digraph Quasii{\n node [shape=record];\n";

    std::vector<std::vector<Slice>*> slices;
    slices.push_back(&first_level_slices);

    while(!slices.empty()){
        std::vector<Slice> *array_of_slices = slices.back();
        slices.pop_back();

        auto array_id = std::to_string(
                reinterpret_cast<size_t>(&((*array_of_slices)[0]))
                );

        // First we create the node
        myfile << array_id + "[label=\"\n";
        for(auto &slice : *array_of_slices){
            myfile << "<" + std::to_string(reinterpret_cast<size_t>(&slice)) + ">";
            myfile << slice.label();
            myfile << "|";
        }

        myfile << "\"\n];\n";

        // Then we link the nodes
        for(size_t i = 0; i < array_of_slices->size(); ++i){
            auto &slice = array_of_slices->at(i);
            auto slice_id = std::to_string(
                    reinterpret_cast<size_t>(&(slice))
                    );
            if(slice.children.empty()){}
            else{
                myfile << array_id + ":" + array_id + "->" + std::to_string(
                        reinterpret_cast<size_t>(&(slice.children[0]))
                        ) + ";\n";
                slices.push_back(&(slice.children));
            }
        }
    }

    myfile << "\n}";
    myfile.close();
}

bool Quasii::sanity_check_recursion(Slice& slice, vector<pair<float, float>> &borders){
    borders.at(slice.column) = make_pair(slice.left_value, slice.right_value);
    if(slice.column == table->col_count() - 1){
        Query query(borders);
        vector<pair<size_t, size_t>> partition;
        partition.push_back(make_pair(slice.offset_begin, slice.offset_end));
        vector<bool> partition_skip (1, false);
        auto result = FullScan::scan_partition(table.get(), query, partition, partition_skip);
        return result.second == (slice.offset_end - slice.offset_begin);
    }else{
        for(auto& child : slice.children){
            auto result = sanity_check_recursion(child, borders);
            if(result == false)
                return false;
        }
    }
    return true;
}

bool Quasii::sanity_check(){
    for(auto& slice : first_level_slices){
        vector<pair<float, float>> borders(table->col_count());
        auto result = sanity_check_recursion(slice, borders);
        if(result == false)
            return false;
    }
    return true;
}

bool Quasii::has_converged_recursion(Slice& slice){
    if(slice.column == table->col_count() - 1){
        return (slice.offset_end - slice.offset_begin) <= last_level_threshold;
    }else{
        // If we are not a leaf, and have no children then we did not converge
        if(slice.children.size() == 0)
            return false;
        for(auto& child : slice.children){
            auto result = has_converged_recursion(child);
            if(result == false)
                return false;
        }
    }
    return true;
}
bool Quasii::has_converged(){
    for(auto& slice : first_level_slices){
        auto result = has_converged_recursion(slice);
        if(result == false)
            return false;
    }
    return true;
}
