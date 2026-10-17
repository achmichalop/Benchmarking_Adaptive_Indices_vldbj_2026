#ifndef CRACKING_KD_TREE_HPP
#define CRACKING_KD_TREE_HPP

#include "kd_tree/kd_tree.hpp"
#include "full_scan.hpp"
#include "abstract_index.hpp"
#include <string>
#include <map>
#include <set>
#include <algorithm>


class CrackingKDTree : public AbstractIndex
{
    public:
    bool should_adapt = true;
    static const size_t ID = 2;
    CrackingKDTree(std::map<std::string, std::string> config);
    ~CrackingKDTree();

    std::string name() override{
        return "CrackingKDTree";
    }

    void initialize(Table *table_to_copy) override;

    void adapt_index(Query &query) override;
    void adapt_index(PointQuery &query, size_t k, size_t factor) override;

    unique_ptr<Table> range_query(Query &query) override;
    maxHeap kNN_query(PointQuery query, size_t k) override;

    void draw_index(std::string path) override{
        index->draw(path);
    }

    bool sanity_check() override{
        return index->sanity_check(table.get());
    }

private:
    std::unique_ptr<KDTree> index;
    size_t minimum_partition_size = 100;
    size_t sample_size = 20;
    void adapt(Query& query);
    void adapt(PointQuery& query);
    bool find_pivot(size_t lower_limit, size_t upper_limit, float queryCoord, 
                            int mode, float &pivot, size_t pivot_dim);

        
    KDNode* find_partition(KDNode * current, size_t &lower_limit, 
                            size_t &upper_limit, PointQuery& query, bool &selected_side);

    Query make_box_query(PointQuery query, Table &sample_table);
    Query make_box_query_quantile_based(PointQuery query, Table &sample_table, 
                                        double p_low, double p_high);
    void adapt_recursion(
        KDNode *current, Query& query,
        size_t pivot_dim, float pivot, size_t lower_limit, size_t upper_limit
    );

    void find_neighbors(
        maxHeap &Q, 
        PointQuery query, 
        KDNode *current, 
        double dist, 
        double &worstDist, 
        vector<double> &proj_dists, 
        size_t lower_limit, 
        size_t upper_limit
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


#endif
