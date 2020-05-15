#pragma once

#include "../image/inc/image.h"
#include "../matrix/matrix.h"
#include "../image/inc/filter_image.h"
#include "../feature_detection/harris_detector.h"


using namespace std;


struct LKIterPyramid {
  // INPUT
  vector<Image> pyramid1;
  vector<Image> pyramid0;
  Image t0;
  Image t1;

  // OUTPUT
  Image v;  // resulting velocity
  Image colorflow; // colorized velocity
  Image error; // difference between warp(t0,v) and t1
  Image ev3; // three channel eigenvalue image for displaying
  Image warped;// warped t0 with flow v
  Image all; // ciombined 4 images for Opencv display

  // OPTIONS
  float subsample_input=2;    // how much to reduce input image size
  float smooth_structure=1;   // how much to smooth structure matrix
  float smooth_vel=1;         // how much to smooth resulting velocity
  int lk_iterations=2;        // LK iterations to run (0 -  no flow, 1 - standard version)
  int pyramid_levels=6;       // pyramid levels (1 - standard algo)
  float pyramid_factor=2;     // ratio of sizes between successive pyramid levels
  float clamp_vel=10;         // how much to clamp velocity
  float vel_color_scale=4;    // saturation of vel image

  bool compute_all=false; // compute total combined image for opencv
  bool compute_colored_ev=true; // compute colored eigenvalue image
};


// Calculate the time-structure matrix of an Image pair.
// const Image& im: the input Image.
// const Image& prev: the previous Image in sequence.
// float s: sigma used for Gaussian smoothing the gradients
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          3rd channel is IxIy, 4th channel is IxIt, 5th channel is IyIt.
Image time_structure_matrix(const Image& im, const Image& prev, float s);


// Compute the eigenvalues of the structure matrix
// Compute the eigenvalues only of S'S (the first three channels only)
// const Image& ts: the time-structure matrix
// returns: 2-channel image: 0-th channel : biggest eigenvalue,
//                           1-st channel : smallest
Image eigenvalue_matrix(const Image& ts);


vector<Image> make_image_pyramid(const Image& a, float factor, int levels);


// Calculate the velocity given a structure Image
// const Image& S: time-structure Image
// const Image& ev: eigenvalue image
// Return: 2 channel (u,v) image  : the x and y
// velocities computed by inv(S'S)*(S'T)
Image velocity_image(const Image& S,const Image& ev);


// Constrain the absolute value of each Image pixel
// const Image& im: Image to constrain
// float v: each pixel will be in range [-v, v]
void constrain_image(const Image& im, float v);


// const Image& im: input image
// const Image& v: velocity image specifying how much each pixel moves
// return warped image, with same size as input image, as discussed on
// the github page.
Image warp_flow(const Image& im, const Image& v);


// Resize velocity image
// Image oldvel: old velocity
// int w,h : new sizes
// return new velocity image
Image velocity_resize(const Image& oldvel, int w, int h);


void compute_iterative_pyramid_LK(LKIterPyramid& lk);


// Calculate the optical flow between two images
// const Image& im: current Image
// Image prev: previous Image
// float smooth_win: amount to smooth structure matrix by
// float smooth_vel: amount to smooth velocity image
// returns: velocity matrix
Image optical_flow_images(const Image& im, const Image& prev, float smooth_win, float smooth_vel);


Image vel2rgb(const Image& v, float thres);
