#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "utils.h"


float three_way_max(float a, float b, float c) {
  return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}


float three_way_min(float a, float b, float c) {
  return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}


float dot_product(const float* a, const float* b, int n) {
  float sum=0;
  for(int q1=0;q1<n;q1++)sum+=a[q1]*b[q1];
  return sum;
}