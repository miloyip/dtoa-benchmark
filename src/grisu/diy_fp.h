/*
  Copyright (c) 2009 Florian Loitsch

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated documentation
  files (the "Software"), to deal in the Software without
  restriction, including without limitation the rights to use,
  copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following
  conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.
  */
#pragma once
#include <stdint.h>
#include <assert.h>

typedef struct diy_fp_t {
    uint64_t f;
    int e;
} diy_fp_t;

static diy_fp_t minus(diy_fp_t x, diy_fp_t y) {
  assert(x.e == y.e);
  assert(x.f >= y.f);
  diy_fp_t r = {.f = x.f - y.f, .e = x.e};
  return r;
}

/*
static diy_fp_t minus(diy_fp_t x, diy_fp_t y) {
  assert(x.e == y.e);
  assert(x.f >= y.f);
  diy_fp_t r = {.f = x.f - y.f, .e = x.e};
  return r;
}
*/

static diy_fp_t multiply(diy_fp_t x, diy_fp_t y) {
    uint64_t a,b,c,d,ac,bc,ad,bd,tmp,h;
  diy_fp_t r; uint64_t M32 = 0xFFFFFFFF;
  a = x.f >> 32; b = x.f & M32;
  c = y.f >> 32; d = y.f & M32;
  ac = a*c; bc = b*c; ad = a*d; bd = b*d;
  tmp = (bd>>32) + (ad&M32) + (bc&M32);
  tmp += 1U << 31; /// mult_round
  r.f = ac+(ad>>32)+(bc>>32)+(tmp >>32);
  r.e = x.e + y.e + 64;
  return r;
}
