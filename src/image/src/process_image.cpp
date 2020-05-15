#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>

#include "../inc/image.h"
#include "../../utils/utils.h"
#include "../../colourspace/colourspaces.h"

using namespace std;


Image Image::rgb_to_grayscale() const {
  assert(c == 3);
  Image&& grayscaleImg = Image(w, h);
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      float value = (0.299f * get_pixel(col, row, 0)) 
                  + (0.587f * get_pixel(col, row, 1)) 
                  + (0.114f * get_pixel(col, row, 2));
      grayscaleImg.set_pixel(col, row, 0, value);
    } 
  }
  return grayscaleImg;
}


void Image::shift(int ch, float v) {
  assert(ch >= 0 && ch < c);
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      pixel(col, row, ch) += v;
    }
  }
}


void Image::clamp() {
  for (int ch = 0; ch < c; ch++) {
    for (int row = 0; row < h; row++) {
      for (int col = 0; col < w; col++) {
        set_pixel(col, row, ch, fmax(0.0f, fmin(1.0f, get_pixel(col, row, ch))));
      }
    }
  }
}


void Image::scale(int ch, float v) {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      pixel(col, row, c) *= v;
    }
  }
}


void Image::RGBtoHSV() {
  assert(c == 3);
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      float red = pixel(col, row, 0);
      float green = pixel(col, row, 1);
      float blue = pixel(col, row, 2);

      HSVcolour hsv = rgb2hsv({red, green, blue});
      pixel(col, row, 0) = hsv.h;
      pixel(col, row, 1) = hsv.s;
      pixel(col, row, 2) = hsv.v;
    }
  }
}


void Image::HSVtoRGB() {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      float hue = pixel(col, row, 0);
      float saturation = pixel(col, row, 1);
      float value = pixel(col, row, 2);

      RGBcolour rgb = hsv2rgb({hue, saturation, value});
      pixel(col, row, 0) = rgb.r;
      pixel(col, row, 1) = rgb.g;
      pixel(col, row, 2) = rgb.b;
    }
  }
}


void Image::LCHtoRGB() {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      float l = pixel(col, row, 0);
      float c = pixel(col, row, 1);
      float h = pixel(col, row, 2);

      RGBcolour rgb = lch2rgb({l, c, h});
      pixel(col, row, 0) = rgb.r;
      pixel(col, row, 1) = rgb.g;
      pixel(col, row, 2) = rgb.b;
    }
  }
}

void Image::RGBtoLCH() {
  for (int row = 0; row < h; row++) {
    for (int col = 0; col < w; col++) {
      float r = pixel(col, row, 0);
      float g = pixel(col, row, 1);
      float b = pixel(col, row, 2);

      LCHcolour lch = rgb2lch({r, g, b});
      pixel(col, row, 0) = lch.l;
      pixel(col, row, 1) = lch.c;
      pixel(col, row, 2) = lch.h;
    }
  }
}
