#include "test_common.h"

using namespace std;

void test_get_pixel() {
  printf("%s\n", __func__);
  Image im = load_image("data/dots.png");
  // Test within image
  TEST(within_eps(0, im.get_pixel(0,0,0)));
  TEST(within_eps(1, im.get_pixel(1,0,1)));
  TEST(within_eps(0, im.get_pixel(2,0,1)));

  // Test padding
  TEST(within_eps(1, im.get_pixel(0,3,1)));
  TEST(within_eps(1, im.get_pixel(7,8,0)));
  TEST(within_eps(0, im.get_pixel(7,8,1)));
  TEST(within_eps(1, im.get_pixel(7,8,2)));
}


void test_set_pixel() {
  printf("%s\n", __func__);
  Image im = load_image("data/dots.png");
  Image d(4,2,3);
  
  d.set_pixel(0,0,0,0); d.set_pixel(0,0,1,0); d.set_pixel(0,0,2,0);
  d.set_pixel(1,0,0,1); d.set_pixel(1,0,1,1); d.set_pixel(1,0,2,1);
  d.set_pixel(2,0,0,1); d.set_pixel(2,0,1,0); d.set_pixel(2,0,2,0);
  d.set_pixel(3,0,0,1); d.set_pixel(3,0,1,1); d.set_pixel(3,0,2,0);
  
  d.set_pixel(0,1,0,0); d.set_pixel(0,1,1,1); d.set_pixel(0,1,2,0);
  d.set_pixel(1,1,0,0); d.set_pixel(1,1,1,1); d.set_pixel(1,1,2,1);
  d.set_pixel(2,1,0,0); d.set_pixel(2,1,1,0); d.set_pixel(2,1,2,1);
  d.set_pixel(3,1,0,1); d.set_pixel(3,1,1,0); d.set_pixel(3,1,2,1);
  
  // Test images are same
  TEST((im == d));
}


void test_grayscale() {
  printf("%s\n", __func__);
  Image im = load_image("data/colorbar.png");
  Image gray = im.rgb_to_grayscale();
  Image g = load_image("data/gray.png");
  TEST((gray == g));
}


void test_copy() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image c = im;
  TEST((im == c));
}


void test_shift() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image c = im;
  c.shift(1, 0.1);
  TEST(within_eps(im.data[0], c.data[0]));
  TEST(within_eps(im.data[im.w*im.h+13] + .1,  c.data[im.w*im.h + 13]));
  TEST(within_eps(im.data[2*im.w*im.h+72],  c.data[2*im.w*im.h + 72]));
  TEST(within_eps(im.data[im.w*im.h+47] + .1,  c.data[im.w*im.h + 47]));
}


void test_rgb_to_hsv() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  im.RGBtoHSV();
  Image hsv = load_image("data/dog.hsv.png");
  TEST((im == hsv));
}


void test_hsv_to_rgb() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image c = im;
  im.RGBtoHSV();
  im.HSVtoRGB();
  TEST((im == c));
}


void test_rgb2lch2rgb() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  Image c = im;
  im.RGBtoLCH();
  im.LCHtoRGB();
  TEST((im == c));
}


void run_tests() {
  test_get_pixel();
  test_set_pixel();
  test_copy();
  test_shift();
  test_grayscale();
  test_rgb_to_hsv();
  test_hsv_to_rgb();
  test_rgb2lch2rgb();
  printf("%d tests, %d passed, %d failed\n", tests_total, tests_total-tests_fail, tests_fail);
}

int main(int argc, char **argv) {
  run_tests();
  return 0;
}