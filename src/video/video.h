#pragma once

#include "../image/inc/image.h"
#include "../feature_detection/feature_detector_types.h"
#include "../matrix/matrix.h"
#include "../utils/utils.h"


struct Video {
  vector<Image> input_frames;
  vector<Image> output_frames;
  vector<vector<Descriptor>> features;
  vector<Matrix> timewise_homographies;
  vector<vector<Point>> smoothed_features;
  vector<vector<Descriptor>> smoothed_descriptors;
  vector<Matrix> smoothing_homographies;
};


// void test_func();
void test_func();
vector<vector<Descriptor>> parse_features();
vector<vector<Match>> parse_matches();
vector<Image> smooth_frames(vector<Image> input);
void get_features_per_frame(Video& video);
void compute_timewise_homogrpahies(Video& video);
void smooth_feature_points(Video& video, float sigma);
void compute_smoothing_homography(Video& video);
void smooth_images(Video& video);