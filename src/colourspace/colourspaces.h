#pragma once

#include <cassert>
#include <cstring>
#include <cmath>
#include <stdexcept>


struct Color {
  union {
    struct { float red,green,blue,alpha; };
    struct { float r,g,b,a; };
    float c[4];
  };

  Color(float r,float g,float b,float a=1.0) : red(r), green(g), blue(b), alpha(a) {}
  Color() : red(0), green(0), blue(0), alpha(1) {}

  static Color HSV(float hue, float sat = 1.0f, float val = 1.0f, float alpha = 1.0f) {
    float h = 6.0f * hue;
    if(h==6.f)h=h-1;
    int i = (int)floor(h);
    float f = (i%2 == 0) ? 1-(h-i) : h-i;
    float m = val * (1-sat);
    float n = val * (1-sat*f);

    switch(i) {
      case 0: return Color(val,n,m,alpha);
      case 1: return Color(n,val,m,alpha);
      case 2: return Color(m,val,n,alpha);
      case 3: return Color(m,n,val,alpha);
      case 4: return Color(n,m,val,alpha);
      case 5: return Color(val,m,n,alpha);
      default: throw std::runtime_error("Found extra colour in rainbow.");
    }
  }


};


struct RGBcolour { float r,g,b; };
struct HSVcolour { float h,s,v; };
struct XYZcolour { float x,y,z; };
struct LCHcolour { float l,c,h; };

float l2g(float a);

float g2l(float a);

RGBcolour linear2gamma(RGBcolour a);

RGBcolour gamma2linear(RGBcolour a);

XYZcolour toXYZ(RGBcolour a);

RGBcolour toRGB(XYZcolour a);

LCHcolour rgb2lch(RGBcolour a);

RGBcolour lch2rgb(LCHcolour a);

HSVcolour rgb2hsv(RGBcolour a);

RGBcolour hsv2rgb(HSVcolour a);