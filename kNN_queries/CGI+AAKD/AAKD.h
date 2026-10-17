#include "../basics.h"
#include "heap.hpp"

struct KDNode{
    KDNode(){
        // queryIDs = new vector<int>();
        last_id++;    
        id = last_id;
    }

    bool operator==(const KDNode& a) const{
        return a.id == id;
    }

    int8_t splitType = 0;
    int axis; // which axis is being split
    int level; // lower is higher, the root is 0
    ELEMTYPE pivotValue = -1; // if it is leaf, then this is meaningless
    bool leafness = true; // is leaf or not
    int dataArrayLeftPointer; // if leaf, this points to the start of the ocntinguous range of data in this leaf, [), inlcuding the left item
    int dataArrayRightPointer; //if leaf, this points to the end of the ocntinguous range of data in this leaf, [), not including the right item
    KDNode* leftChild = NULL; // if inner node, pointer to left child, if leaf, NULL
    KDNode* rightChild = NULL; // if inner node, pointer to right child, if leaf NULL
    int id;
};


class KDNodeHashFunction{
    public:
        size_t operator()(const KDNode *a) const{
            return a->id;
        }
};

class AAKDTree{
    public:
        // constructors
        AAKDTree();
        AAKDTree(ELEMTYPE *data, size_t size, size_t k, size_t factor);
        // Used Methods
        int AdaptiveQuery(ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, Rect cover);
        bool SplitNodeInPlace(KDNode* thisNode, ELEMTYPE pivot, int pivotAxis, int mode);
        maxHeap<NUMDIMS> kNN_query(Point query, size_t k, Rect cover, double &worstDist);
        void kNN_query(maxHeap<NUMDIMS> &Q, Point query, size_t k, Rect cover, double dist, vector<double> proj_dists, double &worstDist);


    private:
        // attributes
        KDNode m_root;
        size_t sample_size = 20;
        ELEMTYPE *dataArr;

        int crackTableLEQPivot(int start, int end, int axis, ELEMTYPE pivot);
        int crackTableGEQPivot(int start, int end, int axis, ELEMTYPE pivot);
        void AdaptiveQuery(KDNode* a_node, ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, int &foundCount, Rect cover);
        int ChooseAxisLargestSideMid(Rect node_cover, Rect query_cover, vector<bool> examinedCoords);
        void Adapt(KDNode* a_node, ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, Rect cover);
        void Adapt(KDNode* a_node, ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, Rect cover, maxHeap<NUMDIMS>&Q, Point query, double &worstDist);


        void get_sample_recursion(KDNode *a_node, Point query, vector<Point> &sample_table, size_t sample_size);
        void fill_sample_table(vector<Point>& sample_table, size_t sample_size, size_t lower_limit, size_t upper_limit);
        Rect make_box_query(Point query, vector<Point> sample_table);
        Rect make_box_query_quantile_based(Point query, vector<Point> sample_table, double p_low, double p_high);
        void find_neighbors(maxHeap<NUMDIMS>& Q, KDNode *a_node, Point query, double dist, double &worstDist, vector<double> &proj_dists);
        void update_neighbors_heap(maxHeap<NUMDIMS> &Q, Point query, double &worstDist, size_t lower_limit, size_t upper_limit);
        double PointToPointDistance(Point& q, float* pt);


};

AAKDTree::AAKDTree(){
    m_root = KDNode();

    m_root.axis = 0;
    m_root.level = 0;
    m_root.leafness = true;
    m_root.dataArrayLeftPointer = 0;
    m_root.dataArrayRightPointer = DATA_COUNT;
    m_root.leftChild = NULL;
    m_root.rightChild = NULL;
    totalNodes++;

    last_id = 0;
    m_root.id = last_id;
}

AAKDTree::AAKDTree(ELEMTYPE *data, size_t size, size_t k, size_t factor){
    m_root = KDNode();

    m_root.axis = 0;
    m_root.level = 0;
    m_root.leafness = true;
    m_root.dataArrayLeftPointer = 0;
    m_root.dataArrayRightPointer = size;
    m_root.leftChild = NULL;
    m_root.rightChild = NULL;

    dataArr = data;

    last_id = 0;
    m_root.id = last_id;

    sample_size = max((size_t)20, factor * k);

}

bool AAKDTree::SplitNodeInPlace(KDNode* thisNode, ELEMTYPE pivot, int pivotAxis, int mode)
{
    #ifdef TIMEBREAKDOWN
        numOfSplits++;
    #endif
    if(thisNode == NULL){
        cout << "thisNode iS null!!!!" << endl;
    }
    if(!thisNode->leafness){
        cout << "this node in split was not leaf, panic" << endl;
        exit(2);
    }

    // cutting on the boundsSet[axis][ans] on axis
    // partition the data in Left and Right of the array

    #ifdef DEBUG
    cout << "Trying to crack piece " << thisNode->dataArrayLeftPointer << " " << thisNode->dataArrayRightPointer << " on " << pivot << " ";
    #endif
    #ifdef TIMEBREAKDOWN
        auto startPartitionTime = clock();
        cerr << "Level is " << thisNode->level << " swaps ";
    #endif

    int crackIndex = (mode) ? crackTableLEQPivot(thisNode->dataArrayLeftPointer, thisNode->dataArrayRightPointer, pivotAxis, pivot) : crackTableGEQPivot(thisNode->dataArrayLeftPointer, thisNode->dataArrayRightPointer, pivotAxis, pivot);
    
    #ifdef TIMEBREAKDOWN
        partitionTime += clock() - startPartitionTime;
        traversals += thisNode->dataArrayRightPointer - thisNode->dataArrayLeftPointer;
    #endif
    
    if(crackIndex == thisNode->dataArrayLeftPointer || crackIndex == thisNode->dataArrayRightPointer){
        #ifdef DEBUG
        cout << " ----- > DIDN'T crack\n";
        #endif
        #ifdef TIMEBREAKDOWN
            failedCracks++;
        #endif
        return false;
    }

    #ifdef DEBUG
    cout << "in splitNode, after if" << endl;
    #endif

    // chnage the axis to the one it was cracked on
    thisNode->axis = pivotAxis;
    thisNode->splitType = (int8_t) ((mode << 1) - 1);

    thisNode->leftChild = new KDNode;
    thisNode->rightChild = new KDNode;

    // cout << "SIZE " << sizeof(KDNode) << endl;

    #ifdef DEBUG
    cout << "in splitNode, after children" << endl;
    #endif

    thisNode->pivotValue = pivot;
    thisNode->leafness = false;

    // and put the crackIndex to the children
    (thisNode->leftChild)->dataArrayLeftPointer = thisNode->dataArrayLeftPointer;
    (thisNode->leftChild)->dataArrayRightPointer = crackIndex;

    (thisNode->rightChild)->dataArrayLeftPointer = crackIndex;
    (thisNode->rightChild)->dataArrayRightPointer = thisNode->dataArrayRightPointer;

    (thisNode->leftChild)->level = thisNode->level + 1;
    (thisNode->rightChild)->level = thisNode->level + 1;

    // maybe they don't need initialising at all TODO
    (thisNode->leftChild)->axis = (thisNode->axis + 1) % NUMDIMS;
    (thisNode->rightChild)->axis = (thisNode->axis + 1) % NUMDIMS;

    
    (thisNode->leftChild)->leafness = true;
    (thisNode->rightChild)->leafness = true;
    totalNodes+=2;
    // then pass thisNode.leftChild and rightChild to each of the recursions
    // a few asserrts: make sure thisNode is leaf -> then make it inner node
    // assert axis of thisNode and axis in function match
    // fix axis of children
    // fix vars for thisNode
    // set uo vars for hte children
    return true;


}

int AAKDTree::crackTableLEQPivot(int start, int end, int axis, ELEMTYPE pivot){


    #ifdef TIMEBREAKDOWN
        int localSwaps = 0;
    #endif
    int i = start, j = end - 1;

    while(i <= j && j > 0){
        if(dataArr[i * NUMDIMS + axis] <= pivot){
            i++;
        }
        else{
            while(j > 0 && j >= i && dataArr[j * NUMDIMS + axis] > pivot){      // TODO can remove j > 0, i think
                j--;
            }
            if(i < j){
                #ifdef TIMEBREAKDOWN
                    clock_t startTime = clock();
                #endif

                for (size_t d = 0; d < NUMDIMS; d++)
                    std::swap(dataArr[i * NUMDIMS + d], dataArr[j * NUMDIMS + d]);

                // std::swap(m_data_ids[i], m_data_ids[j]); // we need to verify whether we need to know the initial IDs or not
                #ifdef TIMEBREAKDOWN
                    swapTime += clock() - startTime;
                #endif
                #ifdef TIMEBREAKDOWN
                    swaps++;
                    localSwaps++;
                #endif
                i++;
                j--; 
            }
        }
    }
    #ifdef TIMEBREAKDOWN
        cerr << localSwaps << " for " << end-start << " data objects" << endl;
    #endif

    return i;

}

int AAKDTree::crackTableGEQPivot(int start, int end, int axis, ELEMTYPE pivot){


    #ifdef TIMEBREAKDOWN
        int localSwaps = 0;
    #endif
    int i = start, j = end - 1;
    while(i <= j && j > 0){

        if(dataArr[i * NUMDIMS + axis] < pivot){
            i++;
        }
        else{
            while(j > 0 && j >= i && dataArr[j * NUMDIMS + axis] >= pivot){      // TODO can remove j > 0, i think
                j--;
            }
            if(i < j){
                #ifdef TIMEBREAKDOWN
                    clock_t startTime = clock();
                #endif
                for (size_t d = 0; d < NUMDIMS; d++)
                    std::swap(dataArr[i * NUMDIMS + d], dataArr[j * NUMDIMS + d]);

                // std::swap(m_data_ids[i], m_data_ids[j]); // we need to verify whether we need to know the initial IDs or not
                #ifdef TIMEBREAKDOWN
                    swapTime += clock() - startTime;
                #endif
                #ifdef TIMEBREAKDOWN
                    swaps++;
                    localSwaps++;
                #endif
                i++;
                j--; 
            }
        }
    }
    #ifdef TIMEBREAKDOWN
        cerr << localSwaps << " for " << end-start << " data objects" << endl;
    #endif

    return i;

}

int AAKDTree::ChooseAxisLargestSideMid(Rect node_cover, Rect query_cover, vector<bool> examinedCoords) {

    float this_ms;
    float current_max_m = (!(examinedCoords[0] && examinedCoords[1])) ? std::abs(node_cover.m_max[0] - node_cover.m_min[0]) : 0;
    int current_max_index = 0;
    for(int i = 1; i < NUMDIMS; i++){
        int offset = i << 1;

        this_ms = (!(examinedCoords[offset] & examinedCoords[offset + 1])) ? std::abs(node_cover.m_max[i] - node_cover.m_min[i]) : 0;

        if(this_ms > current_max_m){
            current_max_m = this_ms;
            current_max_index = i;
        }
    }

    // now we have to figure out if q_min[current_max_index] ids closer to mid of node_cover
    float mid_of_node_cover_axis = (float) ((node_cover.m_max[current_max_index] + node_cover.m_min[current_max_index]) / 2);

    if(std::abs(query_cover.m_min[current_max_index] - mid_of_node_cover_axis) <
    std::abs(query_cover.m_max[current_max_index] - mid_of_node_cover_axis))
    {
        int offset = current_max_index << 1;
        return offset + (int) examinedCoords[offset];
    } 
    else
    {
        int offset = (current_max_index << 1) + 1;
        return offset - (int) examinedCoords[offset];
    }
}

int AAKDTree::AdaptiveQuery(ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, Rect cover){
    int foundCount = 0;

    // first traverse the tree to find the leaves that hold the data in them
    AdaptiveQuery(&m_root, queryMins, queryMaxes, foundCount, cover);
    return foundCount;    
}

void AAKDTree::AdaptiveQuery(KDNode* a_node, ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, int &foundCount, Rect cover){
    
    if (!a_node->leafness)
    {
        // is internal
        // should recurse to children that overlap
        Rect leftCover = Rect();
        Rect rightCover = Rect();
        for(int i = 0; i < NUMDIMS; ++i)
        {
            if(i == a_node->axis)
            {
                leftCover.m_min[i] = cover.m_min[i];
                leftCover.m_max[i] = a_node->pivotValue;

                rightCover.m_min[i] = a_node->pivotValue;
                rightCover.m_max[i] = cover.m_max[i];
            }
            else
            {
                leftCover.m_min[i] = cover.m_min[i];
                leftCover.m_max[i] = cover.m_max[i];

                rightCover.m_min[i] = cover.m_min[i];
                rightCover.m_max[i] = cover.m_max[i];
            }
        }

        switch (a_node->splitType)
        {
            case -1:
            {
                if(queryMins[a_node->axis] < a_node->pivotValue){
                    AdaptiveQuery(a_node->leftChild, queryMins, queryMaxes, foundCount, leftCover);
                }

                if(queryMaxes[a_node->axis] >= a_node->pivotValue){
                    AdaptiveQuery(a_node->rightChild, queryMins, queryMaxes, foundCount, rightCover);
                }

                break;
            }
            case 1:
            {
                if(queryMins[a_node->axis] <= a_node->pivotValue){
                    AdaptiveQuery(a_node->leftChild, queryMins, queryMaxes, foundCount, leftCover);
                }

                if(queryMaxes[a_node->axis] > a_node->pivotValue){
                    AdaptiveQuery(a_node->rightChild, queryMins, queryMaxes, foundCount, rightCover);
                }

                break;
            }
        }

    }
    else    // is a leaf, so we  should crack on the bounds and add to the foundcount
    {
        if(a_node->dataArrayRightPointer - a_node->dataArrayLeftPointer > CRACKTHRESHOLD)
        {
            
            int choice, mm, chosen_axis;
            int numCoords = (NUMDIMS << 1);
            vector<bool> examinedCoords(numCoords, false);
            Rect q = Rect(queryMins, queryMaxes);
            KDNode* thisNode = a_node;

            // then for each of them crack them 2D times
            #ifdef STOCHASTIC
                vector<KDNode*> createdLeaves;
            #endif

            for(int i = 0; i < numCoords; i++)
            {
                choice = ChooseAxisLargestSideMid(cover, q, examinedCoords);
                examinedCoords[choice] = true;
                mm = choice % 2;
                chosen_axis = choice >> 1;

                if(!mm && queryMins[chosen_axis] > cover.m_min[chosen_axis])
                {
                    SplitNodeInPlace(thisNode, queryMins[chosen_axis], chosen_axis, mm);
                    if(!thisNode->leafness) 
                    {
                        #ifdef STOCHASTIC
                            createdLeaves.push_back(thisNode->leftChild);
                        #endif
                        cover.m_min[chosen_axis] = thisNode->pivotValue;
                        thisNode = thisNode->rightChild;

                    }

                    if(thisNode->dataArrayRightPointer - thisNode->dataArrayLeftPointer <= CRACKTHRESHOLD) 
                    {
                        break;
                    }
                }
                else if (mm && queryMaxes[chosen_axis] < cover.m_max[chosen_axis])
                {
                    SplitNodeInPlace(thisNode, queryMaxes[chosen_axis], chosen_axis, mm);
                    if(!thisNode->leafness)
                    {
                        #ifdef STOCHASTIC
                            createdLeaves.push_back(thisNode->rightChild);
                        #endif
                     
                        cover.m_max[chosen_axis] = thisNode->pivotValue;
                        thisNode = thisNode->leftChild;
                    }
                    if(thisNode->dataArrayRightPointer - thisNode->dataArrayLeftPointer <= CRACKTHRESHOLD) 
                    {
                        break;
                    }
                }
            }

            #ifdef STOCHASTIC
                if(createdLeaves.size() > 0)
                {
                    KDNode *nodeToBeCracked = createdLeaves[0];
                    for(int ii = 1; ii < createdLeaves.size(); ++ii)
                    {
                        if(createdLeaves[ii]->dataArrayRightPointer - createdLeaves[ii]->dataArrayLeftPointer > nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer)
                            nodeToBeCracked = createdLeaves[ii];
                    }
                    if(nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer > CRACKTHRESHOLD)
                    {
                        inStoch++;
                        int chosenStochasticAxis = 0;
                        for(int ii = 1; ii < NUMDIMS; ++ii)
                        {
                            if(cover.m_max[ii] - cover.m_min[ii] > cover.m_max[chosenStochasticAxis] - cover.m_min[chosenStochasticAxis])
                                chosenStochasticAxis = ii;
                        }

                        vector<int> rndValues;
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));

                        float median = max(min(m_data_arr[rndValues[0]][chosenStochasticAxis],m_data_arr[rndValues[1]][chosenStochasticAxis]), min(max(m_data_arr[rndValues[0]][chosenStochasticAxis],m_data_arr[rndValues[1]][chosenStochasticAxis]),m_data_arr[rndValues[2]][chosenStochasticAxis]));
                        bool flag = SplitNodeInPlace(nodeToBeCracked, median, chosenStochasticAxis, 0);
                    }
                }
            #endif

            #ifdef TIMEBREAKDOWN
                clock_t start = clock();
            #endif

            #ifdef TIMEBREAKDOWN
                adaptiveSplitTime += clock() - start;
            #endif

            #ifdef TIMEBREAKDOWN
                start = clock();
            #endif

            for(int ii = thisNode->dataArrayLeftPointer; ii < thisNode->dataArrayRightPointer; ++ii){
                size_t offset = ii * NUMDIMS;
                
                if (Overlap(queryMins, queryMaxes, &dataArr[offset]))
                    foundCount++;
            }

            #ifdef TIMEBREAKDOWN
                adaptiveCountingResultsTime += clock() - start;
            #endif
        }
        else{
            #ifdef TIMEBREAKDOWN
                clock_t start = clock();
            #endif

            for (int ii = a_node->dataArrayLeftPointer; ii < a_node->dataArrayRightPointer; ++ii){
                size_t offset = ii * NUMDIMS;

                if (Overlap(queryMins, queryMaxes, &dataArr[offset]))
                    foundCount++;
            }

            #ifdef TIMEBREAKDOWN
                adaptiveCountingResultsTime += clock() - start;
            #endif
        }
        
    }
}

maxHeap<NUMDIMS> AAKDTree::kNN_query(Point query, size_t k, Rect cover, double &worstDist)
{

    if (sample_size >= (m_root.dataArrayRightPointer - m_root.dataArrayLeftPointer))
    {
        maxHeap<NUMDIMS> Q(k);
        double dist = 0;
        vector<double> proj_dists(NUMDIMS, 0.0);

        find_neighbors(Q, &m_root, query, dist, worstDist, proj_dists);
        return Q;
    }

    vector<Point> sample_table;
    sample_table.reserve(sample_size);
    get_sample_recursion(&m_root, query, sample_table, sample_size);

    // Rect boxQuery = make_box_query(query, sample_table);

    // ## Quantile-side approaches without p_low = p_high = 0.5 ##
    Rect boxQuery = make_box_query_quantile_based(query, sample_table, 3.0/4.0, 1.0/4.0);   // p_low = 3/4,  p_high = 1/4

    Adapt(&m_root, boxQuery.m_min, boxQuery.m_max, cover);

    maxHeap<NUMDIMS> Q(k);
    double dist = 0;
    vector<double> proj_dists(NUMDIMS, 0.0);

    find_neighbors(Q, &m_root, query, dist, worstDist, proj_dists);

    // cout << "Examined Regions: " << examined_regions << " -- Examined Points: " << examined_points << endl;
    // examined_regions = examined_points = 0;


    return Q;
}

void AAKDTree::kNN_query(maxHeap<NUMDIMS> &Q, Point query, size_t k, Rect cover, double dist, vector<double> proj_dists, double &worstDist)
{
    double radius = sqrt(worstDist);

    ELEMTYPE minPoint[NUMDIMS], maxPoint[NUMDIMS];
    for (size_t d = 0; d < NUMDIMS; d++)
    {
        minPoint[d] = max(cover.m_min[d], static_cast<float>(query.m_values[d] - radius));
        maxPoint[d] = min(cover.m_max[d], static_cast<float>(query.m_values[d] + radius));
    }
    Rect boxQuery = Rect(minPoint, maxPoint);

    Adapt(&m_root, boxQuery.m_min, boxQuery.m_max, cover, Q, query, worstDist);
}

void AAKDTree::Adapt(KDNode* a_node, ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, Rect cover){
    
    if (!a_node->leafness)
    {
        // is internal
        // should recurse to children that overlap
        Rect leftCover = Rect();
        Rect rightCover = Rect();
        for(int i = 0; i < NUMDIMS; ++i)
        {
            if(i == a_node->axis)
            {
                leftCover.m_min[i] = cover.m_min[i];
                leftCover.m_max[i] = a_node->pivotValue;

                rightCover.m_min[i] = a_node->pivotValue;
                rightCover.m_max[i] = cover.m_max[i];
            }
            else
            {
                leftCover.m_min[i] = cover.m_min[i];
                leftCover.m_max[i] = cover.m_max[i];

                rightCover.m_min[i] = cover.m_min[i];
                rightCover.m_max[i] = cover.m_max[i];
            }
        }

        switch (a_node->splitType)
        {
            case -1:
            {
                if(queryMins[a_node->axis] < a_node->pivotValue){
                    Adapt(a_node->leftChild, queryMins, queryMaxes, leftCover);
                }

                if(queryMaxes[a_node->axis] >= a_node->pivotValue){
                    Adapt(a_node->rightChild, queryMins, queryMaxes, rightCover);
                }

                break;
            }
            case 1:
            {
                if(queryMins[a_node->axis] <= a_node->pivotValue){
                    Adapt(a_node->leftChild, queryMins, queryMaxes, leftCover);
                }

                if(queryMaxes[a_node->axis] > a_node->pivotValue){
                    Adapt(a_node->rightChild, queryMins, queryMaxes, rightCover);
                }

                break;
            }
        }
    }
    else    // is a leaf, so we  should crack on the bounds and add to the foundcount
    {
        if(a_node->dataArrayRightPointer - a_node->dataArrayLeftPointer > CRACKTHRESHOLD)
        {
            
            int choice, mm, chosen_axis;
            int numCoords = (NUMDIMS << 1);
            vector<bool> examinedCoords(numCoords, false);
            Rect q = Rect(queryMins, queryMaxes);
            KDNode* thisNode = a_node;

            // then for each of them crack them 2D times
            #ifdef STOCHASTIC
                vector<KDNode*> createdLeaves;
            #endif

            for(int i = 0; i < numCoords; i++)
            {
                choice = ChooseAxisLargestSideMid(cover, q, examinedCoords);
                examinedCoords[choice] = true;
                mm = choice % 2;
                chosen_axis = choice >> 1;

                if(!mm && queryMins[chosen_axis] > cover.m_min[chosen_axis])
                {
                    SplitNodeInPlace(thisNode, queryMins[chosen_axis], chosen_axis, mm);
                    if(!thisNode->leafness) 
                    {
                        #ifdef STOCHASTIC
                            if (thisNode->leftChild->dataArrayRightPointer - thisNode->leftChild->dataArrayLeftPointer > CRACKTHRESHOLD)
                                createdLeaves.push_back(thisNode->leftChild);
                        #endif
                        cover.m_min[chosen_axis] = thisNode->pivotValue;
                        thisNode = thisNode->rightChild;
                    }
                    if(thisNode->dataArrayRightPointer - thisNode->dataArrayLeftPointer <= CRACKTHRESHOLD) 
                    {
                        break;
                    }
                }
                else if (mm && queryMaxes[chosen_axis] < cover.m_max[chosen_axis])
                {
                    SplitNodeInPlace(thisNode, queryMaxes[chosen_axis], chosen_axis, mm);
                    if(!thisNode->leafness)
                    {
                        #ifdef STOCHASTIC
                            if (thisNode->rightChild->dataArrayRightPointer - thisNode->rightChild->dataArrayLeftPointer > CRACKTHRESHOLD)
                                createdLeaves.push_back(thisNode->rightChild);
                        #endif
                     
                        cover.m_max[chosen_axis] = thisNode->pivotValue;
                        thisNode = thisNode->leftChild;

                    }

                    if(thisNode->dataArrayRightPointer - thisNode->dataArrayLeftPointer <= CRACKTHRESHOLD) 
                    {
                        break;
                    }
                }
            }

            #ifdef STOCHASTIC
                if(createdLeaves.size() > 0)
                {
                    KDNode *nodeToBeCracked = createdLeaves[0];
                    for(int ii = 1; ii < createdLeaves.size(); ++ii)
                    {
                        if(createdLeaves[ii]->dataArrayRightPointer - createdLeaves[ii]->dataArrayLeftPointer > nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer)
                            nodeToBeCracked = createdLeaves[ii];
                    }

                    // if(nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer > CRACKTHRESHOLD)
                    // {
                        inStoch++;
                        int chosenStochasticAxis = 0;
                        for(int ii = 1; ii < NUMDIMS; ++ii)
                        {
                            if(cover.m_max[ii] - cover.m_min[ii] > cover.m_max[chosenStochasticAxis] - cover.m_min[chosenStochasticAxis])
                                chosenStochasticAxis = ii;
                        }

                        vector<int> rndValues;
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));

                        float median = max(min(m_data_arr[rndValues[0]][chosenStochasticAxis],m_data_arr[rndValues[1]][chosenStochasticAxis]), min(max(m_data_arr[rndValues[0]][chosenStochasticAxis],m_data_arr[rndValues[1]][chosenStochasticAxis]),m_data_arr[rndValues[2]][chosenStochasticAxis]));
                        bool flag = SplitNodeInPlace(nodeToBeCracked, median, chosenStochasticAxis, 0);
                    // }
                }
            #endif

            #ifdef TIMEBREAKDOWN
                clock_t start = clock();
            #endif

            #ifdef TIMEBREAKDOWN
                adaptiveSplitTime += clock() - start;
            #endif

            #ifdef TIMEBREAKDOWN
                start = clock();
            #endif

            #ifdef TIMEBREAKDOWN
                adaptiveCountingResultsTime += clock() - start;
            #endif
        }        
    }
}


void AAKDTree::Adapt(KDNode* a_node, ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, Rect cover, maxHeap<NUMDIMS>&Q, Point query, double &worstDist){
    
    if (!a_node->leafness)
    {
        // is internal
        // should recurse to children that overlap
        Rect leftCover = Rect();
        Rect rightCover = Rect();
        for(int i = 0; i < NUMDIMS; ++i)
        {
            if(i == a_node->axis)
            {
                leftCover.m_min[i] = cover.m_min[i];
                leftCover.m_max[i] = a_node->pivotValue;

                rightCover.m_min[i] = a_node->pivotValue;
                rightCover.m_max[i] = cover.m_max[i];
            }
            else
            {
                leftCover.m_min[i] = cover.m_min[i];
                leftCover.m_max[i] = cover.m_max[i];

                rightCover.m_min[i] = cover.m_min[i];
                rightCover.m_max[i] = cover.m_max[i];
            }
        }

        switch (a_node->splitType)
        {
            case -1:
            {
                if(queryMins[a_node->axis] < a_node->pivotValue){
                    Adapt(a_node->leftChild, queryMins, queryMaxes, leftCover, Q, query, worstDist);
                }

                if(queryMaxes[a_node->axis] >= a_node->pivotValue){
                    Adapt(a_node->rightChild, queryMins, queryMaxes, rightCover, Q, query, worstDist);
                }

                break;
            }
            case 1:
            {
                if(queryMins[a_node->axis] <= a_node->pivotValue){
                    Adapt(a_node->leftChild, queryMins, queryMaxes, leftCover, Q, query, worstDist);
                }

                if(queryMaxes[a_node->axis] > a_node->pivotValue){
                    Adapt(a_node->rightChild, queryMins, queryMaxes, rightCover, Q, query, worstDist);
                }

                break;
            }
        }
    }
    else    // is a leaf, so we  should crack on the bounds and add to the foundcount
    {
        if(a_node->dataArrayRightPointer - a_node->dataArrayLeftPointer > CRACKTHRESHOLD)
        {
            
            int choice, mm, chosen_axis;
            int numCoords = (NUMDIMS << 1);
            vector<bool> examinedCoords(numCoords, false);
            Rect q = Rect(queryMins, queryMaxes);
            KDNode* thisNode = a_node;

            // then for each of them crack them 2D times
            #ifdef STOCHASTIC
                vector<KDNode*> createdLeaves;
            #endif

            for(int i = 0; i < numCoords; i++)
            {
                choice = ChooseAxisLargestSideMid(cover, q, examinedCoords);
                examinedCoords[choice] = true;
                mm = choice % 2;
                chosen_axis = choice >> 1;

                if(!mm && queryMins[chosen_axis] > cover.m_min[chosen_axis])
                {
                    SplitNodeInPlace(thisNode, queryMins[chosen_axis], chosen_axis, mm);
                    if(!thisNode->leafness) 
                    {
                        #ifdef STOCHASTIC
                            if (thisNode->leftChild->dataArrayRightPointer - thisNode->leftChild->dataArrayLeftPointer > CRACKTHRESHOLD)
                                createdLeaves.push_back(thisNode->leftChild);
                        #endif
                        cover.m_min[chosen_axis] = thisNode->pivotValue;
                        thisNode = thisNode->rightChild;
                    }
                    if(thisNode->dataArrayRightPointer - thisNode->dataArrayLeftPointer <= CRACKTHRESHOLD) 
                    {
                        break;
                    }
                }
                else if (mm && queryMaxes[chosen_axis] < cover.m_max[chosen_axis])
                {
                    SplitNodeInPlace(thisNode, queryMaxes[chosen_axis], chosen_axis, mm);
                    if(!thisNode->leafness)
                    {
                        #ifdef STOCHASTIC
                            if (thisNode->rightChild->dataArrayRightPointer - thisNode->rightChild->dataArrayLeftPointer > CRACKTHRESHOLD)
                                createdLeaves.push_back(thisNode->rightChild);
                        #endif
                     
                        cover.m_max[chosen_axis] = thisNode->pivotValue;
                        thisNode = thisNode->leftChild;

                    }

                    if(thisNode->dataArrayRightPointer - thisNode->dataArrayLeftPointer <= CRACKTHRESHOLD) 
                    {
                        break;
                    }
                }
            }

            #ifdef STOCHASTIC
                if(createdLeaves.size() > 0)
                {
                    KDNode *nodeToBeCracked = createdLeaves[0];
                    for(int ii = 1; ii < createdLeaves.size(); ++ii)
                    {
                        if(createdLeaves[ii]->dataArrayRightPointer - createdLeaves[ii]->dataArrayLeftPointer > nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer)
                            nodeToBeCracked = createdLeaves[ii];
                    }

                    // if(nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer > CRACKTHRESHOLD)
                    // {
                        inStoch++;
                        int chosenStochasticAxis = 0;
                        for(int ii = 1; ii < NUMDIMS; ++ii)
                        {
                            if(cover.m_max[ii] - cover.m_min[ii] > cover.m_max[chosenStochasticAxis] - cover.m_min[chosenStochasticAxis])
                                chosenStochasticAxis = ii;
                        }

                        vector<int> rndValues;
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));
                        rndValues.push_back(nodeToBeCracked->dataArrayLeftPointer + ( rand() % (nodeToBeCracked->dataArrayRightPointer - nodeToBeCracked->dataArrayLeftPointer + 1 )));

                        float median = max(min(m_data_arr[rndValues[0]][chosenStochasticAxis],m_data_arr[rndValues[1]][chosenStochasticAxis]), min(max(m_data_arr[rndValues[0]][chosenStochasticAxis],m_data_arr[rndValues[1]][chosenStochasticAxis]),m_data_arr[rndValues[2]][chosenStochasticAxis]));
                        bool flag = SplitNodeInPlace(nodeToBeCracked, median, chosenStochasticAxis, 0);
                    // }
                }
            #endif

            #ifdef TIMEBREAKDOWN
                clock_t start = clock();
            #endif

            #ifdef TIMEBREAKDOWN
                adaptiveSplitTime += clock() - start;
            #endif

            #ifdef TIMEBREAKDOWN
                start = clock();
            #endif
            
            
            update_neighbors_heap(Q, query, worstDist, thisNode->dataArrayLeftPointer, thisNode->dataArrayRightPointer);

            #ifdef TIMEBREAKDOWN
                adaptiveCountingResultsTime += clock() - start;
            #endif
        }
        else
        {
            update_neighbors_heap(Q, query, worstDist, a_node->dataArrayLeftPointer, a_node->dataArrayRightPointer);
        }
    }
}

void AAKDTree::get_sample_recursion(KDNode *a_node, Point query, vector<Point> &sample_table, size_t sample_size)
{
    if (sample_table.size() >= sample_size)
        return;

    if (a_node->leafness)
    {
        fill_sample_table(sample_table, sample_size, a_node->dataArrayLeftPointer, a_node->dataArrayRightPointer);
        return;
    }

    size_t dim = a_node->axis;
    ELEMTYPE val = query.m_values[dim];

    switch (a_node->splitType)
    {
        case -1:
        {
            if (val < a_node->pivotValue)
            {
                get_sample_recursion(a_node->leftChild, query, sample_table, sample_size);
                get_sample_recursion(a_node->rightChild, query, sample_table, sample_size);
            }
            else
            {
                get_sample_recursion(a_node->rightChild, query, sample_table, sample_size);
                get_sample_recursion(a_node->leftChild, query, sample_table, sample_size);
            }

            break;
        }
        case 1:
        {
            if (val <= a_node->pivotValue)
            {
                get_sample_recursion(a_node->leftChild, query, sample_table, sample_size);
                get_sample_recursion(a_node->rightChild, query, sample_table, sample_size);
            }
            else
            {
                get_sample_recursion(a_node->rightChild, query, sample_table, sample_size);
                get_sample_recursion(a_node->leftChild, query, sample_table, sample_size);
            }
            break;
        }
    }
}

void AAKDTree::fill_sample_table(vector<Point>& sample_table, size_t sample_size, size_t lower_limit, size_t upper_limit)
{
    size_t partition_size = upper_limit - lower_limit;
    size_t residue = sample_size - sample_table.size();
    size_t step = (partition_size <= residue) ? 1 : ceil(partition_size / (double)residue);
    size_t numIters = (partition_size < residue) ? partition_size : residue;
    
    // cout << "Partition Size: " << partition_size << " -- " 
    //      << "Lower: " << lower_limit << " -- "
    //      << "Upper: " << upper_limit << " -- "
    //      << "Filled: " << sample_table.size() << " -- "
    //      << "Residue: " << residue << endl;

    for (size_t i = 0, index = lower_limit; i < numIters; i++, index+=step)
    {
        size_t offset = index * NUMDIMS;
        ELEMTYPE pt[NUMDIMS];
        for (size_t d = 0; d < NUMDIMS; d++)
        {
            pt[d] = dataArr[offset + d];
        }

        sample_table.emplace_back(Point(pt));
    }
    
}

Rect AAKDTree::make_box_query(Point query, vector<Point> sample_table)
{
    vector<vector<ELEMTYPE>> minValsPerDim(NUMDIMS), maxValsPerDim(NUMDIMS);

    for (size_t i = 0; i < sample_table.size(); i++)
    {
        Point &r = sample_table[i];

        for (size_t d = 0; d < NUMDIMS; d++)
        {
            if (r.m_values[d] < query.m_values[d])
            {
                minValsPerDim[d].push_back(r.m_values[d]);
            }
            else
            {
                maxValsPerDim[d].push_back(r.m_values[d]);
            }
        }
    }

    ELEMTYPE boxMins[NUMDIMS];
    ELEMTYPE boxMaxes[NUMDIMS];

    for (size_t d = 0; d < NUMDIMS; d++)
    {
        if (!minValsPerDim[d].empty() && !maxValsPerDim[d].empty())
        {
            size_t medianPosInMins = minValsPerDim[d].size() >> 1;
            size_t medianPosInMaxes = maxValsPerDim[d].size() >> 1;

            nth_element(minValsPerDim[d].begin(), minValsPerDim[d].begin() + medianPosInMins, minValsPerDim[d].end());
            nth_element(maxValsPerDim[d].begin(), maxValsPerDim[d].begin() + medianPosInMaxes, maxValsPerDim[d].end());

            boxMins[d] = minValsPerDim[d][medianPosInMins];
            boxMaxes[d] = maxValsPerDim[d][medianPosInMaxes];
        }
        else if (minValsPerDim[d].empty() && !maxValsPerDim[d].empty())
        {
            size_t medianPosInMaxes = maxValsPerDim[d].size() >> 1;
            nth_element(maxValsPerDim[d].begin(), maxValsPerDim[d].begin() + medianPosInMaxes, maxValsPerDim[d].end());
            float medMax = maxValsPerDim[d][medianPosInMaxes];
            boxMins[d] = max(0.0f, query.m_values[d] - (medMax - query.m_values[d]));
            boxMaxes[d] = medMax;
        }
        else if (!minValsPerDim[d].empty() && maxValsPerDim[d].empty())
        {
            size_t medianPosInMins = minValsPerDim[d].size() >> 1;
            nth_element(minValsPerDim[d].begin(), minValsPerDim[d].begin() + medianPosInMins, minValsPerDim[d].end());
            float medMin = minValsPerDim[d][medianPosInMins];
            boxMins[d] = medMin;
            boxMaxes[d] = min(1.0f, query.m_values[d] + (query.m_values[d] - medMin));

        }
        else
        {
            float offset = query.m_values[d] / 2;
            boxMins[d] = max(0.0f, query.m_values[d] - offset);
            boxMaxes[d] = min(1.0f, query.m_values[d] + offset);
        }
    }

    Rect boxQuery(boxMins, boxMaxes);

    return boxQuery;
}

Rect AAKDTree::make_box_query_quantile_based(Point query, vector<Point> sample_table, double p_low, double p_high)
{
    vector<vector<ELEMTYPE>> lowVals(NUMDIMS), highVals(NUMDIMS);

    for (size_t i = 0; i < sample_table.size(); i++)
    {
        Point &r = sample_table[i];

        for (size_t d = 0; d < NUMDIMS; d++)
        {
            if (r.m_values[d] < query.m_values[d])
            {
                lowVals[d].push_back(r.m_values[d]);
            }
            else
            {
                highVals[d].push_back(r.m_values[d]);
            }
        }
    }

    ELEMTYPE boxMins[NUMDIMS];
    ELEMTYPE boxMaxes[NUMDIMS];

    for (size_t d = 0; d < NUMDIMS; d++)
    {
        if (!lowVals[d].empty() && !highVals[d].empty())
        {

            size_t lowPos = static_cast<size_t>(lowVals[d].size() * p_low);
            size_t highPos = static_cast<size_t>(highVals[d].size() * p_high);

            lowPos = min(lowPos, lowVals[d].size()-1);
            highPos = min(highPos, highVals[d].size()-1);

            nth_element(lowVals[d].begin(), lowVals[d].begin() + lowPos, lowVals[d].end());
            nth_element(highVals[d].begin(), highVals[d].begin() + highPos, highVals[d].end());

            boxMins[d] = lowVals[d][lowPos];
            boxMaxes[d] = highVals[d][highPos];
        }
        else if (lowVals[d].empty() && !highVals[d].empty())
        {
            size_t highPos = static_cast<size_t>(p_high * highVals[d].size());
            highPos = min(highPos, highVals[d].size()-1);
            
            nth_element(highVals[d].begin(), highVals[d].begin() + highPos, highVals[d].end());
            ELEMTYPE selectedVal = highVals[d][highPos];
            boxMins[d] = max(0.0f, query.m_values[d] - (selectedVal - query.m_values[d]));
            boxMaxes[d] = selectedVal;
        }
        else if (!lowVals[d].empty() && highVals[d].empty())
        {
            size_t lowPos = static_cast<size_t>(p_low * lowVals[d].size());
            lowPos = min(lowPos, lowVals[d].size()-1);
            nth_element(lowVals[d].begin(), lowVals[d].begin() + lowPos, lowVals[d].end());

            ELEMTYPE selectedVal = lowVals[d][lowPos];
            boxMins[d] = selectedVal;
            boxMaxes[d] = min(1.0f, query.m_values[d] + (query.m_values[d] - selectedVal));

        }
        else
        {
            float offset = query.m_values[d] / 2;
            boxMins[d] = max(0.0f, query.m_values[d] - offset);
            boxMaxes[d] = min(1.0f, query.m_values[d] + offset);
        }
    }

    Rect boxQuery(boxMins, boxMaxes);

    return boxQuery;
}

void AAKDTree::find_neighbors(maxHeap<NUMDIMS>& Q, KDNode *a_node, Point query, 
                    double dist, double &worstDist, vector<double> &proj_dists)
{
 
    if (a_node->leafness)
    {
        update_neighbors_heap(Q, query, worstDist, a_node->dataArrayLeftPointer, a_node->dataArrayRightPointer);
        return;
    }

    size_t dim = a_node->axis;
    ELEMTYPE val = query.m_values[dim];
    double diff = val - a_node->pivotValue;
    double new_proj = diff * diff;
    KDNode* bestChild, *otherChild;


    switch (a_node->splitType)
    {
        case -1:
        {
            bestChild = (diff < 0) ? a_node->leftChild : a_node->rightChild;
            otherChild = (diff < 0) ? a_node->rightChild : a_node->leftChild;
            break;
        }
        case 1:
        {
            bestChild = (diff > 0) ? a_node->rightChild : a_node->leftChild;
            otherChild = (diff > 0) ? a_node->leftChild : a_node->rightChild;
            break;
        }
    }

    find_neighbors(Q, bestChild, query, dist, worstDist, proj_dists);

    double old_proj = proj_dists[dim];
    dist = dist + new_proj - old_proj;
    proj_dists[dim] = new_proj;


    if (Q.size() == Q.capacity && dist >= worstDist)
    {
        proj_dists[dim] = old_proj;
        return;
    }

    find_neighbors(Q, otherChild, query, dist, worstDist, proj_dists);
    proj_dists[dim] = old_proj;
}

void AAKDTree::update_neighbors_heap(maxHeap<NUMDIMS> &Q, Point query, double &worstDist, size_t lower_limit, size_t upper_limit)
{
    // examined_points += upper_limit - lower_limit;
    // examined_regions++;
    for (size_t index = lower_limit; index < upper_limit; index++)
    {
        size_t offset = index * NUMDIMS;
        ELEMTYPE pt[NUMDIMS];
        for (size_t d = 0; d < NUMDIMS; d++)
        {
            pt[d] = dataArr[offset + d];
        }
        double dist = PointToPointDistance(query, pt);
        Q.insert(maxHeapNode<NUMDIMS>(dist, pt));
    }

    if (Q.size() == Q.capacity)
        worstDist = Q.top().dist;
}

double AAKDTree::PointToPointDistance(Point& q, float* pt)
{
    double dist = 0.0;
    for (size_t d = 0; d < NUMDIMS; d++)
    {
        double diff = q.m_values[d] - pt[d];
        dist += diff * diff;
    }

    return dist;
}

