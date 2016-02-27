/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "AM_demod.h"


void AMDemod::update(void)
{
  if (!pass) return;

  audio_block_t *blocki, *blockq;
  int16_t *pi, *pq, *end;
  int32_t sum;
	
  blocki = receiveWritable(0); // receives I, end result is written in channel 0
  blockq = receiveReadOnly(1); // receives Q
  
  if (!blocki) {
    if (blockq) release(blockq);
    return;
  }
  
  if (!blockq) {
    release(blocki);
    return;
  }
  
  pi = (int16_t *)(blocki->data);
  pq = (int16_t *)(blockq->data);
  end = pi + AUDIO_BLOCK_SAMPLES;

  while (pi < end) {
    // square and sum I and Q
    sum = *pi * *pi;
    sum += *pq * *pq;
    // The result of squaring a 1.15 is 2.30.
    // Shift the sum up one bit to make it 1.31 (Q31)
    // and then that becomes the input to the arm sqrt function
    sum <<= 1;
    arm_sqrt_q31((q31_t)sum,(q31_t *) &sum);
    // The result is in the high order 16 bits of sum
    *pi++ = sum >> 16;
    pq++;    
  }
  transmit(blocki);
  release(blocki);
  release(blockq);
}

