#ifndef INDEX_FACTORY_H
#define INDEX_FACTORY_H

#include "abstract_index.hpp"
#include "full_scan.hpp"
#include "full_scan_candidate_list.hpp"
#include "full_scan_slow.hpp"
#include "quasii.hpp"
#include "cracking_kd_tree.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stdexcept>

using namespace std;

class IndexFactory
{
public:
    static unique_ptr<AbstractIndex> getIndex(
            size_t algorithm_id,
            map<string, string> config = map<string, string>()
        ){
        switch(algorithm_id){
            case FullScan::ID:
                return make_unique<FullScan>(config);
            case FullScanCandidateList::ID:
                return make_unique<FullScanCandidateList>(config);
            case CrackingKDTree::ID:
                return make_unique<CrackingKDTree>(config);
            case Quasii::ID:
                return make_unique<Quasii>(config);
            default:
                throw std::invalid_argument("Invalid Algorithm ID");
                assert(false);
        }
    }

    static vector<size_t> algorithmIDs(){
        return {
                FullScan::ID,
                CrackingKDTree::ID,
                Quasii::ID
        };
    }

    static shared_ptr<AbstractIndex> baselineIndex(
            map<string, string> config = map<string, string>()
        ){
        return make_unique<FullScanSlow>(config);
    }
};
#endif // INDEX_FACTORY_H
