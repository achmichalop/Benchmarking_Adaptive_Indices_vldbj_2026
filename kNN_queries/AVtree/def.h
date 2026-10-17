#include <cmath>
#include <cstring>
#include <sstream>
#include <queue>
#include <fstream>
#include <chrono>
#include "omp.h"
#include "getopt.h"
#ifdef ED
    #include "./tree/AVtreeString.h"
#else
    #include "./tree/AVtree.h"
#endif
#include <sys/stat.h>                                                     
#include <iostream> 
#include <functional>
#include <iomanip>


using namespace std;
using namespace std::chrono;

#define LINEAR_SCAN                                                 0
#define STANDARD                                                    1
#define STANDARD_MEDIOCRE                                           2
#define MEDIOCRE_CACHE                                              3

#define KNNMediocre                                                 4
#define KNNCrackOnKth                                               5




// #ifdef datacount1M
//     #define DATA_COUNT 1000000
// #elif datacount20M
//     #define DATA_COUNT 20000000
// #endif

// #ifdef DIM2
//     #define NUMDIMS 2
// #elif DIM3
//     #define NUMDIMS 3
// #elif DIM4
//     #define NUMDIMS 4
// #elif DIM5
//     #define NUMDIMS 5
// #elif DIM6
//     #define NUMDIMS 6
// #elif DIM7
//     #define NUMDIMS 7
// #endif



// struct Rect
// {
//     Rect()  {}

//     Rect(float a_min[NUMDIMS], float a_max[NUMDIMS]){
//         for(int i = 0; i < NUMDIMS; i++) m_min[i] = a_min[i];
//         for(int i = 0; i < NUMDIMS; i++) m_max[i] = a_max[i];
//     }


//     float m_min[NUMDIMS];
//     float m_max[NUMDIMS];
// };


// struct Point {
//     Point(){}
//     Point(float values[NUMDIMS]){
//         for(int i =0; i < NUMDIMS; ++i) m_values[i] = values[i];
//     }
//     float m_values[NUMDIMS];
// };
    