#ifndef FIXED_POINT_H
#define FIXED_POINT_H

#include <inttypes.h> 
#define _FXP_F (1<<14)

typedef int32_t fxp_t;

fxp_t fixed_conv_fxp(int n) {
  return n * _FPX_F;
}

int fixed_conv_int(fxp_t x) {
  return x / _FPX_F;
}

int fixed_conv_int_near(fxp_t x) {
  if(x<0) x -= _FPX_F/2;
  else x += _FPX_F/2;
  return x / _FPX_F;
}

fxp_t fixed_add(fxp_t x, fxp_t y) {
  return x+y;
}

fxp_t fixed_sub(fxp_t x, fxp_t y) {
  return x-y;
}

fxp_t fixed_add_n(fxp_t x, int n) {
  return x + n * _FPX_F;
}

fxp_t fixed_sub_n(fxp_t x, int n) {
  return x - n * _FPX_F;
}

fxp_t fixed_mul(fxp_t x, fxp_t, y) {
  return ((int64_t)x) * y / _FPX_F;
}

fxp_t fixed_div(fxp_t x, fxp_t, y) {
  return ((int64_t)x) * _FPX_F / y;
}

fxp_t fixed_mul_n(fxp_t x, int n) {
  return x * n;
}

fxp_t fixed_div_n(fxp_t x, int n) {
  return x / n;
}

#endif