#pragma once

#include <cassert>
#include <cstring>
#include <cmath>


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