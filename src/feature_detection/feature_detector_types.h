// Types used for feature detection

#pragma once

#include "../image/inc/image.h"

// A 2d point.
// float x, y: the coordinates of the point.
struct Point  {
  double x, y;
  
  Point() : x(0), y(0) {}
  Point(double x, double y) : x(x), y(y) {}
};


// A descriptor for a point in an image.
// point p: x,y coordinates of the image pixel.
// vector<float> data: the descriptor for the pixel.
struct Descriptor {
  Point p;
  vector<float> data;
  
  Descriptor(){}
  Descriptor(const Point& p) : p(p) {}
};


// A match between two points in an Image.
// const Descriptor* a, b: Pointers to the Descriptors in the corresponding images.
// float distance: the distance between the descriptors for the points.
struct Match {
  const Descriptor* a=nullptr;
  const Descriptor* b=nullptr;
  float distance=0.f;
  
  Match(){}
  Match(const Descriptor* a,const Descriptor* b,float dist=0.f) : a(a), b(b), distance(dist) {}
  
  bool operator<(const Match& other) { return distance<other.distance; }
};
