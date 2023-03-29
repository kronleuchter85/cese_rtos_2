/*
 * Copyright (c) 2023 Sebastian Bedin <sebabedin@gmail.com>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @file   : test.c
 * @date   : Feb 25, 2023
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver.h"
#include "testharness.h"
#include "test.h"

/********************** macros and definitions *******************************/

#define CLIENT_LIST_LEN_        (100)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static uint8_t rx_frame_idx_;
static uint8_t tx_frame_idx_;
static uint32_t frames_send_cnt_;
static uint32_t frames_process_cnt_;

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

static void frame_send_(uint32_t nframes)
{
  frames_send_cnt_ += nframes;
}

static void frames_process_(const uint8_t* pmsg, size_t msg_size)
{
  for(int i = 0; i < msg_size; ++i)
  {
    if(rx_frame_idx_ != pmsg[i])
    {
      TLOG("Error, frame %d, data corruption %u != %u", i, (unsigned int)rx_frame_idx_, (unsigned int)pmsg[i]);
      test_fail_();
    }
    rx_frame_idx_++;
  }
  frames_process_cnt_ += msg_size;
}

static void assert_frame_(uint32_t nframes)
{
  if(0 != frames_send_cnt_)
  {
    TLOG("Error, send counter %u != %u", (unsigned int)0, (unsigned int)frames_send_cnt_);
    test_fail_();
  }

  if(nframes != frames_process_cnt_)
  {
    TLOG("Error, process counter %u != %u", (unsigned int)nframes, (unsigned int)frames_process_cnt_);
    test_fail_();
  }
}

static void test_setup_(void)
{
  rx_frame_idx_ = 0;
  tx_frame_idx_ = 0;
  frames_send_cnt_ = 0;
  frames_process_cnt_ = 0;
}

static void test_teardown_(void)
{
  return;
}

static void case_without_frame_(void)
{
  int nframes = 0;
  frame_send_(nframes);
  TDELAY(nframes * 1000 + 100);
  assert_frame_(nframes);
}

static void case_100_frame_(void)
{
  int nframes = 100;
  frame_send_(nframes);
  TDELAY(1000 + 100);
  assert_frame_(nframes);
}

static void case_200_frame_(void)
{
  int nframes = 200;
  frame_send_(nframes);
  TDELAY(1000 + 100);
  assert_frame_(nframes);
}

static void task_(void* argument)
{
  test_start_();
  TRUN(case_without_frame_);
  TRUN(case_100_frame_);
  TRUN(case_200_frame_);
  test_ok_();
}

/********************** external functions definition ************************/

void test_init(void)
{
  testharness_init_();
}

size_t serial_uart_read(uint8_t* pmsg, size_t max_size)
{
  if(frames_send_cnt_ < max_size)
  {
    max_size = frames_send_cnt_;
  }

  for(int i = 0; i < max_size; ++i)
  {
    pmsg[i] = tx_frame_idx_;
    tx_frame_idx_++;
  }

  frames_send_cnt_ -= max_size;
  return max_size;
}

void process(const uint8_t* pmsg, size_t msg_size)
{
  frames_process_(pmsg, msg_size);
}

/********************** end of file ******************************************/
