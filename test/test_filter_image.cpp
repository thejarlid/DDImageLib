#include "test_common.h"
#include "../src/image/inc/filter_image.h"

using namespace std;


void test_highpass_filter() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image f = make_highpass_filter();
  Image blur = convolve_image(im, f, 0);
  blur.clamp();
  Image gt = load_image("data/dog-highpass.png");
  save_image(blur, "output/highpass-dog.jpg");
  TEST((blur == gt));
}


void test_emboss_filter() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image f = make_emboss_filter();
  Image blur = convolve_image(im, f, 1);
  blur.clamp();
  
  Image gt = load_image("data/dog-emboss.png");
  save_image(blur, "output/emboss-dog.jpg");
  TEST((blur == gt));
}


void test_sharpen_filter() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image f = make_sharpen_filter();
  Image blur = convolve_image(im, f, 1);
  blur.clamp();
  
  Image gt = load_image("data/dog-sharpen.png");
  save_image(blur, "output/sharpen-dog.jpg");
  TEST((blur == gt));
}


void test_convolution() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image f = make_box_filter(7);
  Image blur = convolve_image(im, f, 1);
  blur.clamp();
  
  Image gt = load_image("data/dog-box7.png");
  save_image(blur, "output/box-fliter-7-dog.jpg");
  TEST((blur == gt));
}


void test_gaussian_filter() {
  printf("%s\n", __func__);
  Image f = make_gaussian_filter(7);
  
  for(int i = 0; i < f.w * f.h * f.c; i++)f.data[i] *= 100;
  
  Image gt = load_image("data/gaussian_filter_7.png");
  TEST((f == gt));
}


void test_gaussian_blur() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image f = make_gaussian_filter(2);
  Image blur = convolve_image(im, f, 1);
  blur.clamp();
  
  Image gt = load_image("data/dog-gauss2.png");
  save_image(blur, "output/gaussian-fliter-2-dog.jpg");
  TEST((blur == gt));
}


void test_hybrid_image() {
  printf("%s\n", __func__);
  Image man = load_image("data/melisa.png");
  Image woman = load_image("data/aria.png");
  Image f = make_gaussian_filter(2);
  Image lfreq_man = convolve_image(man, f, 1);
  Image lfreq_w = convolve_image(woman, f, 1);
  Image hfreq_w = woman - lfreq_w;
  Image reconstruct = lfreq_man + hfreq_w;
  Image gt = load_image("data/hybrid.png");
  reconstruct.clamp();
  save_image(reconstruct, "output/hybrid");
  TEST((reconstruct == gt));
}


void test_frequency_image() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image f = make_gaussian_filter(2);
  Image lfreq = convolve_image(im, f, 1);
  Image hfreq = im - lfreq;
  Image reconstruct = lfreq + hfreq;
  
  Image low_freq = load_image("data/low-frequency.png");
  Image high_freq = load_image("data/high-frequency-clamp.png");
  
  lfreq.clamp();
  hfreq.clamp();

  save_image(low_freq, "output/low-frequency-dog");
  save_image(low_freq, "output/high-frequency-dog");
  TEST((lfreq == low_freq));
  TEST((hfreq == high_freq));
  TEST((reconstruct == im));
}


void test_sobel() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  pair<Image,Image> res = sobel_image(im);
  Image mag = res.first;
  Image theta = res.second;
  
  mag.feature_normalize();
  theta.feature_normalize();
  
  Image gt_mag = load_image("data/magnitude.png");
  Image gt_theta = load_image("data/theta.png");
  TEST(gt_mag.w == mag.w && gt_theta.w == theta.w);
  TEST(gt_mag.h == mag.h && gt_theta.h == theta.h);
  TEST(gt_mag.c == mag.c && gt_theta.c == theta.c);
  if( gt_mag.w != mag.w || gt_theta.w != theta.w || 
      gt_mag.h != mag.h || gt_theta.h != theta.h || 
      gt_mag.c != mag.c || gt_theta.c != theta.c ) return;
  
  for(int i = 0; i < gt_mag.w*gt_mag.h; ++i){
      if(within_eps(gt_mag.data[i], 0)){
          gt_theta.data[i] = 0;
          theta.data[i] = 0;
      }
      if(within_eps(gt_theta.data[i], 0) || within_eps(gt_theta.data[i], 1)){
          gt_theta.data[i] = 0;
          theta.data[i] = 0;
      }
  }
  
  save_png(mag,"output/mag");
  save_png(theta,"output/theta");
  
  Image imo=colorize_sobel(im);
  save_png(imo,"output/color_sobel");
  
  TEST((mag == gt_mag));
  TEST((theta == gt_theta));
}

void test_bilateral() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image bif= bilateral_filter(im,3,0.1);
  
  save_png(bif,"output/bilateral");
}


void run_tests() {
  test_gaussian_filter();
  test_sharpen_filter();
  test_emboss_filter();
  test_highpass_filter();
  test_convolution();
  test_gaussian_blur();
  test_hybrid_image();
  test_frequency_image();
  test_sobel();
  test_bilateral();

  printf("%d tests, %d passed, %d failed\n", tests_total, tests_total-tests_fail, tests_fail);
}

int main(int argc, char **argv) {
  run_tests();
  return 0;
}