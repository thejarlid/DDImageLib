#include "video.h"
#include <vector>

using namespace std;

int main(int argc, char **argv) {
  test_func();
  vector<Image> test_im(1000); // 1000 for train 973 for car
  for (int i = 1; i <= 1000; i++) {
    char filename[26] = "frames_train/frame";
    char buf[4];
    sprintf(buf, "%04d", i);
    filename[18] = buf[0];
    filename[19] = buf[1];
    filename[20] = buf[2];
    filename[21] = buf[3];
    filename[22] = '.';
    filename[23] = 'j';
    filename[24] = 'p';
    filename[25] = 'g';
    string fn(filename, 26);

    Image img = load_image(fn);
    test_im[i - 1] = img;
  }
  printf("loading images complete\n");

  vector<Image> output = smooth_frames(test_im);
  for (int i = 0; i < output.size(); i++) {
    char filename[26] = "smooth_train/frame";
    char buf[4];
    sprintf(buf, "%04d", i);
    filename[18] = buf[0];
    filename[19] = buf[1];
    filename[20] = buf[2];
    filename[21] = buf[3];
    filename[22] = '.';
    filename[23] = 'j';
    filename[24] = 'p';
    filename[25] = 'g';
    string fn(filename, 26);
    save_image(output[i], filename);
  }

  test_func();
  return 0;
}
