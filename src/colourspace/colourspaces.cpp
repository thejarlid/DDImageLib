#include "colourspaces.h"
#include "../utils/utils.h"

float l2g(float a) {
  if (a < 0.0031308) {
    return 12.92*a;
  }
  return 1.055*powf(a,1.0f/2.4f)-0.055;
}


float g2l(float a) {
  if (a < 0.04045) {
    return a/12.92;
  }
  return powf((a+0.055f)/1.055f,2.4f);
}


RGBcolour linear2gamma(RGBcolour a){
  a.r = l2g(a.r);
  a.g = l2g(a.g);
  a.b = l2g(a.b);
  return a;
}


RGBcolour gamma2linear(RGBcolour a) {
  a.r = g2l(a.r);
  a.g = g2l(a.g);
  a.b = g2l(a.b);
  return a;
}


XYZcolour toXYZ(RGBcolour a) {
  XYZcolour b;
  a = gamma2linear(a);
  b.x = 0.412383*a.r + 0.357585*a.g + 0.18048*a.b;
  b.y = 0.212635*a.r + 0.71517*a.g + 0.072192*a.b;
  b.z = 0.01933*a.r + 0.119195*a.g + 0.950528*a.b;
  return b;
}


RGBcolour toRGB(XYZcolour a) {
  RGBcolour b;
  b.r = (3.24103)*a.x + (-1.53741)*a.y + (-0.49862)*a.z;
  b.g = (-0.969242)*a.x + (1.87596)*a.y + (0.041555)*a.z;
  b.b = (0.055632)*a.x + (-0.203979)*a.y + (1.05698)*a.z;
  b = linear2gamma(b);
  return b;
}


LCHcolour rgb2lch(RGBcolour a) {
  LCHcolour b = {0.f,0.f,0.f};
  XYZcolour c = toXYZ(a);
  
  if (c.x == 0.f && c.y == 0.f && c.z == 0.f) {
    return b;
  }
  
  
  float u1 = 4*c.x/(1*c.x+15*c.y+3*c.z);
  float v1 = 9*c.y/(1*c.x+15*c.y+3*c.z);
  
  
  float un = 0.2009;
  float vn = 0.4610;
  
  float cutoff = powf(6.f/29.f,3);
  
  float l=0;
  if(c.y<=cutoff)l=powf(29.f/3.f,3)*c.y;
  else l=116.f*powf(c.y,1.f/3.f)-16.f;
  float u=13.f*l*(u1-un);
  float v=13.f*l*(v1-vn);
  
  
  b.l=l;
  b.c=sqrtf(u*u+v*v);
  b.h=atan2f(u,v);
  
  return b;
}


RGBcolour lch2rgb(LCHcolour a) {
  XYZcolour b={0.f,0.f,0.f};
  
  if(a.l==0.f && a.c==0.f && a.h==0.f)return toRGB(b);
  
  float u=a.c*sinf(a.h);
  float v=a.c*cosf(a.h);
  float l=a.l;
  
  
  
  float un=0.2009;
  float vn=0.4610;
  
  float cutoff=8;
  
  
  float u1=u/(13.f*l)+un;
  float v1=v/(13.f*l)+vn;
  
  if(l<=cutoff)b.y=l*powf(3.f/29.f,3);
  else b.y=powf((l+16.f)/116.f,3);
  
  b.x=b.y*(9*u1)/(4*v1);
  b.z=b.y*(12-3*u1-20*v1)/(4*v1);
  
  
  return toRGB(b);
}


HSVcolour rgb2hsv(RGBcolour a) {
  HSVcolour b;

  float max = three_way_max(a.r, a.g, a.b);

  float min = three_way_min(a.r, a.g, a.b);

  // value is just the max of the rgb values
  float V = max;
  
  // saturation is the ratio between the difference of the min and max and how large max is
  float diff = max - min;
  float S = max == 0.0 ? 0.0 : diff/max;

  // hue is the hexagonal distance of our target colour from red
  float hue_prime = 0.0f;
  float hue = 0.0;
  if (diff != 0.0) {
    if (max == a.r) {
      hue_prime = (a.g - a.b)/diff;
    } else if (max == a.g) {
      hue_prime = 2 + (a.b - a.r)/diff;
    } else if (max == a.b) {
      hue_prime = 4 + (a.r - a.g)/diff;
    }

    if (hue_prime < 0) {
      hue = 1 + (hue_prime/6);
    } else {
      hue = hue_prime/6;
    }
  }

  b.h = hue;
  b.s = S;
  b.v = V;
  return b;
}


RGBcolour hsv2rgb(HSVcolour a) {
  RGBcolour b;
  float max = a.v;
  float diff = a.s * max;
  float min = max - diff;

  float red = 0;
  float green = 0;
  float blue = 0;

  if (diff != 0.0) {
    float hue_prime = a.h * 6.0;
    float x = diff * (1.0 - fabs(fmod(hue_prime, 2) - 1));
    if (hue_prime >= 4) {
      if (hue_prime <= 5) {
        red = x + min;
        green = min;
        blue = diff + min;
      } else {
        red = diff + min;
        green = min;
        blue = x + min;
      }
    } else if (hue_prime >= 2) {
      if (hue_prime <= 3) {
        red = min;
        green = diff + min;
        blue = x + min;
      } else {
        red = min;
        green = x + min;
        blue = diff + min;
      }
    } else {
      if (hue_prime <= 1) {
        red = diff + min;
        green = x + min;
        blue = min;
      } else {
        red = x + min;
        green = diff + min;
        blue = min;
      }
    }
  } else {
    red = green = blue = max;
  }

  b.r = red;
  b.g = green;
  b.b = blue;
  return b;
}