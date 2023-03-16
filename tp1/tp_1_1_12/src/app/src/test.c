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

#include "hal.h"
#include "driver.h"
#include "testharness.h"
#include "test_mock.h"

/********************** macros and definitions *******************************/

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static uint8_t last_command_;
static uint8_t button_value_;

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

static void assert_command_(void) {
	if ((button_value_ & 0x07) != last_command_) {
		TLOG("Error, last command %x != %x", button_value_, last_command_);
		test_fail_();
	}
}

static void test_setup_(void) {
	button_value_ = 0x00;
}

static void test_teardown_(void) {
	return;
}

static void case_no_buttons_(void) {
	button_value_ = 0X00;
	TDELAY(200);
	assert_command_();
}

static void case_one_button_(void) {
	button_value_ = 0X01;
	TDELAY(200);
	assert_command_();
}

static void case_two_buttons_(void) {
	button_value_ = 0X03;
	TDELAY(200);
	assert_command_();
}

static void case_all_buttons_(void) {
	button_value_ = 0X07;
	TDELAY(200);
	assert_command_();
}

static void task_(void *argument) {
	test_start_();
	TRUN(case_no_buttons_);
	TRUN(case_one_button_);
	TRUN(case_two_buttons_);
	TRUN(case_all_buttons_);
	test_ok_();
}

/********************** external functions definition ************************/

void test_init(void) {
	button_value_ = 0x00;
	last_command_ = 0xFF;

	testharness_init_();
}

// mocks

bool button_read(button_t button) {
	bool state = (0 != (button_value_ & (0x01 << button))) ? true : false;
//	TLOG("Button %d read: %d", button, state);
	return state;
}

void led_command_send(uint8_t command) {
	last_command_ = command;
//	TLOG("Led command: %X", command);
}

/********************** end of file ******************************************/
