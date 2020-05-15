// You probably don't want to edit this file
#include <cstdio>
#include <cstdlib>

#include <string>

#include "../inc/image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../inc/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../inc/stb_image_write.h"

void save_image_stb(const Image& im, const string& name, int png)
  {
  unsigned char *data = (unsigned char *)calloc(im.w*im.h*im.c, sizeof(char));
  
  for(int k = 0; k < im.c; ++k)for(int i = 0; i < im.w*im.h; ++i)
    data[i*im.c+k] = (unsigned char) roundf((255*im.data[i + k*im.w*im.h]));
  
  string file=name + (png?".png":".jpg");
  
  int success = 0;
  if(png)success = stbi_write_png(file.c_str(), im.w, im.h, im.c, data, im.w*im.c);
  else   success = stbi_write_jpg(file.c_str(), im.w, im.h, im.c, data, 100);
  
  free(data);
  
  if(!success) fprintf(stderr, "Failed to write image %s\n", file.c_str());
  }

void save_png(const Image& im, const string& name) { save_image_stb(im, name, 1); }

void save_image(const Image& im, const string& name) { save_image_stb(im, name, 0); }

// 
// Load an image using stb
// channels = [0..4]
// channels > 0 forces the image to have that many channels
//
Image load_image_stb(const string& filename, int channels)
  {
  int w, h, c;
  unsigned char *data = stbi_load(filename.c_str(), &w, &h, &c, channels);
  if (!data)
    {
    fprintf(stderr, "Cannot load image \"%s\"\nSTB Reason: %s\n", filename.c_str(), stbi_failure_reason());
    exit(0);
    }
  
  if (channels) c = channels;
  
  int i,j,k;
  Image im(w, h, c);
  
  for(k = 0; k < c; ++k)
    for(j = 0; j < h; ++j)
      for(i = 0; i < w; ++i)
        {
        int dst_index = i + w*j + w*h*k;
        int src_index = k + c*i + c*w*j;
        im.data[dst_index] = (float)data[src_index]/255.;
        }
  //We don't like alpha channels, #YOLO
  if(im.c == 4) im.c = 3;
  free(data);
  return im;
  }

Image load_image(const string& filename) { return load_image_stb(filename,0); }

// #ifdef OPENCV

// void rgbgr_image(Image& im)
//   {
//   for(int i = 0; i < im.w*im.h; ++i)swap(im.data[i],im.data[i+im.w*im.h*2]);
//   }

// Image Mat2Image(const cv::Mat& frame)
//   {
//   Image im(frame.cols,frame.rows,3);
//   unsigned char* data=frame.data;
//   for(int q2=0;q2<frame.rows;q2++)for(int q1=0;q1<frame.cols;q1++)for(int c=0;c<3;c++)im(q1,q2,c)=(*(data++))/255.0;
//   //printf("%d %d %d %d\n",im.w,im.h,frame.channels(),im.c);
//   rgbgr_image(im);
//   return im;
//   }

// cv::Mat Image2Mat(const Image& im)
//   {
//   Image im2=im;
//   rgbgr_image(im2);
//   cv::Mat frame(im2.h,im2.w,CV_8UC3);
//   unsigned char* data=frame.data;
//   for(int q2=0;q2<frame.rows;q2++)for(int q1=0;q1<frame.cols;q1++)for(int c=0;c<3;c++)*(data++)=(unsigned char)(im2(q1,q2,c)*255.0);
//   return frame;
//   }

// Image get_image_from_stream(cv::VideoCapture& cap)
//   {
//   cv::Mat frame;
//   cap >> frame; // get a new frame from camera
//   return Mat2Image(frame);
//   }

// #endif
