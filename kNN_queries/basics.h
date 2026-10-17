#ifndef BASICS_H_
#define BASICS_H_


#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <set>
#include <unordered_map>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <cstdlib>

#include <algorithm>
#include <functional>
#include <vector>

#include <utility>
#include <queue>
#include <string>
#include <chrono>
#include <string.h>
#include <forward_list>
#include <map>



using namespace std;
using namespace std::chrono;

typedef int ValueType;
typedef float ELEMTYPE;
typedef double ELEMTYPEREAL;

#define EPS 1e-08

int falsePositives = 0;
int totalRes = 0;

#ifdef sample100
    #define SAMPLERATE 100
#elif sample1
    #define SAMPLERATE 1
#elif sample10
    #define SAMPLERATE 10
#elif sample50
    #define SAMPLERATE 50
#elif sample100
    #define SAMPLERATE 100
#elif sample500
    #define SAMPLERATE 500
#elif sample1000
    #define SAMPLERATE 1000
#elif sample10000
    #define SAMPLERATE 10000
#elif sample100000
    #define SAMPLERATE 100000
#endif


// Synthetic Data
#ifdef datacount20M
    #define DATA_COUNT 20000000
#elif datacount40M
    #define DATA_COUNT 40000000
#elif datacount60M
    #define DATA_COUNT 60000000
#elif datacount80M
    #define DATA_COUNT 80000000
#elif datacountRoads // Real Data
    #define DATA_COUNT 19349214
#elif datacountYellow_nyt
    #define DATA_COUNT 21154022
#endif


#ifdef DIM2
    #define NUMDIMS 2
#elif DIM3
    #define NUMDIMS 3
#elif DIM4
    #define NUMDIMS 4
#elif DIM5
    #define NUMDIMS 5
#elif DIM6
    #define NUMDIMS 6
#elif DIM7
    #define NUMDIMS 7
#elif DIM8
    #define NUMDIMS 8
#endif

#ifdef thresh32
    #define CRACKTHRESHOLD 32
#elif thresh64
    #define CRACKTHRESHOLD 64
#elif thresh128
    #define CRACKTHRESHOLD 128
#elif thresh256
    #define CRACKTHRESHOLD 256
#elif thresh512
    #define CRACKTHRESHOLD 512
#elif thresh1024
    #define CRACKTHRESHOLD 1024
#elif thresh2048
    #define CRACKTHRESHOLD 2048
#elif thresh4096
    #define CRACKTHRESHOLD 4096
#elif thresh2
    #define CRACKTHRESHOLD 1
#else
    #define CRACKTHRESHOLD 64
#endif


#ifdef maxlevel3
    #define MAXLEVEL 3
#elif maxlevel4
    #define MAXLEVEL 4
#elif maxlevel5
    #define MAXLEVEL 5
#elif maxlevel6
    #define MAXLEVEL 6
#endif


#ifdef cellThreshold100
    #define CELLTHRESHOLD 100
#elif cellThreshold4  // For testing purposes
    #define CELLTHRESHOLD 4
#elif cellThreshold10K
    #define CELLTHRESHOLD 10000
#elif cellThreshold100K
    #define CELLTHRESHOLD 100000
#elif cellThreshold50K
    #define CELLTHRESHOLD 50000
#elif cellThreshold25K
    #define CELLTHRESHOLD 25000
#endif

int inStoch = 0;
int actuallyCracked = 0;
int sizeOfStochCrackedPiece = 0;


int totalNodes = 0;
#ifdef TIMEBREAKDOWN
    int recursiveCalls = 0;
    int numOfSplits = 0;
    int failedCracks = 0;
    int swaps = 0;
    int traversals = 0;
    
    
    clock_t prefixSumTime = 0;
    clock_t getWeightTime = 0;
    clock_t splitTime = 0;
    clock_t binarySearchTime = 0;
    clock_t partitionTime = 0;
    clock_t swapTime = 0;

    clock_t mediocreTime = 0;

    clock_t adaptiveSplitTime = 0;
    clock_t adaptiveCountingResultsTime = 0;

    clock_t pushingTime = 0;
#endif


#ifdef REVISION_TIMES
    clock_t falsePositiveTime = 0;
    clock_t adaptationTime = 0;
    clock_t totalSearchTime = 0;
#endif

#ifdef DIM2
    #define NUMDIMS 2
#elif DIM3
    #define NUMDIMS 3
#elif DIM4
    #define NUMDIMS 4
#elif DIM5
    #define NUMDIMS 5
#elif DIM6
    #define NUMDIMS 6
#elif DIM7
    #define NUMDIMS 7
#endif

#ifdef thresh32
    #define CRACKTHRESHOLD 32
#elif thresh64
    #define CRACKTHRESHOLD 64
#elif thresh128
    #define CRACKTHRESHOLD 128
#elif thresh256
    #define CRACKTHRESHOLD 256
#elif thresh512
    #define CRACKTHRESHOLD 512
#elif thresh1024
    #define CRACKTHRESHOLD 1024
#elif thresh2048
    #define CRACKTHRESHOLD 2048
#elif thresh4096
    #define CRACKTHRESHOLD 4096
#elif thresh2
    #define CRACKTHRESHOLD 1
#else
    #define CRACKTHRESHOLD 64
#endif

#ifdef TIMEBREAKDOWN
clock_t begin_rtile; clock_t end_rtile;
clock_t total_rtile = 0;
clock_t begin_memcpy, end_memcpy, total_memcpy;
clock_t begin_nonadapt, end_nonadapt;
clock_t begin_queryeval, end_queryeval, total_queryeval; 
#endif

ELEMTYPE m_data_arr[DATA_COUNT][NUMDIMS];
// ELEMTYPE m_data_arr_mins[DATA_COUNT][NUMDIMS];
// ELEMTYPE m_data_arr_maxes[DATA_COUNT][NUMDIMS];



int m_data_ids[DATA_COUNT];
// vector<vector<ELEMTYPE>> m_data_arr;

int last_id;

#define ASSERT assert // RTree uses ASSERT( condition )
#ifndef Min
#define Min std::min
#endif //Min
#ifndef Max
#define Max std::max
#endif //Max



// level, startX, startY, endX, endY, cutAxis, KDNode
// we need level for order purposes. We want to access item based on their level.



struct Point {
    Point(){}
    Point(ELEMTYPE values[NUMDIMS]){
        for(int i =0; i < NUMDIMS; ++i) m_values[i] = values[i];
    }
    ELEMTYPE m_values[NUMDIMS];
};


struct Rect
{
    Rect()  {}

    Rect(ELEMTYPE a_min[NUMDIMS], ELEMTYPE a_max[NUMDIMS]){
        for(int i = 0; i < NUMDIMS; i++) m_min[i] = a_min[i];
        for(int i = 0; i < NUMDIMS; i++) m_max[i] = a_max[i];
    }


    ELEMTYPE m_min[NUMDIMS];
    ELEMTYPE m_max[NUMDIMS];
};
Rect m_data_arr_rects[DATA_COUNT];

bool Overlap(Rect *a_rectA, Rect *a_rectB) {
    for (int index = 0; index < NUMDIMS; ++index) {
        if (a_rectA->m_min[index] > a_rectB->m_max[index] || a_rectA->m_max[index] < a_rectB->m_min[index]) {
            return false;
        }
    }
    return true;
}


bool Overlap(ELEMTYPE *a_queryMins, ELEMTYPE *a_queryMaxes, Rect *a_dataRect) {

    for (int index = 0; index < NUMDIMS; ++index) {
        if (a_queryMins[index] > a_dataRect->m_max[index] || a_queryMaxes[index] < a_dataRect->m_min[index]) {
            return false;
        }
    }
    return true;
}

vector<Rect> foundIds;

bool Overlap(ELEMTYPE *a_queryMins, ELEMTYPE *a_queryMaxes, ELEMTYPE *a_dataPoint) {
    for (int index = 0; index < NUMDIMS; ++index) {
        if (a_queryMins[index] > a_dataPoint[index] || a_queryMaxes[index] < a_dataPoint[index]) {
            return false;
        }
    }
    return true;
}

bool Overlap(ELEMTYPE *a_queryMins, ELEMTYPE *a_queryMaxes, ELEMTYPE a_dataPointX, ELEMTYPE a_dataPointY) {
    if (a_queryMins[0] > a_dataPointX || a_queryMaxes[0] < a_dataPointX) {
        return false;
    }

    if (a_queryMins[1] > a_dataPointY || a_queryMaxes[1] < a_dataPointY) {
        return false;
    }

    return true;
}


bool Overlap(ELEMTYPE *a_queryMins, ELEMTYPE *a_queryMaxes, ELEMTYPE a_dataPointX, ELEMTYPE a_dataPointY, ELEMTYPE a_dataPointZ) {
    if (a_queryMins[0] > a_dataPointX || a_queryMaxes[0] < a_dataPointX) {
        return false;
    }
    if (a_queryMins[1] > a_dataPointY || a_queryMaxes[1] < a_dataPointY) {
        return false;
    }

    if (a_queryMins[2] > a_dataPointZ || a_queryMaxes[2] < a_dataPointZ) {
        return false;
    }
    return true;
}


double timing(){
    static struct timeval t1, t2;
    gettimeofday(&t2,NULL);
    double ret = t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6;
    t1 = t2;
    return ret;
}

#endif
