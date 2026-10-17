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

        // Used Methods
        int AdaptiveQuery(Rect &extendedQuery, Rect &originalQuery, Rect cover);
        bool SplitNodeInPlace(KDNode* thisNode, ELEMTYPE pivot, int pivotAxis, int mode);

    private:
        // attributes
        KDNode m_root;

        int crackTableLEQPivot(int start, int end, int axis, ELEMTYPE pivot);
        int crackTableGEQPivot(int start, int end, int axis, ELEMTYPE pivot);
        void AdaptiveQuery(KDNode* a_node, ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, int &foundCount, vector<Rect> &candidates, Rect cover);
        int ChooseAxisLargestSideMid(Rect node_cover, Rect query_cover, vector<bool> examinedCoords);
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
        if(m_data_arr_rects[i].m_min[axis] <= pivot){
            i++;
        }
        else{
            while(j > 0 && j >= i && m_data_arr_rects[j].m_min[axis] > pivot){      // TODO can remove j > 0, i think
                j--;
            }
            if(i < j){
                #ifdef TIMEBREAKDOWN
                    clock_t startTime = clock();
                #endif

                std::swap(m_data_arr_rects[i], m_data_arr_rects[j]);
                std::swap(m_data_ids[i], m_data_ids[j]);
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

        if(m_data_arr_rects[i].m_min[axis] < pivot){
            i++;
        }
        else{
            while(j > 0 && j >= i && m_data_arr_rects[j].m_min[axis] >= pivot){      // TODO can remove j > 0, i think
                j--;
            }
            if(i < j){
                #ifdef TIMEBREAKDOWN
                    clock_t startTime = clock();
                #endif
                std::swap(m_data_arr_rects[i], m_data_arr_rects[j]);
                std::swap(m_data_ids[i], m_data_ids[j]);
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

// int AAKDTree::AdaptiveQuery(ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, Rect cover){
//     int foundCount = 0;

//     // first traverse the tree to find the leaves that hold the data in them
//     AdaptiveQuery(&m_root, queryMins, queryMaxes, foundCount, cover);
//     return foundCount;    
// }

int AAKDTree::AdaptiveQuery(Rect &extendedQuery, Rect &originalQuery, Rect cover){
    int foundCount = 0;
    // first traverse the tree to find the leaves that hold the data in them
    vector<Rect> candidates;
    AdaptiveQuery(&m_root, extendedQuery.m_min, extendedQuery.m_max, foundCount, candidates, cover);

    int refinedResults = 0;
    for (auto c : candidates)
    {
        if (Overlap(originalQuery, c)){
            refinedResults++;
        }

    }

    return refinedResults;    
}

void AAKDTree::AdaptiveQuery(KDNode* a_node, ELEMTYPE *queryMins, ELEMTYPE *queryMaxes, int &foundCount, vector<Rect> &candidates, Rect cover){
    
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
                    AdaptiveQuery(a_node->leftChild, queryMins, queryMaxes, foundCount, candidates, leftCover);
                }

                if(queryMaxes[a_node->axis] >= a_node->pivotValue){
                    AdaptiveQuery(a_node->rightChild, queryMins, queryMaxes, foundCount, candidates, rightCover);
                }

                break;
            }
            case 1:
            {
                if(queryMins[a_node->axis] <= a_node->pivotValue){
                    AdaptiveQuery(a_node->leftChild, queryMins, queryMaxes, foundCount, candidates, leftCover);
                }

                if(queryMaxes[a_node->axis] > a_node->pivotValue){
                    AdaptiveQuery(a_node->rightChild, queryMins, queryMaxes, foundCount, candidates, rightCover);
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

                        float median = max(min(m_data_arr_rects[rndValues[0]].m_min[chosenStochasticAxis],m_data_arr_rects[rndValues[1]].m_min[chosenStochasticAxis]), min(max(m_data_arr_rects[rndValues[0]].m_min[chosenStochasticAxis],m_data_arr_rects[rndValues[1]].m_min[chosenStochasticAxis]),m_data_arr_rects[rndValues[2]].m_min[chosenStochasticAxis]));
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

            for(int ii = thisNode->dataArrayLeftPointer; ii < thisNode->dataArrayRightPointer; ++ii){
                if(Overlap(queryMins, queryMaxes, m_data_arr_rects[ii])){
                    candidates.emplace_back(m_data_arr_rects[ii]);
                    foundCount++;
                }
            }
            #ifdef TIMEBREAKDOWN
                adaptiveCountingResultsTime += clock() - start;
            #endif
        }
        else{
            #ifdef TIMEBREAKDOWN
                clock_t start = clock();
            #endif
            for(int ii = a_node->dataArrayLeftPointer; ii < a_node->dataArrayRightPointer; ++ii){
                if(Overlap(queryMins, queryMaxes, m_data_arr_rects[ii])){
                    candidates.emplace_back(m_data_arr_rects[ii]);
                    foundCount++;
                }
            }
            #ifdef TIMEBREAKDOWN
                adaptiveCountingResultsTime += clock() - start;
            #endif

        }
        
    }
}
