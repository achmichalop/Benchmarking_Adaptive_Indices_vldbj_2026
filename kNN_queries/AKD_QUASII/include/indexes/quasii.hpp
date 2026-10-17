#ifndef QUASII_H
#define QUASII_H

#include "abstract_index.hpp"
#include "full_scan.hpp"
#include "slice.hpp"
#include <map>
#include <string>
#include <cstdint>
#include <limits>

class Quasii : public AbstractIndex
{
public:
    bool should_adapt = true;
    static const size_t ID = 6;
    Quasii(std::map<std::string, std::string> config);
    ~Quasii();

    std::string name() override{
        return "Quasii";
    }

    void initialize(Table *table_to_copy) override;

    void adapt_index(Query &query) override;
    void adapt_index(PointQuery &query, size_t k, size_t factor) override;

    unique_ptr<Table> range_query(Query &query) override;
    maxHeap kNN_query(PointQuery query, size_t k) override;

    void draw_index(std::string path) override;

    bool sanity_check() override;
    bool has_converged();

private:
    std::vector<Slice> first_level_slices;
    size_t sample_size = 20;
    size_t last_level_threshold = 2000;
    std::vector<size_t> dimensions_threshold;

    size_t count_slices(std::vector<Slice> &slices);

    void adapt(Query& query);

    pair<vector<pair<size_t, size_t>>, vector<vector<bool>>>
    search(Query& query);
    int64_t binarySearch(const std::vector<Slice> &slice, float key);
    void search_recursion(
            Slice &slice,
            Query &query,
            vector<pair<size_t, size_t>> &partitions,
            vector<vector<bool>> &partition_skip,
            vector<pair<float, float>> partition_borders
            );
    size_t predicate_index(size_t column, Query &query);

    void calculate_level_thresholds();

    void build(std::vector<Slice> &slices, Query &query);
    void build(std::vector<Slice> &slices, PointQuery &query);
    Query make_box_query(PointQuery query, Table &sample_table);
    Query make_box_query_quantile_based(PointQuery query, Table &sample_table, double p_low, double p_high);
    void get_sample_recursion(Slice& slice, PointQuery& query, Table& sample_table, size_t sample_size);
    void fill_sample_table(size_t lower_limit, size_t upper_limit, Table& sample_table, size_t sample_size);

    std::vector<Slice> sliceArtificial(Slice &slice);

    void sliceArtificialRecursion(Slice& slice, std::vector<Slice>& result);

    std::vector<Slice> sliceTwoWay(Slice &slice, float key);

    std::vector<Slice> sliceThreeWay(Slice &slice, float low, float high);

    std::vector<Slice> refine(Slice &slice, Predicate &predicate);

    bool sanity_check_recursion(Slice& slice, std::vector<std::pair<float, float>> &borders);

    bool has_converged_recursion(Slice&);

    inline Slice createDefaultChild(size_t col, size_t offset_begin, size_t offset_end){
        float min = std::numeric_limits<float>::max();
        float max = numeric_limits<float>::lowest();
        for(size_t i = offset_begin; i < offset_end; ++i){
            auto v = table->columns[col]->data[i];
            if(v < min) min = v;
            if(v > max) max = v;
        }
        return Slice(col, offset_begin, offset_end, min, max);
    }

    void find_neighbors(
        maxHeap &Q, 
        PointQuery query, 
        Slice& slice, 
        double dist, 
        double &worstDist, 
        vector<double> &proj_dists
    );
    
    void update_neighbors_heap(
        maxHeap &Q, 
        PointQuery query, 
        double &worstDist, 
        size_t lower_limit, 
        size_t upper_limit
    );

    double calculate_distance(PointQuery query, std::unique_ptr<float[]>& record);

};
#endif // QUASII_H
