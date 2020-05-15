// Filtering Tools for images

#pragma once

#include "image.h"

/**
 * @brief Despite the name actually does a cross correllation on the image
 * doesn't do anything complex like decompose filters. But essentially applies
 * the given filter to the image preserving the channels if the flag is set
 * 
 * @param im the image for which to convolve
 * @param filter the filter to app,y
 * @param preserve whether the channel structure should be preserved
 * @return Image the new image resulting from the convolution
 */
Image convolve_image(const Image& im, const Image& filter, int preserve);


/**
 * @brief Makes a box filter which is simply an average blur filter 
 * where all elements sum to 1
 * 
 * for a 3x3 filter this is the matrix:
 * 
 * | 1/9   1/9   1/9 |
 * | 1/9   1/9   1/9 |
 * | 1/9   1/9   1/9 |
 * 
 * @param w the dimension of the filter
 * @return Image the filter
 */
Image make_box_filter(int w);



Image make_highpass_filter(void);
Image make_sharpen_filter(void);
Image make_emboss_filter(void);
Image make_gaussian_filter(float sigma);

// Creates a 1d Gaussian filter.
// float sigma: standard deviation of Gaussian.
// returns: single row Image of the filter.
Image make_1d_gaussian(float sigma);


// Smooths an image using separable Gaussian filter.
// const Image& im: image to smooth.
// float sigma: std dev. for Gaussian.
// returns: smoothed Image.
Image smooth_image(const Image& im, float sigma);

Image fast_smooth_image(const Image& im, float sigma);

Image make_gx_filter(void);
Image make_gy_filter(void);

Image add_image(const Image& a, const Image& b);
Image sub_image(const Image& a, const Image& b);

inline Image operator-(const Image& a, const Image& b) { return sub_image(a,b); }
inline Image operator+(const Image& a, const Image& b) { return add_image(a,b); }

pair<Image,Image> sobel_image(const Image&  im);
Image colorize_sobel(const Image&  im);
Image smooth_image(const Image&  im, float sigma);
Image bilateral_filter(const Image& im, float sigma, float sigma2);