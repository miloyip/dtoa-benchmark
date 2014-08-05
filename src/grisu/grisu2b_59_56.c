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
#include "diy_fp.h"
#include "k_comp.h"
#include "double.h"
#include "powers.h"
#include <stdbool.h>
#include <stdio.h>
#include "grisu2.h"
#include <inttypes.h>

#define TEN9 1000000000

void grisu_round(char* buffer, int len,
		 uint64_t delta, uint64_t rest,
		 uint64_t ten_kappa, uint64_t wp_w) {
  while (rest < wp_w &&
	 delta - rest >= ten_kappa &&
	 (rest + ten_kappa < wp_w || /// closer
	 wp_w - rest > rest+ten_kappa - wp_w))
  {
    buffer[len-1]--; rest += ten_kappa;
  }
}

void digit_gen(diy_fp_t W, diy_fp_t Mp, diy_fp_t delta,
	       char* buffer, int* len, int* K) {
  uint32_t div; int d,kappa; diy_fp_t one, wp_w;
  wp_w = minus(Mp, W);
  one.f = ((uint64_t) 1) << -Mp.e; one.e = Mp.e;
  uint32_t p1 = Mp.f >> -one.e; /// Mp_cut
  uint64_t p2 = Mp.f & (one.f - 1);
  *len = 0; kappa = 10; div = TEN9;
  while (kappa > 0) {
    d = p1 / div;
    if (d || *len) buffer[(*len)++] = '0' + d; /// Mp_inv1
    p1 %= div; kappa--;
    uint64_t tmp = (((uint64_t)p1)<<-one.e)+p2;
    if (tmp <= delta.f) { /// Mp_delta
      *K += kappa;
      grisu_round(buffer, *len, delta.f, tmp, ((uint64_t)div) << -one.e, wp_w.f);
      return;
    }
    div /= 10;
  }
  uint64_t unit = 1;
  while (1) {
    p2 *= 10; delta.f *= 10; unit *= 10;
    d = p2 >> -one.e;
    if (d || *len) buffer[(*len)++] = '0' + d;
    p2 &= one.f - 1; kappa--;
    if (p2 < delta.f) {
      *K += kappa;
      grisu_round(buffer, *len, delta.f, p2, one.f, wp_w.f*unit);
      return;
    }
  }
}

void grisu2(double v, char* buffer, int* length, int* K) {
  diy_fp_t w_m, w_p;
  int q = 64, alpha = -59, gamma = -56; int pos;
  normalized_boundaries(v, &w_m, &w_p);
  diy_fp_t w = normalize_diy_fp(double2diy_fp(v));
  int mk = k_comp(w_p.e + q, alpha, gamma);
  diy_fp_t c_mk = cached_power(mk);
  diy_fp_t W  = multiply(w,   c_mk);
  diy_fp_t Wp = multiply(w_p, c_mk);
  diy_fp_t Wm = multiply(w_m, c_mk);
  Wm.f++; Wp.f--;
  diy_fp_t delta = minus(Wp, Wm);
  *K = -mk;
  digit_gen(W, Wp, delta, buffer, length, K);
}
