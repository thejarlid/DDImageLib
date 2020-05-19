#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>

#include "video.h"
#include "../feature_detection/harris_detector.h"
#include "../panorama/panorama.h"

using namespace std;
void test_func()
{
  printf("we got em lads\n");
}

vector<Image> smooth_frames(vector<Image> input)
{
  Video video;
  video.input_frames = input;                                            // N frames
  video.features = vector<vector<Descriptor>>(input.size());             // N lists of features
  video.smoothed_features = vector<vector<Point>>(input.size());         // N lists of smoothed features
  video.smoothed_descriptors = vector<vector<Descriptor>>(input.size()); // N lists of smoothed descriptors
  video.timewise_homographies = vector<Matrix>(input.size() - 1);        // N - 1 timewise homographies
  video.smoothing_homographies = vector<Matrix>(input.size());           // N smoothing homographies
  video.output_frames = vector<Image>(input.size());

  // PHASE 1
  // get all features for all frames
  printf("getting features\n");
  get_features_per_frame(video);

  // compute homography between each timestep
  printf("computing timewise homography\n");
  compute_timewise_homogrpahies(video);

  // PHASE 2
  // using the homographies we can calculate the way a point
  // moves through each timestep. We can then smooth a feature's trajectory
  // for some window
  printf("smoothing feature points\n");
  smooth_feature_points(video, 3.0f);

  // PHASE 3
  // Now that we have the smooth features for each time step we need to compute
  // homography for each timestep that takes a given frame from the raw features
  // to the smoothed feature
  printf("computing smoothing homography\n");
  compute_smoothing_homography(video);

  printf("smoothing images\n");
  smooth_images(video);

  printf("completed\n");
  return video.output_frames;
}

void get_features_per_frame(Video &video)
{
  for (int i = 0; i < video.input_frames.size(); i++)
  {
    vector<Descriptor> features = harris_corner_detector(video.input_frames[i], 0.7, 0.25, 10, 3, 0); // im, sigma, thresh, window, nms, corner_method
    video.features[i] = features;
    printf("feature size %lu\n", features.size());
  }
}

void compute_timewise_homogrpahies(Video &video)
{
  for (int i = 0; i < video.input_frames.size() - 1; i++)
  {
    vector<Match> matches = match_descriptors(video.features[i], video.features[i + 1]);
    Matrix H = RANSAC(matches, 5, 10000, 50);
    video.timewise_homographies[i] = H;
  }
}

void normalize_1d_gaussian(Image &gaussian)
{
  float sum = 0;
  for (int i = 0; i < gaussian.w; i++)
  {
    sum += gaussian(i, 0, 0);
  }

  for (int i = 0; i < gaussian.w; i++)
  {
    gaussian(i, 0, 0) /= sum;
  }
}

void smooth_feature_points(Video &video, float sigma)
{
  Image gaussian_filter = make_1d_gaussian(sigma);
  normalize_1d_gaussian(gaussian_filter);

  // for each timestep smooth all the features according to a gaussian
  // filter
  for (int t = 0; t < video.features.size(); t++)
  { // t is timestep
    vector<Descriptor> features_at_time = video.features[t];
    vector<Point> smoothed_features_at_time = vector<Point>(features_at_time.size());
    for (int i = 0; i < features_at_time.size(); i++)
    { // j is feature @ timestep i

      // initialize smoothed point to the raw feature multiplied by the gaussian at the
      // the centre index
      float smooth_x = features_at_time[i].p.x * gaussian_filter(gaussian_filter.w / 2, 0, 0);
      float smooth_y = features_at_time[i].p.y * gaussian_filter(gaussian_filter.w / 2, 0, 0);
      Point left_pt(features_at_time[i].p.x, features_at_time[i].p.y);
      Point right_pt(features_at_time[i].p.x, features_at_time[i].p.y);
      for (int j = 1; j < gaussian_filter.w / 2; j++)
      {
        // get timestamp @ at interval ring from target time
        int timestep_l = MAX(0, t - j);
        int timestep_r = MIN(t + j, video.features.size() - 1);

        // get homographies from timestep_l to timestep_l + 1
        // and timestep_r - 1 to timestep_r to see how the previous
        // pixel moved a frame earlier and a frame later
        Matrix H_l = t - j <= 0 ? Matrix::identity_homography() : video.timewise_homographies[timestep_l];
        Matrix H_r = t + j >= video.features.size() ? Matrix::identity_homography() : video.timewise_homographies[timestep_r - 1];

        // need to invert left homography becasue we're going backwards
        Matrix H_l_inv = H_l.inverse();

        // project point forwared and back in time with appropriate homography
        Point projected_left = project_point(H_l_inv, left_pt);
        Point projected_right = project_point(H_r, right_pt);

        left_pt = projected_left;
        right_pt = projected_right;

        // adjust our new smoothed point by the point at i - j and i + j multiplied by the
        // gaussian function
        smooth_x += ((left_pt.x * gaussian_filter((gaussian_filter.w / 2) - j, 0, 0)) + right_pt.x * gaussian_filter((gaussian_filter.w / 2) + j, 0, 0));
        smooth_y += ((left_pt.y * gaussian_filter((gaussian_filter.w / 2) - j, 0, 0)) + right_pt.y * gaussian_filter((gaussian_filter.w / 2) + j, 0, 0));
      }

      // now we have found our newly projected point
      // printf("xs %f ys %f xr %f yr %f\n", smooth_x, smooth_y, features_at_time[i].p.x, features_at_time[i].p.y);
      Point smooth_pt(smooth_x, smooth_y);
      smoothed_features_at_time[i] = smooth_pt;
    }
    video.smoothed_features[t] = smoothed_features_at_time;
  }
}

void compute_smoothing_homography(Video &video)
{
  for (int i = 0; i < video.smoothed_features.size(); i++)
  {
    vector<Descriptor> descriptors;
    for (int j = 0; j < video.smoothed_features[i].size(); j++)
    {
      Descriptor d;
      d.p = video.smoothed_features[i][j];
      descriptors.push_back(d);
    }
    video.smoothed_descriptors[i] = descriptors;
  }

  // go through each timestep
  // create a match object with raw points and smooth points for that step
  // compute homography using that match object
  // store homography
  for (int i = 0; i < video.features.size(); i++)
  {
    vector<Match> matches;
    for (int j = 0; j < video.features[i].size(); j++)
    {
      Match match;
      match.a = &video.features[i][j];
      match.b = &video.smoothed_descriptors[i][j];
      matches.push_back(match);
    }
    Matrix smooth_homography = compute_homography_ba(matches);
    video.smoothing_homographies[i] = smooth_homography;
  }
}

void smooth_images(Video &video)
{
  for (int t = 0; t < video.input_frames.size(); t++)
  {
    Image current_frame = video.input_frames[t];
    Matrix smoothing_homography = video.smoothing_homographies[t];    // from raw to smooth
    Matrix smoothing_homography_inv = smoothing_homography.inverse(); // from smooth to raw

    Point c1 = project_point(smoothing_homography, Point(0, 0));
    Point c2 = project_point(smoothing_homography, Point(current_frame.w - 1, 0));
    Point c3 = project_point(smoothing_homography, Point(0, current_frame.h - 1));
    Point c4 = project_point(smoothing_homography, Point(current_frame.w - 1, current_frame.h - 1));

    // Find top left and bottom right corners of image b warped into image a.
    Point topleft, botright;
    botright.x = max(c1.x, max(c2.x, max(c3.x, c4.x))); // smooth image bottom right x
    botright.y = max(c1.y, max(c2.y, max(c3.y, c4.y))); // smooth image bottom right y
    topleft.x = min(c1.x, min(c2.x, min(c3.x, c4.x)));  // smooth image top left x
    topleft.y = min(c1.y, min(c2.y, min(c3.y, c4.y)));  // smooth image top left y

    // used to crop to the minimum bounding box
    botright.x = min(c4.x, c2.x);
    botright.y = min(c4.y, c3.y);
    topleft.x = max(c1.x, c3.x);
    topleft.y = max(c1.y, c2.y);

    // bound protection for frames that project oddly
    topleft.x = MAX(0.0, topleft.x);
    topleft.y = MAX(0.0, topleft.y);
    botright.x = MIN(current_frame.w, botright.x);
    botright.y = MIN(current_frame.h, botright.y);

    int dx = min(0, (int)topleft.x);
    int dy = min(0, (int)topleft.y);
    int w = max(current_frame.w, (int)botright.x) - dx;
    int h = max(current_frame.h, (int)botright.y) - dy;

    Image output_frame(w, h, current_frame.c);

    for (int j = topleft.y; j < botright.y; j++)
    {
      for (int i = topleft.x; i < botright.x; i++)
      {
        Point projected = project_point(smoothing_homography_inv, Point(i, j)); // projected from smooth to raw to see what pixel it aligns to
        if (projected.x >= 0 && projected.x < current_frame.w && projected.y >= 0 && projected.y < current_frame.h)
        {
          float value_c0 = current_frame.bilinear_interpolate(projected.x, projected.y, 0);
          float value_c1 = current_frame.bilinear_interpolate(projected.x, projected.y, 1);
          float value_c2 = current_frame.bilinear_interpolate(projected.x, projected.y, 2);
          output_frame.set_pixel(i - dx, j - dy, 0, value_c0);
          output_frame.set_pixel(i - dx, j - dy, 1, value_c1);
          output_frame.set_pixel(i - dx, j - dy, 2, value_c2);
        }
      }
    }
    video.output_frames[t] = trim_image(output_frame).bilinear_resize(current_frame.w, current_frame.h);
  }
}