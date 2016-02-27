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

#include "freq_conv.h"
#include "arm_math.h"

void AudioEffectFreqConv::update(void)
{
  audio_block_t *blockI, *blockQ;
  audio_block_t *blockA, *blockB, *blockC, *blockD;

//  blockI = receiveReadOnly(0);
//  blockQ = receiveReadOnly(1);
  blockI = receiveWritable(0);
  blockQ = receiveWritable(1);

  if (!blockI) {
    if (blockQ) release(blockQ);
    return;
  }
  if (!blockQ) {
    release(blockI);
    return;
  }

  if (!pass)
  {
    transmit(blockI, 0);
    transmit(blockQ, 1);
    release(blockI);
    release(blockQ);
    return;
  }

  blockA = allocate();
  blockB = allocate();
  blockC = allocate();
  blockD = allocate();


  if (blockA && blockB && blockC && blockD)
  {

    if (!dir) 
    {
      // A = I * sinQ
      arm_mult_q15((q15_t *)blockI->data, ( q15_t *) Osc_Q_buffer_i, (q15_t *)blockA->data, AUDIO_BLOCK_SAMPLES);
      // B = Q * sinI
      arm_mult_q15((q15_t *)blockQ->data, ( q15_t *) Osc_I_buffer_i  , (q15_t *)blockB->data, AUDIO_BLOCK_SAMPLES);
      // C = Q * sinQ
      arm_mult_q15((q15_t *)blockQ->data, ( q15_t *) Osc_Q_buffer_i, (q15_t *)blockC->data, AUDIO_BLOCK_SAMPLES);
      // D = I * sinI
      arm_mult_q15((q15_t *)blockI->data, ( q15_t *) Osc_I_buffer_i  , (q15_t *)blockD->data, AUDIO_BLOCK_SAMPLES);
      // I = A + B
//      arm_add_q15((q15_t *)blockA->data, (q15_t *)blockB->data, (q15_t *)blockA->data, AUDIO_BLOCK_SAMPLES); // summation for I channel
      arm_add_q15((q15_t *)blockA->data, (q15_t *)blockB->data, (q15_t *)blockI->data, AUDIO_BLOCK_SAMPLES); // summation for I channel
      release(blockB); // 
      // Q = C - D
//      arm_sub_q15((q15_t *)blockC->data, (q15_t *)blockD->data, (q15_t *)blockC->data, AUDIO_BLOCK_SAMPLES); // difference for Q channel
      arm_sub_q15((q15_t *)blockC->data, (q15_t *)blockD->data, (q15_t *)blockQ->data, AUDIO_BLOCK_SAMPLES); // difference for Q channel
      release(blockD);
    }
    else
    {
      // A = Q * sinQ
      arm_mult_q15((q15_t *)blockQ->data, ( q15_t *) Osc_Q_buffer_i, (q15_t *)blockA->data, AUDIO_BLOCK_SAMPLES);
      // B = I * sinI
      arm_mult_q15((q15_t *)blockI->data, ( q15_t *) Osc_I_buffer_i  , (q15_t *)blockB->data, AUDIO_BLOCK_SAMPLES);
      // C = I * sinQ
      arm_mult_q15((q15_t *)blockI->data, ( q15_t *) Osc_Q_buffer_i, (q15_t *)blockC->data, AUDIO_BLOCK_SAMPLES);
      // D = Q * sinI
      arm_mult_q15((q15_t *)blockQ->data, ( q15_t *) Osc_I_buffer_i  , (q15_t *)blockD->data, AUDIO_BLOCK_SAMPLES);
      // Q = A + B
//      arm_add_q15((q15_t *)blockA->data, (q15_t *)blockB->data, (q15_t *)blockC->data, AUDIO_BLOCK_SAMPLES); // summation for I channel
      arm_add_q15((q15_t *)blockA->data, (q15_t *)blockB->data, (q15_t *)blockQ->data, AUDIO_BLOCK_SAMPLES); // summation for Q channel !!!
      release(blockB);
      // I = C - D  
//      arm_sub_q15((q15_t *)blockC->data, (q15_t *)blockD->data, (q15_t *)blockA->data, AUDIO_BLOCK_SAMPLES); // difference for Q channel
      arm_sub_q15((q15_t *)blockC->data, (q15_t *)blockD->data, (q15_t *)blockI->data, AUDIO_BLOCK_SAMPLES); // difference for I channel !!!
      release(blockD);
    }
//    transmit(blockA, 0);
//    transmit(blockC, 1);
    transmit(blockI, 0);
    transmit(blockQ, 1);
    release(blockA);
    release(blockC);
  }
  release(blockI);
  release(blockQ);


}

