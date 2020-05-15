#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>

#include "../inc/image.h"
#include "../../utils/utils.h"

using namespace std;


float Image::nn_interpolate(float x, float y, int ch) const {
  return get_pixel(round(x - 0.5f), round(y - 0.5f), ch);
}


float Image::bilinear_interpolate(float x, float y, int ch) const {
  x -= 0.5f;
  y -= 0.5f;

  int lower_x = floor(x);
  int lower_y = floor(y);
  int upper_x = lower_x + 1;
  int upper_y = lower_y + 1;

  float v1 = get_pixel(lower_x, lower_y, ch);
  float v2 = get_pixel(upper_x, lower_y, ch);
  float v3 = get_pixel(lower_x, upper_y, ch);
  float v4 = get_pixel(upper_x, upper_y, ch);

  float q1 = (v1 * (((float)upper_x) - x)) + (v2 * (x - ((float)lower_x)));
  float q2 = (v3 * (((float)upper_x) - x)) + (v4 * (x - ((float)lower_x)));

  return (q1 * (((float)upper_y) - y)) + (q2 * (y - ((float)lower_y)));
}


Image Image::nn_resize(int w, int h) {
  float col_scale = (float)this->w/(float)w;
  float row_scale = (float)this->h/(float)h;
  Image resized(w, h, c);
  for (int ch = 0; ch < c; ch++) {
    for (int row = 0; row < h; row++) {
      for (int col = 0; col < w; col++) {
        resized(col, row, ch) = nn_interpolate(col_scale * ((float)col + 0.5f), row_scale * ((float)row + 0.5f), ch);
      }
    }
  }
  return resized;
}


Image Image::bilinear_resize(int w, int h) {
  float col_scale = (float)this->w/(float)w;
  float row_scale = (float)this->h/(float)h;
  Image resized(w, h, c);
  for (int ch = 0; ch < c; ch++) {
    for (int row = 0; row < h; row++) {
      for (int col = 0; col < w; col++) {
        resized(col, row, ch) = bilinear_interpolate(col_scale * ((float)col + 0.5f), row_scale * ((float)row + 0.5f), ch);
      }
    }
  }
  return resized;
}