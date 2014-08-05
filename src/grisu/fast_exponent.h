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

static void fill_exponent(int K, char* buffer) {
    int i = 0;
    if (K < 0) {
	buffer[i++] = '-';
	K = -K;
    }
    if (K >= 100) {
	buffer[i++] = '0' + K / 100; K %= 100;
	buffer[i++] = '0' + K / 10; K %= 10;
	buffer[i++] = '0' + K;
    } else if (K >= 10) {
	buffer[i++] = '0' + K / 10; K %= 10;
	buffer[i++] = '0' + K;
    } else {
	buffer[i++] = '0' + K;
    }
    buffer[i] = '\0';
}
