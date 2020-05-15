#pragma once

#include <vector>
#include <string>
#include <chrono>

using namespace std;

#include "../image/inc/image.h"


#define NOT_IMPLEMENTED() do{static bool done=false;if(!done)fprintf(stderr,"Function \"%s\"  in file \"%s\" line %d not implemented yet!!!\n",__FUNCTION__, __FILE__, __LINE__);done=true;}while(0)


const float TEST_EPS=0.005;
inline int within_eps(float a, float b) { return a-TEST_EPS<b && b<a+TEST_EPS; }

// MIN MAX MACROS
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

float three_way_max(float a, float b, float c);
float three_way_min(float a, float b, float c);

