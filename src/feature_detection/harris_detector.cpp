#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>

#include "harris_detector.h"

using namespace std;


// Create a feature descriptor for an index in an image.
// const Image& im: source image.
// int x,y: coordinates for the pixel we want to describe.
// returns: Descriptor for that index.
Descriptor describe_index(const Image& im, int x, int y, int w) {
  Descriptor d;
  d.p={(double)x,(double)y};
  d.data.reserve(w*w*im.c);
  
  // If you want you can experiment with other descriptors
  // This subtracts the central value from neighbors
  // to compensate some for exposure/lighting changes.
  for(int c=0;c<im.c;c++) {
    float cval = im.get_pixel(x,y,c);
    for(int dx=-w/2;dx<=w/2;dx++)for(int dy=-w/2;dy<=w/2;dy++)
      d.data.push_back(im.get_pixel(x+dx,y+dy,c)-cval);
  }
  return d;
}


// Marks the spot of a point in an image.
// Image& im: image to mark.
// Point p: spot to mark in the image.
void mark_spot(Image& im, const Point& p) {
  int x = p.x;
  int y = p.y;
  
  for(int i = -9; i < 10; ++i) {
    im.set_pixel(x+i, y, 0, 1);
    im.set_pixel(x, y+i, 0, 1);
    im.set_pixel(x+i, y, 1, 0);
    im.set_pixel(x, y+i, 1, 0);
    im.set_pixel(x+i, y, 2, 1);
    im.set_pixel(x, y+i, 2, 1);
  }
}


// Marks corners denoted by an array of descriptors.
// Image& im: image to mark.
// vector<Descriptor> d: corners in the image.
Image mark_corners(const Image& im, const vector<Descriptor>& d) {
  Image im2=im;
  for(auto&e1:d)mark_spot(im2,e1.p);
  return im2;
}


// Calculate the structure matrix of an image.
// const Image& im im: the input image.
// float sigma: std dev. to use for weighted sum.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          third channOel is IxIy.
Image structure_matrix(const Image& im2, float sigma) {
  // only grayscale or rgb
  assert((im2.c==1 || im2.c==3) && "only grayscale or rgb supported");
  Image im;
  // convert to grayscale if necessary
  if (im2.c == 1) im = im2;
  else im = im2.rgb_to_grayscale();

  Image S(im.w, im.h, 3);
  Image ix = convolve_image(im, make_gx_filter(), 0);
  Image iy = convolve_image(im, make_gy_filter(), 0);

  for (int y = 0; y < im.h; y++) {
    for (int x = 0; x < im.w; x++) {
      S(x, y, 0) = ix(x, y, 0) * ix(x, y, 0);
      S(x, y, 1) = iy(x, y, 0) * iy(x, y, 0);
      S(x, y, 2) = ix(x, y, 0) * iy(x, y, 0);
    }
  }

  return convolve_image(S, make_gaussian_filter(sigma), 1);
}


// Estimate the cornerness of each pixel given a structure matrix S.
// const Image& im S: structure matrix for an image.
// returns: a response map of cornerness calculations.
// int method: 0: det(S)/tr(S)    1 (optional) : exact 2nd eigenvalue
Image cornerness_response(const Image& S, int method) {
  Image R(S.w, S.h);
  // method==0: E(S) = det(S) / trace(S)
  // method==1 (optional): E(S) = exact 2nd eigenvalue (what is the formula??)
  for (int y = 0; y < S.h; y++) {
    for (int x = 0; x < S.w; x++) {
      float det = (S(x, y, 0) * S(x, y, 1)) - (S(x, y, 2) * S(x, y, 2));
      float trace = S(x, y, 0) + S(x, y, 1);
      R(x, y, 0) = det/trace;
    }
  }
  return R;
}


// Perform non-max supression on an image of feature responses.
// const Image& im: 1-channel image of feature responses.
// int w: distance to look for larger responses.
// returns: Image with only local-maxima responses within w pixels.
Image nms_image(const Image& im, int w) {
  Image r=im;
  // for every pixel in the image:
  //     for neighbors within w:
  //         if neighbor response greater than pixel response:
  //             set response to be very low (I use -999999 [why not 0??])
  for (int y = 0; y < im.h; y++) {
    for (int x = 0; x < im.w; x++) {
      float curr_val = im.get_pixel(x, y, 0);
      float surpressed = curr_val;
      for (int dy = 0; dy < (w * 2) + 1; dy++) {
        for (int dx = 0; dx < (w * 2) + 1; dx++) {
          int x_prime = x - (w - dx);
          int y_prime = y - (w - dy);
          if (im.get_pixel(x_prime, y_prime, 0) > curr_val) {
            surpressed = -999999;
          }
        }
      }
      r.set_pixel(x, y, 0, surpressed);
    }
  }
  return r;
}


// Perform corner detection and extract features from the corners.
// const Image& im: input image.
// const Image& nms: nms image
// float thresh: threshold for cornerness.
// returns: vector of descriptors of the corners in the image.
vector<Descriptor> detect_corners(const Image& im, const Image& nms, float thresh, int window) {
  vector<Descriptor> d;
  //TODO: fill in vector<Descriptor> with descriptors of corners, use describe_index.
  // to add element to 'd' use:   d.push_back(X);
  for (int y = 0; y < im.h; y++) {
    for (int x = 0; x < im.w; x++) {
      if (nms(x, y, 0) >= thresh) {
        d.push_back(describe_index(im, x, y, window));
      }
    }
  }
  return d;
}


// Perform harris corner detection and extract features from the corners.
// const Image& im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
// returns: vector of descriptors of the corners in the image.
vector<Descriptor> harris_corner_detector(const Image& im, float sigma, float thresh, int window, int nms, int corner_method) {
  // Calculate structure matrix
  Image S = structure_matrix(im, sigma);
  
  // Estimate cornerness
  Image R = cornerness_response(S,corner_method);
  
  // Run NMS on the responses
  Image Rnms = nms_image(R, nms);
  
  return detect_corners(im, Rnms, thresh, window);
}


// Find and draw corners on an image.
// Image& im: input image.
// float sigma: std. dev for harris.
// float thresh: threshold for cornerness.
// int nms: distance to look for local-maxes in response map.
Image detect_and_draw_corners(const Image& im, float sigma, float thresh, int window, int nms, int corner_method) {
  vector<Descriptor> d = harris_corner_detector(im, sigma, thresh, window, nms, corner_method);
  return mark_corners(im, d);
}
