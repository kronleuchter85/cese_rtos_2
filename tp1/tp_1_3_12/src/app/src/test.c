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
#include "test_mock.h"

/********************** macros and definitions *******************************/

/********************** internal data declaration ****************************/

typedef struct
{
    int open;
    int close;
} counter_t_;

typedef struct
{
    bool sensor;
    counter_t_ counter;
} access_t_;

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static access_t_ car_[ACCESS__CNT];
static access_t_ emergency_[ACCESS__CNT];

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

static void access_counter_reset_(access_t_* acc)
{
  acc->counter.open = 0;
  acc->counter.close = 0;
}

static void counters_reset_(void)
{
  access_counter_reset_(car_ + 0);
  access_counter_reset_(car_ + 1);
  access_counter_reset_(emergency_ + 0);
  access_counter_reset_(emergency_ + 1);
}

static void assert_access_open_counter_(access_t_* acc, int access, int cnt)
{
  acc = acc + access;

  if(cnt != acc->counter.open)
  {
    TLOG("Error, access %d, open counter %d != %d", access, cnt, acc->counter.open);
    test_fail_();
  }
}

static void assert_access_close_counter_(access_t_* acc, int access, int cnt)
{
  acc = acc + access;

  if(cnt != acc->counter.close)
  {
    TLOG("Error, access %d, close counter %d != %d", access, cnt, acc->counter.close);
    test_fail_();
  }
}

static void assert_access_counters_(access_t_* acc, int open_0, int close_0, int open_1, int close_1)
{
  assert_access_open_counter_(acc, 0, open_0);
  assert_access_close_counter_(acc, 0, close_0);

  assert_access_open_counter_(acc, 1, open_1);
  assert_access_close_counter_(acc, 1, close_1);
}

static void assert_cars_counters_(int open_0, int close_0, int open_1, int close_1)
{
  assert_access_counters_(car_, open_0, close_0, open_1, close_1);
}

static void assert_emergencies_counters_(int open_0, int close_0, int open_1, int close_1)
{
  assert_access_counters_(emergency_, open_0, close_0, open_1, close_1);
}

static void set_cars_(bool car_0, bool car_1)
{
  car_[0].sensor = car_0;
  car_[1].sensor = car_1;
}

static void set_emergencies_(bool eme_0, bool eme_1)
{
  emergency_[0].sensor = eme_0;
  emergency_[1].sensor = eme_1;
}

static void test_setup_(void)
{
  counters_reset_();

  set_cars_(false, false);
  assert_cars_counters_(0, 0, 0, 0);

  set_emergencies_(false, false);
  assert_emergencies_counters_(0, 0, 0, 0);
}

static void test_teardown_(void)
{
  set_cars_(false, false);
  set_emergencies_(false, false);

  counters_reset_();
}

static void case_without_events_(void)
{
  set_cars_(false, false);
  TDELAY(1000);
  assert_cars_counters_(0, 0, 0, 0);
}

static void case_two_car_no_simultaneously_(void)
{
  set_cars_(true, false);
  TDELAY(200);
  assert_cars_counters_(1, 0, 0, 0);
  TDELAY(2000);
  assert_cars_counters_(1, 1, 0, 0);

  set_cars_(false, true);
  TDELAY(200);
  assert_cars_counters_(1, 1, 1, 0);
  TDELAY(2000);
  assert_cars_counters_(1, 1, 1, 1);
}

static void case_two_car_simultaneously_(void)
{
  set_cars_(true, true);
  TDELAY(2*(2000 + 200));
  assert_cars_counters_(1, 1, 1, 1);
}

static void case_two_emergencies_no_simultaneously_(void)
{
  set_emergencies_(true, false);
  TDELAY(200);
  assert_emergencies_counters_(1, 0, 0, 0);
  TDELAY(2000);
  assert_emergencies_counters_(1, 1, 0, 0);

  set_emergencies_(false, true);
  TDELAY(200);
  assert_emergencies_counters_(1, 1, 1, 0);
  TDELAY(2000);
  assert_emergencies_counters_(1, 1, 1, 1);
}

static void case_two_emergencies_simultaneously_(void)
{
  set_emergencies_(true, true);
  TDELAY(2*(200 + 2000));
  assert_emergencies_counters_(1, 1, 1, 1);
}

static void case_cars_and_emergencies_(void)
{
  set_cars_(true, false);
  TDELAY(200);
  assert_cars_counters_(1, 0, 0, 0);

  set_cars_(false, true);
  TDELAY(200);
  set_emergencies_(true, false);
  TDELAY(2000);
  assert_cars_counters_(1, 1, 0, 0);

  TDELAY(200);
  assert_emergencies_counters_(1, 0, 0, 0);
  set_emergencies_(false, true);
  TDELAY(2000);
  assert_emergencies_counters_(1, 1, 1, 0);
  TDELAY(200 + 2000);
  assert_emergencies_counters_(1, 1, 1, 1);

  TDELAY(200 + 2000);
  assert_cars_counters_(1, 1, 1, 1);
}

static void task_(void* argument)
{
  test_start_();
  TRUN(case_without_events_);
  TRUN(case_two_car_no_simultaneously_);
  TRUN(case_two_car_simultaneously_);
  TRUN(case_two_emergencies_no_simultaneously_);
  TRUN(case_two_emergencies_simultaneously_);
  TRUN(case_cars_and_emergencies_);
  test_ok_();
}

static void open_(access_t access, bool emergency)
{
  taskENTER_CRITICAL();
  {
    if(emergency)
    {
      emergency_[access].counter.open++;
      TLOG("Emergency access %d open", access);
    }
    else
    {
      car_[access].counter.open++;
      TLOG("Car access %d open", access);
    }
  }
  taskEXIT_CRITICAL();
}

static void close_(access_t access, bool emergency)
{
  taskENTER_CRITICAL();
  {
    if(emergency)
    {
      emergency_[access].counter.close++;
      TLOG("Emergency access %d close", access);
    }
    else
    {
      car_[access].counter.close++;
      TLOG("Car access %d close", access);
    }
  }
  taskEXIT_CRITICAL();
}

/********************** external functions definition ************************/

void test_init(void)
{
  counters_reset_();
  set_cars_(false, false);
  set_emergencies_(false, false);

  testharness_init_();
}

bool car_sensor_read(access_t access)
{
  bool state = car_[access].sensor;
  car_[access].sensor = false;

  if(state)
  {
    TLOG("Car sensor %d read: %d", access, state);
  }

  return state;
}

void access_open(access_t access)
{
  open_(access, false);
}

void access_close(access_t access)
{
  close_(access, false);
}

bool emergency_button_read(access_t access)
{
  bool state = emergency_[access].sensor;
  emergency_[access].sensor = false;

  if(state)
  {
    TLOG("Emergency button %d read: %d", access, state);
  }

  return state;
}

void emergency_access_open(access_t access)
{
  open_(access, true);
}

void emergency_access_close(access_t access)
{
  close_(access, true);
}

/********************** end of file ******************************************/
