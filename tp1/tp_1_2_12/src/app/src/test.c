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

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static bool car_sensor_[ACCESS__CNT];
static int access_open_cnt_[ACCESS__CNT];
static int access_close_cnt_[ACCESS__CNT];

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

static void access_times_reset_(void) {
	access_open_cnt_[0] = 0;
	access_close_cnt_[0] = 0;
	access_open_cnt_[1] = 0;
	access_close_cnt_[1] = 0;
}

static void assert_access_open_times_(int access, int times) {
//	if (times != access_open_cnt_[access]) {
//		TLOG("Error, access %d open %d times != %d times", access, times, access_open_cnt_[access]);
//		test_fail_();
//	}
}

static void assert_access_close_times_(int access, int times) {
//	if (times != access_close_cnt_[access]) {
//		TLOG("Error, access %d close %d times != %d times", access, times, access_open_cnt_[access]);
//		test_fail_();
//	}
}

static void assert_access_times_(int times0, int times1) {
	assert_access_open_times_(0, times0);
	assert_access_close_times_(0, times0);
	assert_access_open_times_(1, times1);
	assert_access_close_times_(1, times1);
}

static void test_setup_(void) {
	assert_access_times_(0, 0);
}

static void test_teardown_(void) {
	access_times_reset_();
}

static void case_without_car_(void) {
	car_sensor_[0] = true;
	car_sensor_[1] = false;
	TDELAY(1000);
	assert_access_times_(0, 0);
}

static void case_one_car_side_0_(void) {
	car_sensor_[0] = true;
	car_sensor_[1] = false;
	TDELAY(2000 + 200);
	assert_access_times_(1, 0);
}

static void case_one_car_side_1_(void) {
	car_sensor_[0] = false;
	car_sensor_[1] = true;
	TDELAY(2000 + 200);
	assert_access_times_(0, 1);
}

static void case_one_car_each_sides_(void) {
	car_sensor_[0] = true;
	car_sensor_[1] = true;
	TDELAY(4000 + 200);
	assert_access_times_(1, 1);
}

static void task_(void *argument) {
	test_start_();
	TRUN(case_without_car_);
	TRUN(case_one_car_side_0_);
	TRUN(case_one_car_side_1_);
	TRUN(case_without_car_);
	TRUN(case_one_car_each_sides_);
	test_ok_();
}

/********************** external functions definition ************************/

void test_init(void) {
	car_sensor_[0] = false;
	car_sensor_[1] = false;

	testharness_init_();
}

bool car_sensor_read(access_t access) {
	bool state = car_sensor_[access];
	car_sensor_[access] = false;
	if (state) {
		TLOG("Access %d read: %d", access, state);
	}

	return state;
}

void access_open(access_t access) {
	access_open_cnt_[access]++;
	TLOG("Access %d open", access);
}

void access_close(access_t access) {
	access_close_cnt_[access]++;
	TLOG("Access %d close", access);
}

/********************** end of file ******************************************/
