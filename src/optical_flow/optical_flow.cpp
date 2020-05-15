#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cassert>
#include <chrono>

#include "optical_flow.h"
#include "../colourspace/colourspaces.h"

using namespace std;

// Calculate the time-structure matrix of an Image pair.
// const Image& im: the input Image.
// const Image& prev: the previous Image in sequence.
// float s: sigma used for Gaussian smoothing the gradients
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          3rd channel is IxIy, 4th channel is IxIt, 5th channel is IyIt.
Image time_structure_matrix(const Image& im, const Image& prev, float s) {
  assert(im.c==1 && prev.c==1 && "Only for grayscale images");

  Image S(im.w,im.h,5);

  // TODO: calculate gradients, structure components, and smooth them
  Image x_derivative_fiter(3, 1, 1);
  x_derivative_fiter(0, 0, 0) = -1;
  x_derivative_fiter(1, 0, 0) = 0;
  x_derivative_fiter(2, 0, 0) = 1;

  Image y_derivative_fiter(1, 3, 1);
  y_derivative_fiter(0, 0, 0) = -1;
  y_derivative_fiter(0, 1, 0) = 0;
  y_derivative_fiter(0, 2, 0) = 1;

  Image ix = convolve_image(prev, x_derivative_fiter, 0);
  Image iy = convolve_image(prev, y_derivative_fiter, 0);

  for (int y = 0; y < im.h; y++) {
    for (int x = 0; x < im.w; x++) {
      S(x, y, 0) = ix(x, y, 0) * ix(x, y, 0);
      S(x, y, 1) = iy(x, y, 0) * iy(x, y, 0);
      S(x, y, 2) = ix(x, y, 0) * iy(x, y, 0);
      float it = im(x, y, 0) - prev(x, y, 0);
      S(x, y, 3) = -(ix(x, y, 0) * it);
      S(x, y, 4) = -(iy(x, y, 0) * it);
    }
  }

  return fast_smooth_image(S, s);
}

// Compute the eigenvalues of the structure matrix
// Compute the eigenvalues only of S'S (the first three channels only)
// const Image& ts: the time-structure matrix
// returns: 2-channel image: 0-th channel : biggest eigenvalue,
//                           1-st channel : smallest
Image eigenvalue_matrix(const Image& ts) {

  Image im(ts.w,ts.h,2);

  for (int j = 0; j < ts.h; j++) {
    for (int i = 0; i < ts.w; i++) {
      float b = (-ts(i, j, 0) - ts(i, j, 1));
      float a = 1;
      float c = ((ts(i, j, 0) * ts(i, j, 1)) - (ts(i, j, 2) * ts(i, j, 2)));
      float e1 = (-b + sqrt((b * b) - 4 * a * c))/(2 * a);
      float e2 = (-b - sqrt((b * b) - 4 * a * c))/(2 * a);

      if (e1 < e2) {
        im(i, j, 0) = e2;
        im(i, j, 1) = e1;
      } else {
        im(i, j, 0) = e1;
        im(i, j, 1) = e2;
      }
    }
  }

  return im;
}


vector<Image> make_image_pyramid(const Image& a, float factor, int levels) {
  assert(a.c==1 && "Only for grayscale");


  vector<Image> imgs(levels);
  imgs[0]=a;

  for(int l=1;l<levels;l++) {
    Image f=fast_smooth_image(imgs[l-1],factor/1.5);

    int bw=max(1,(int)(f.w/factor));
    int bh=max(1,(int)(f.h/factor));
    imgs[l]=f.bilinear_resize(bw,bh);
  }

  return imgs;
}




// Calculate the velocity given a structure Image
// const Image& S: time-structure Image
// const Image& ev: eigenvalue image
// Return: 2 channel (u,v) image  : the x and y
// velocities computed by inv(S'S)*(S'T)
Image velocity_image(const Image& S,const Image& ev) {
  Image v(S.w, S.h, 2);

  // TODO: compute velocity for each pixel using (S'S)(S'T) formula
  // Use the class Matrix2x2 and Vector2 insted of Matrix
  // (lots of memory allocations with Matrix -> slow)
  // Use the eigenvalue image to avoid computing flow
  // if the smallest eigenvalue is smaller than 1e-5
  // In that case just set it to (0,0)
  float threshold = 1e-5f;
  for (int j = 0; j < S.h; j++) {
    for (int i = 0; i < S.w; i++) {
      if (ev(i, j, 1) < threshold) {
        v(i, j, 0) = 0;
        v(i, j, 1) = 0;
      } else {
        Matrix2x2 sts(S(i, j, 0), S(i, j, 2), S(i, j, 2), S(i, j, 1));
        sts = sts.inverse();
        Vector2 st(S(i, j, 3), S(i, j, 4));
        float x = sts.a * st.a + sts.b * st.b;
        float y = sts.c * st.a + sts.d * st.b;
        v(i, j, 0) = x;
        v(i, j, 1) = y;
      }
    }
  }

  return v;
}

// Constrain the absolute value of each Image pixel
// const Image& im: Image to constrain
// float v: each pixel will be in range [-v, v]
void constrain_image(const Image& im, float v) {
  for(int i = 0; i < im.w*im.h*im.c; ++i) {
    if (im.data[i] < -v) im.data[i] = -v;
    if (im.data[i] >  v) im.data[i] =  v;
  }
}

// const Image& im: input image
// const Image& v: velocity image specifying how much each pixel moves
// return warped image, with same size as input image, as discussed on
// the github page.
Image warp_flow(const Image& im, const Image& v) {
  assert(im.c==1 && v.c==2 && "Only for grayscale and vel image needs 2 channels");
  assert(im.w==v.w && im.h==v.h && "Image and velocity need to be same size");

  float old_weight=1e-4;
  Image result=im;

  // create sum_weight and sum_weighted_value
  Image sum_weight(im.w, im.h, 1);
  Image sum_weighted_value(im.w, im.h, 1);
  int sum_total = 0;
  int sum_weighted_total = 0;
  for (int j = 0; j < im.h; j++) {
    for (int i = 0; i < im.w; i++) {
      sum_weight(i, j, 0) = old_weight;
      sum_weighted_value(i, j, 0) = im(i, j, 0) * old_weight;
      sum_weighted_total += im(i, j, 0) * old_weight;
      sum_total += old_weight;
    }
  }

  // TODO: Warp image "im" according to flow "v"
  for (int j = 0; j < im.h; j++) {
    for (int i = 0; i < im.w; i++) {
      float new_x = i + v(i, j, 0);
      float new_y = j + v(i, j, 1);
      int lower_x = floor(new_x);
      int lower_y = floor(new_y);
      int upper_x = lower_x + 1;
      int upper_y = lower_y + 1;

      // get weights
      float a1 = ((float)upper_x - new_x) * ((float)upper_y - new_y); // bottom right
      float a2 = (new_x - (float)lower_x) * ((float)upper_y - new_y); // bottom left
      float a3 = ((float)upper_x - new_x) * (new_y - (float)lower_y); // top right
      float a4 = (new_x - (float)lower_x) * (new_y - (float)lower_y); // top left


      sum_weight.set_pixel(lower_x, lower_y, 0, sum_weight.get_pixel(lower_x, lower_y) + a1);
      sum_weight.set_pixel(lower_x, upper_y, 0, sum_weight.get_pixel(lower_x, upper_y) + a3);
      sum_weight.set_pixel(upper_x, lower_y, 0, sum_weight.get_pixel(upper_x, lower_y) + a2);
      sum_weight.set_pixel(upper_x, upper_y, 0, sum_weight.get_pixel(upper_x, upper_y) + a4);

      sum_weighted_value.set_pixel(lower_x, lower_y, 0, sum_weighted_value.get_pixel(lower_x, lower_y) + (a1 * im(i, j, 0)));
      sum_weighted_value.set_pixel(lower_x, upper_y, 0, sum_weighted_value.get_pixel(lower_x, upper_y) + (a3 * im(i, j, 0)));
      sum_weighted_value.set_pixel(upper_x, lower_y, 0, sum_weighted_value.get_pixel(upper_x, lower_y) + (a2 * im(i, j, 0)));
      sum_weighted_value.set_pixel(upper_x, upper_y, 0, sum_weighted_value.get_pixel(upper_x, upper_y) + (a4 * im(i, j, 0)));
    }
  }

  // now need to normalize the pixels in the resulting image based on sum_weighted_value and sum_weight
  for (int j = 0; j < im.h; j++) {
    for (int i = 0; i < im.w; i++) {
      result(i, j, 0) = sum_weighted_value(i, j, 0) / sum_weight(i, j, 0);
    }
  }
  return result;
}

// Resize velocity image
// Image oldvel: old velocity
// int w,h : new sizes
// return new velocity image
Image velocity_resize(const Image& oldvel, int w, int h) {
  Image v(w,h,2);
  // TODO: resize the velocity image
  // do we just resize the image as normal or do we change the values as well?
  v = oldvel.bilinear_resize(w, h);
  float x_ratio = w/oldvel.w;
  float y_ratio = h/oldvel.h;
  for (int j = 0; j < h; j++) {
    for (int i = 0; i < w; i++) {
      v(i, j, 0) *= x_ratio;
      v(i, j, 1) *= y_ratio;
    }
  }
  return v;
}


void compute_iterative_pyramid_LK(LKIterPyramid& lk) {
  Image S;
  Image ev;
  Image v2;

  int h=lk.pyramid0[0].h;
  int w=lk.pyramid0[0].w;

  for(int q2=lk.pyramid_levels-1;q2>=0;q2--) {

    int pw=lk.pyramid1[q2].w;
    int ph=lk.pyramid1[q2].h;


    if(q2==lk.pyramid_levels-1) {
      lk.v=Image(pw,ph,2);
      lk.warped=lk.pyramid0[q2];
    }
    else {
      lk.v=velocity_resize(lk.v,pw,ph);
      lk.warped=warp_flow(lk.pyramid0[q2],lk.v);
    }

    for(int q1=0;q1<lk.lk_iterations;q1++) {
      S = time_structure_matrix(lk.pyramid1[q2], lk.warped, lk.smooth_structure);
      ev = eigenvalue_matrix(S);
      v2 = velocity_image(S, ev);

      v2=fast_smooth_image(v2,lk.smooth_vel);
      lk.v=lk.v+v2;

      constrain_image(lk.v,lk.clamp_vel);
      lk.warped=warp_flow(lk.pyramid0[q2],lk.v);
    }

  }



  lk.colorflow=vel2rgb(lk.v,lk.vel_color_scale);
  lk.error=(lk.warped-lk.pyramid1[0]).abs();

  if(lk.compute_all) {
    lk.all=Image(w*2,h*2,3);
    for(int c=0;c<3;c++)for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)lk.all(q1+0,q2+0,c)=lk.t1(q1,q2,c);
    for(int c=0;c<3;c++)for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)lk.all(q1+w,q2+0,c)=lk.colorflow(q1,q2,c);
    for(int c=0;c<3;c++)for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)lk.all(q1+0,q2+h,c)=lk.warped(q1,q2);
    for(int c=0;c<3;c++)for(int q2=0;q2<h;q2++)for(int q1=0;q1<w;q1++)lk.all(q1+w,q2+h,c)=lk.error(q1,q2);
  }

  if(lk.compute_colored_ev) {
    lk.ev3=Image(ev.w,ev.h,3);
    memcpy(lk.ev3.data,ev.data,ev.size()*sizeof(float));
  }

}


// Calculate the optical flow between two images
// const Image& im: current Image
// Image prev: previous Image
// float smooth_win: amount to smooth structure matrix by
// float smooth_vel: amount to smooth velocity image
// returns: velocity matrix
Image optical_flow_images(const Image& im, const Image& prev, float smooth_win, float smooth_vel) {

  Image S = time_structure_matrix(im, prev, smooth_win);
  Image ev = eigenvalue_matrix(S);
  Image v = velocity_image(S, ev);
  if(smooth_vel==0)return v;
  return fast_smooth_image(v,smooth_vel);
}


Image vel2rgb(const Image& v, float thres) {
  assert(v.c==2 && "velocity must contain 2 channels");
  Image ret(v.w,v.h,3);
  
  for(int q2=0;q2<v.h;q2++)for(int q1=0;q1<v.w;q1++) {
    float dx=v(q1,q2,0);
    float dy=v(q1,q2,1);
    //printf("%f %f %f\n",dx,dy,sqrtf(dx*dx+dy*dy));
    float mag=min(sqrtf(dx*dx+dy*dy)/thres,1.f);
    float hue=(atan2f(dy,dx)+M_PI)/2/M_PI;
    if(hue<0)hue=0;
    if(hue>1)hue=1;
    Color c=Color::HSV(hue,mag,mag);
    ret(q1,q2,0)=c.c[0];
    ret(q1,q2,1)=c.c[1];
    ret(q1,q2,2)=c.c[2];
  }
  return ret;
}