#include <cstdio>
#include <cstring>
#include <cassert>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>

#include "../inc/image.h"
#include "../../utils/utils.h"

using namespace std;

// MARK: - Constructor

Image::Image() : Image(0, 0, 0) {}


Image::Image(int w, int h, int c) : w(w), h(h), c(c), data(nullptr) {
  // if there is data to be allocated then allocate a data zero'd out 
  // data buffer to hold the image content 
  if (w*h*c) {
    data = (float*)calloc(w*h*c, sizeof(float));
  }
}


// MARK: - Destructor

Image::~Image() {
  free(data);
}

  
// MARK: - Copy Constructor

Image::Image(const Image& other) : data(nullptr) {
  *this = other;
}


// MARK: - Move Contructor

Image::Image(Image&& other) : data(nullptr) {
  *this = move(other);
}


Image& Image::operator=(const Image& other) {
  if (this == &other) {
    return *this;
  }

  if (data) {
    free(data);
    data = nullptr;
  }

  w = other.w;
  h = other.h;
  c = other.c;

  data = (float*)calloc(w*h*c, sizeof(float));
  memcpy(data, other.data, sizeof(float)*w*h*c);
  return *this;
}


Image& Image::operator=(Image&& other) {
  if (this == &other) {
    return *this;
  }

  if (data) {
    free(data);
  }

  w = other.w;
  h = other.h;
  c = other.c;
  data = other.data;

  other.data = nullptr;
  other.w = other.h = other.c = 0;
  return *this;
}


bool Image::operator==(Image& other) {
  if (w != other.w || h != other.h || c != other.c) {
    printf("Expected %d x %d x %d image, got %d x %d x %d\n", other.w, other.h, other.c, w, h, c);
    return 0;
  }

  for(int i = 0; i < w*h*c; ++i) if(!within_eps(data[i], other.data[i])) {
    printf("The value at %d %d %d should be %f, but it is %f! \n", i/(w*h), (i%(w*h))/h, (i%(w*h))%h, other.data[i], data[i]);
    return 0;
  }
  return 1;
}


float& Image::operator()(int x, int y, int ch) {
  assert(ch<c && ch>=0 && x<w && x>=0 && y<h && y>=0);
  return data[ch*w*h + y*w + x];
}


float& Image::operator()(int x, int y) {
  assert(c==1 && x<w && x>=0 && y<h && y>=0);
  return data[y*w + x];
}


const float& Image::operator()(int x, int y, int ch) const {
  assert(ch<c && ch>=0 && x<w && x>=0 && y<h && y>=0);
  return data[ch*w*h + y*w + x];
}


const float& Image::operator()(int x, int y) const {
  assert(c==1 && x<w && x>=0 && y<h && y>=0);
  return data[y*w + x];
}


float& Image::pixel(int x, int y, int ch)               { return operator()(x,y,ch); }
const float& Image::pixel(int x, int y, int ch) const   { return operator()(x,y,ch); }
float& Image::pixel(int x, int y)                       { return operator()(x,y); }
const float& Image::pixel(int x, int y) const           { return operator()(x,y); }


float Image::get_pixel(int x, int y, int ch) const {
  assert(ch<c && ch>=0);
  x = x >= w ? w-1 : x;
  x = x < 0 ? 0 : x;
  y = y >= h ? h-1 : y;
  y = y < 0 ? 0 : y;
  return data[ch*h*w + y*w + x];
}


float Image::get_pixel(int x, int y) const {
  assert(c==1);
  return get_pixel(x, y, 0);
}
    

void Image::set_pixel(int x, int y, int ch, float v) {
  if(x>=w)  return;
  if(y>=h)  return;
  if(ch>=c) return;
  if(x<0)   return;
  if(y<0)   return;
  if(ch<0)  return;
  data[ch*w*h + y*w + x] = v;
}


void Image::set_channel(int ch, const Image& im) {
  assert(im.c==1 && ch<c && ch>=0);
  assert(im.w==w && im.h==h);
  memcpy(&pixel(0,0,ch),im.data,sizeof(float)*im.size());
}


Image Image::get_channel(int ch) const  {
  assert(ch<c && ch>=0);
  Image im(w,h,1);
  memcpy(im.data,&pixel(0,0,ch),sizeof(float)*im.size());
  return im;
}


const float* Image::RowPtr(int row, int ch) const  { return data + ch*w*h + row*w; }
float* Image::RowPtr(int row, int ch)              { return data + ch*w*h + row*w; }
  
  
bool Image::contains(float x, float y) const { 
  return x > -0.5f && x < w - 0.5f && y > -0.5f && y < h - 0.5f; 
}


bool Image::is_empty(int x, int y) const {
  assert(x < w && x >= 0 && y < h && y >= 0);
  for (int ch = 0; ch < c; ch++) {
    if(pixel(x, y, ch)) {
      return false;
    }
  }
  return true;
}


bool Image::is_nonempty_patch(int x, int y, int w) const {
  int channelFlag = 0;
  for (int xPos = x-w; xPos <= x+w; xPos++) {
    for (int yPos = y-w; yPos <= y+w; yPos++) {
      for (int ch = 0; ch < c; ch++) {
        if (get_pixel(xPos, yPos, ch)) {
          channelFlag++;
        }
      }
    }
  }
  return channelFlag != 0;
}


void Image::l1_normalize() {
  float sum = 0;
  for (int ch=0; ch < c; ch++) {
    for (int row=0; row < h; row++) {
      for (int col=0; col < w; col++) {
        sum += get_pixel(row, col, ch);
      }
    }
  }

  for (int ch=0; ch < c; ch++) {
    for (int row=0; row < h; row++) {
      for (int col=0; col < w; col++) {
        set_pixel(row, col, ch, get_pixel(row, col, ch)/sum);
      }
    }
  }
}


int Image::size() const {
  return w*h*c; 
}


void Image::clear() const { 
  memset(data, 0, sizeof(float)*c*w*h); 
}


Image Image::abs(void) const  {
  Image ret=*this;
  for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++) {
    for(int q3=0;q3<c;q3++) {
      float a=pixel(q1,q2,q3);
      ret(q1,q2,q3)=fabsf(a);
    }
  }
  return ret;
}


template <size_t TSZ>
void TiledTranspose(Image& img_out, const Image& img_in, int c) {
  const size_t w = img_in.w;
  const size_t h = img_in.h;
  const size_t BPP = sizeof(float);
  
  float d[TSZ][TSZ];
  
  for(size_t xin = 0; xin < w; xin += TSZ) {
    for(size_t yin = 0; yin < h; yin += TSZ) {
      const size_t xspan = min(TSZ, w - xin);
      const size_t yspan = min(TSZ, h - yin);
      const size_t dmin = min(xspan, yspan);
      const size_t dmax = max(xspan, yspan);
      const size_t xout = yin;
      const size_t yout = xin;
      
      for(size_t y = 0; y < yspan; y++) {
        memcpy(d[y], &img_in(xin, yin + y, c), xspan * BPP);
      }
      
      for(size_t x = 0; x < dmin; x++) {
        for(size_t y = x + 1; y < dmax; y++) {
          swap(d[x][y], d[y][x]);
        }
      }
      
      for(size_t y = 0; y < xspan; y++) {
        memcpy(&img_out(xout, yout + y,  c), d[y], yspan * BPP);
      }
    }
  }
}


Image Image::transpose(void) const {
  //TIME(1);
  Image ret(h,w,c);
  
  if(c>1) {
    vector<thread> th;
    for(int c=0;c<this->c;c++)th.push_back(thread([&ret,this,c](){TiledTranspose<80>(ret,*this,c);}));
    for(auto&e1:th)e1.join();
  }
  else TiledTranspose<80>(ret,*this,0);
  
  return ret;
}


void Image::save(const string& file) {
  FILE* fn = fopen(file.c_str(),"wb");
  fwrite(&w, sizeof(w), 1, fn);
  fwrite(&h, sizeof(h), 1, fn);
  fwrite(&c, sizeof(c), 1, fn);
  fwrite(data, sizeof(float), size(), fn);
  fclose(fn);
}
  

static Image load(const string& file) {
  int w, h, c;
  FILE* fn = fopen(file.c_str(), "rb");
  fread(&w, sizeof(w), 1, fn);
  fread(&h, sizeof(h), 1, fn);
  fread(&c, sizeof(c), 1, fn);
  Image im(w, h, c);
  fread(im.data, sizeof(float), im.size(), fn);
  fclose(fn);
  return im;
}