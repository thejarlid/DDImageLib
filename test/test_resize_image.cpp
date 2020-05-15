#include "test_common.h"

using namespace std;

void test_nn_resize() {
  printf("%s\n", __func__);
  Image im = load_image("data/dogsmall.jpg");
  Image resized = im.nn_resize(im.w*4, im.h*4);
  Image gt = load_image("data/dog4x-nn-for-test.png");
  TEST((resized == gt));
  
  Image im2 = load_image("data/dog.jpg");
  Image resized2 = im2.nn_resize(713, 467);
  Image gt2 = load_image("data/dog-resize-nn.png");
  TEST((resized2 ==gt2));
}


void test_bl_resize() {
  printf("%s\n", __func__);
  Image im = load_image("data/dogsmall.jpg");
  Image resized = im.bilinear_resize(im.w*4, im.h*4);
  Image gt = load_image("data/dog4x-bl.png");
  save_image(resized, "resized1.jpg");
  TEST((resized == gt));

  Image im2 = load_image("data/dog.jpg");
  Image resized2 = im2.bilinear_resize(713, 467);
  Image gt2 = load_image("data/dog-resize-bil.png");
  save_image(resized2, "resized2.jpg");
  TEST((resized2 == gt2));
}


void test_multiple_resize() {
  printf("%s\n", __func__);
  Image im = load_image("data/dog.jpg");
  for (int i = 0; i < 10; i++) {
    Image im1 = im.bilinear_resize(im.w*4, im.h*4);
    Image im2 = im1.bilinear_resize(im1.w/4, im1.h/4);
    im = im2;
    save_image(im, "multi.jpg");
  }
  Image gt = load_image("data/dog-multipleresize.png");
  TEST((im == gt));
}


void run_tests() {
  test_nn_resize();
  test_bl_resize();
  test_multiple_resize();
  printf("%d tests, %d passed, %d failed\n", tests_total, tests_total-tests_fail, tests_fail);
}

int main(int argc, char **argv) {
  run_tests();
  return 0;
}
