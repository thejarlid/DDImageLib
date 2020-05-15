#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <mutex>

#include "../inc/filter_image.h"
#include "../../utils/utils.h"

using namespace std;


// Helper methods to construct some basic filters and apply them


Image convolve_image(const Image& im, const Image& filter, int preserve) {
  assert(filter.c == 1 || filter.c == im.c);
  Image ret = Image(im.w, im.h, preserve ? im.c : 1);
  for (int c = 0; c < im.c; c++) {
    for (int y = 0; y < im.h; y++) {
      for (int x = 0; x < im.w; x++) {

        // now loop the filter
        int filter_c = filter.c ? 0 : c;
        for (int j = 0; j < filter.h; j++) {
          for(int i = 0; i < filter.w; i++) {
            float value = (filter.get_pixel(i, j, filter_c) * im.get_pixel(x + (i - filter.w/2), y + (j - filter.h/2), c));
            ret.set_pixel(x, y, preserve ? c : 0, ret.get_pixel(x, y, preserve ? c : 0) + value);
          }
        }
      }
    }
  }
  return ret;
}


Image make_box_filter(int w) {
  Image box_filter(w, w);
  for (int row = 0; row < w; row++) {
    for (int col = 0; col < w; col++) {
      box_filter.set_pixel(col, row, 0, 1);
    }
  }
  box_filter.l1_normalize();
  return box_filter;
}


Image make_highpass_filter() {
  Image ret = Image(3, 3, 1);
  ret(0, 0, 0) = 0;
  ret(1, 0, 0)= -1;
  ret(2, 0, 0) = 0;
  ret(0, 1, 0) = -1;
  ret(1, 1, 0) = 4;
  ret(2, 1, 0) = -1;
  ret(0, 2, 0) = 0;
  ret(1, 2, 0) = -1;
  ret(2, 2, 0) = 0;
  return ret;
}


Image make_sharpen_filter() {
  Image ret = Image(3, 3, 1);
  ret(0, 0, 0) = 0;
  ret(1, 0, 0)= -1;
  ret(2, 0, 0) = 0;
  ret(0, 1, 0) = -1;
  ret(1, 1, 0) = 5;
  ret(2, 1, 0) = -1;
  ret(0, 2, 0) = 0;
  ret(1, 2, 0) = -1;
  ret(2, 2, 0) = 0;
  return ret;
}


Image make_emboss_filter() {
  Image ret = Image(3, 3, 1);
  ret(0, 0, 0) = -2;
  ret(1, 0, 0)= -1;
  ret(2, 0, 0) = 0;
  ret(0, 1, 0) = -1;
  ret(1, 1, 0) = 1;
  ret(2, 1, 0) = 1;
  ret(0, 2, 0) = 0;
  ret(1, 2, 0) = 1;
  ret(2, 2, 0) = 2;
  return ret;
}


Image make_1d_gaussian(float sigma) {
  int dimension = 6 * sigma;
  dimension = dimension % 2 == 0 ? dimension + 1 : dimension;
  Image ret = Image(dimension, 1, 1);
  for (int x = 0; x < dimension; x++) {
    ret(x, 0, 0) = exp(-((((float)x - (float)dimension/2.0f) + 0.5) * ((float)x - (float)dimension/2.0f) + 0.5)/(2.0f * pow(sigma, 2)))/(2 * M_PI * pow(sigma, 2));
  }
  return ret;
}


Image smooth_image(const Image& im, float sigma) {
  Image horizontal = make_1d_gaussian(sigma);
  Image vertical(1, horizontal.w, 1);
  for(int x = 0; x < horizontal.w; x++) {
    vertical(0, x, 0) = horizontal(x, 0, 0);
  }

  Image result = convolve_image(im, horizontal, 1);
  result = convolve_image(result, vertical, 1);

  return result;
}


Image fast_smooth_image(const Image& im, float sigma) {
  assert(sigma>=0.f);
  int w=roundf(sigma*6);
  if(w%2==0)w++;
  
  vector<float> g(w);
  float*gf=g.data()+w/2;
  
  {
  float sum=0;
  for(int q1=-w/2;q1<=w/2;q1++)gf[q1]=expf(-(q1*q1)/(2.f*sigma*sigma));
  for(int q1=-w/2;q1<=w/2;q1++)sum+=gf[q1];
  for(int q1=-w/2;q1<=w/2;q1++)gf[q1]/=sum;
  }
  
  
  auto do_one=[gf,w](const Image& im) {
    Image expand(im.w+w-1,im.h,im.c);
    for(int c=0;c<im.c;c++)
    for(int q2=0;q2<expand.h;q2++)for(int q1=0;q1<expand.w;q1++)
      expand(q1,q2,c)=im.get_pixel(q1-w/2,q2,c);
    
    Image ret(im.w,im.h,im.c);
    
    int Nthreads=8;
    int total=im.h*im.c;
    vector<thread> th;
    for(int t=0;t<Nthreads;t++)th.push_back(thread([&](int a,int b){
      for(int q=a;q<b;q++){
        int c=q/im.h;
        int q2=q%im.h;
        for(int q1=0;q1<im.w;q1++)ret(q1,q2,c)=dot_product(&expand(q1,q2,c),gf-w/2,w);
      }
    },t*total/Nthreads,(t+1)*total/Nthreads));
    for(auto&e1:th)e1.join();
    
    return ret;
  };
  
  Image first=do_one(im);
  Image second=do_one(first.transpose()).transpose();
  
  
  return second;
}


Image make_gaussian_filter(float sigma) {
  int dimension = 6 * sigma;
  dimension = dimension % 2 == 0 ? dimension + 1 : dimension;
  Image ret = Image(dimension, dimension, 1);
  for (int y = 0; y < dimension; y++) {
    for (int x = 0; x < dimension; x++) {
      float exponent = expf(-(powf((float)x - ((float)dimension/2.0f) + 0.5, 2) + powf((float)y - ((float)dimension/2.0f) + 0.5, 2))/(2 * powf(sigma, 2)));
      float value = exponent / (2 * M_PI * powf(sigma, 2));
      ret(x, y, 0) = value;
    }
  }
  ret.l1_normalize();
  return ret;
}


Image add_image(const Image& a, const Image& b) {
  assert(a.w == b.w && a.h == b.h && a.c == b.c);
  Image ret = Image(a.w, a.h, a.c);
  for (int c = 0; c < a.c; c++) {
    for (int j = 0; j < a.h; j++) {
      for (int i = 0; i < a.w; i++) {
        ret(i, j, c) = a(i, j, c) + b(i, j, c);
      }
    }
  }
  return ret;
}

Image sub_image(const Image& a, const Image& b) {
  assert(a.w == b.w && a.h == b.h && a.c == b.c);
  Image ret = Image(a.w, a.h, a.c);
  for (int c = 0; c < a.c; c++) {
    for (int j = 0; j < a.h; j++) {
      for (int i = 0; i < a.w; i++) {
        ret(i, j, c) = a(i, j, c) - b(i, j, c);
      }
    }
  }
  return ret;
}


Image make_gx_filter() {
  Image ret = Image(3, 3, 1);
  ret(0, 0, 0) = -1;
  ret(1, 0, 0) = 0;
  ret(2, 0, 0) = 1;
  ret(0, 1, 0) = -2;
  ret(1, 1, 0) = 0;
  ret(2, 1, 0) = 2;
  ret(0, 2, 0) = -1;
  ret(1, 2, 0) = 0;
  ret(2, 2, 0) = 1;
  return ret;
}


Image make_gy_filter() {
  Image ret = Image(3, 3, 1);
  ret(0, 0, 0) = -1;
  ret(1, 0, 0) = -2;
  ret(2, 0, 0) = -1;
  ret(0, 1, 0) = 0;
  ret(1, 1, 0) = 0;
  ret(2, 1, 0) = 0;
  ret(0, 2, 0) = 1;
  ret(1, 2, 0) = 2;
  ret(2, 2, 0) = 1;
  return ret;
}


pair<Image,Image> sobel_image(const Image& im) {
  Image Mag(im.w,im.h);
  Image Theta(im.w,im.h);

  Image gx = convolve_image(im, make_gx_filter(), 0);
  Image gy = convolve_image(im, make_gy_filter(), 0);
  for (int y = 0; y < im.h; y++) {
    for (int x = 0; x < im.w; x++) {
      Mag(x, y, 0) = sqrtf(powf(gx.get_pixel(x, y, 0), 2) + powf(gy.get_pixel(x, y, 0), 2));
      Theta(x, y, 0) = atan2f(gy.get_pixel(x, y, 0), gx.get_pixel(x, y, 0));
    }
  }

  return {Mag,Theta};
}


Image colorize_sobel(const Image& im) {
  Image ret = Image(im.w, im.h, 3);
  pair<Image, Image> temp = sobel_image(im);
  for (int j = 0; j < im.h; j++) {
    for (int i = 0; i < im.w; i++) {
      ret(i, j, 0) = temp.second(i, j, 0);
      ret(i, j, 1) = temp.first(i, j, 0);
      ret(i, j, 2) = temp.first(i, j, 0);
    }
  }
  ret.feature_normalize();
  ret.HSVtoRGB();
  save_image(ret, "output/sobel");
  return convolve_image(ret, make_box_filter(3), 1);
}


Image bilateral_filter(const Image& im, float sigma1, float sigma2) {
  Image ret = Image(im.w, im.h, im.c);
  int dimension = 3 * sigma1;
  dimension = dimension % 2 == 0 ? dimension + 1 : dimension;
  float previous = 0;
  for (int c = 0; c < im.c; c++) {
    for (int y = 0; y < im.h; y++) {
      for (int x = 0; x < im.w; x++) {
        float normalized_factor = 0.0f;
        float value = 0.0f;
        for (int j = 0; j < dimension; j++) {
          for (int i = 0; i < dimension; i++) {
            int x_prime = x + (i - (dimension/2.0f)) + 0.5;
            int y_prime = y + (j - (dimension/2.0f)) + 0.5;
            float regular_gaussian = expf(-((powf(x_prime - x, 2) + powf(y_prime - y, 2))/(2 * powf(sigma1, 2)))) / (2 * M_PI * powf(sigma1, 2));
            float intensity_gaussian = expf(-(powf((im.get_pixel(x_prime, y_prime, c) - im.get_pixel(x, y, c)), 2))/(2 * powf(sigma2, 2))) / (2 * M_PI * powf(sigma2, 2));
            previous = intensity_gaussian;
            value += (intensity_gaussian * regular_gaussian * im.get_pixel(x_prime, y_prime, c));
            normalized_factor += (regular_gaussian * intensity_gaussian);
          }
        }
        value /= normalized_factor;
        ret(x, y, c) = value;
      }
    }
  }
  save_image(ret, "output/bilateral");
  return ret;
}


// Image class filter specific instance methods 

void Image::feature_normalize() {
  for (int ch = 0; ch < c; ch++) {
    float min = get_pixel(0, 0, 0);
    float max = get_pixel(0, 0, 0);
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        float value = get_pixel(i, j, ch);
        if (value < min) {
          min = value;
        }
        if (value > max) {
          max = value;
        }
      }
    }
    float diff = max - min;
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (diff == 0) {
          set_pixel(i, j, ch, 0);
        } else {
          set_pixel(i, j, ch, (get_pixel(i, j, ch) - min)/diff);
        }
      }
    }
  }
}


void Image::feature_normalize_total() {
  float min = get_pixel(0, 0, 0);
  float max = get_pixel(0, 0, 0);
  for (int ch = 0; ch < c; ch++) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        float value = get_pixel(i, j, ch);
        if (value < min) {
          min = value;
        }
        if (value > max) {
          max = value;
        }
      }
    }
  }

  float diff = max - min;
  for (int ch = 0; ch < c; ch++) {
    for (int j = 0; j < h; j++) {
      for (int i = 0; i < w; i++) {
        if (diff == 0) {
          set_pixel(i, j, ch, 0);
        } else {
          set_pixel(i, j, ch, (get_pixel(i, j, ch) - min)/diff);
        }
      }
    }
  }
}