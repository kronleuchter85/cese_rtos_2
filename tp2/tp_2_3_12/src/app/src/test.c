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
#include "client.h"
#include "testharness.h"
#include "test.h"

/********************** macros and definitions *******************************/

#define CLIENT_LIST_LEN_        (100)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

static client_callback_t callback_;
static uint32_t clients_send_cnt_;
static uint32_t clients_process_cnt_;
static uint32_t clients_list_[CLIENT_LIST_LEN_];
static client_t client_;

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

static void client_send_(uint32_t nclients, uint32_t delay)
{

	for (int i = clients_send_cnt_; i < (clients_send_cnt_ + nclients); ++i)
			{
		clients_list_[i] = 1;

		client_.id = i;
		sprintf(client_.name, "%d", i);
		sprintf(client_.addr, "%d", i);
		client_.age = i;

		callback_(&client_);
		if (0 < delay)
				{
			TDELAY(delay);
		}
	}

	clients_send_cnt_ += nclients;
}

static void client_process_(client_t *pclient)
{
	uint32_t id = pclient->id;
	if ((id < CLIENT_LIST_LEN_) && (id != pclient->age))
			{
		TLOG("Error, client %u, data corruption", (unsigned int )id);
		test_fail_();
	}

	clients_list_[id] = 0;
	clients_process_cnt_++;
}

static void assert_clientes_(uint32_t nclient)
{
	if (nclient != clients_send_cnt_)
			{
		TLOG("Error, send counter %u != %u", (unsigned int )nclient, (unsigned int )clients_send_cnt_);
		test_fail_();
	}

	if (nclient != clients_process_cnt_)
			{
		TLOG("Error, process counter %u != %u", (unsigned int )nclient, (unsigned int )clients_process_cnt_);
		test_fail_();
	}

	for (int i = 0; i < nclient; ++i)
			{
		if (0 != clients_list_[i])
				{
			TLOG("Error, client %d unprocessed", i);
			test_fail_();
		}
	}
}

static void test_setup_(void)
{
	for (int i = 0; i < CLIENT_LIST_LEN_; ++i)
			{
		clients_list_[i] = 0;
	}
	clients_send_cnt_ = 0;
	clients_process_cnt_ = 0;
}

static void test_teardown_(void)
{
	return;
}

static void case_without_clients_(void)
{
	int nclients = 0;
	client_send_(nclients, 0);
	TDELAY(nclients * 1000 + 100);
	assert_clientes_(nclients);
}

static void case_one_client_(void)
{
	int nclients = 1;
	client_send_(nclients, 10);
	TDELAY(nclients * 1000 + 100);
	assert_clientes_(nclients);
}

static void case_two_clients_(void)
{
	int nclients = 2;
	client_send_(nclients, 10);
	TDELAY(nclients * 1000 + 100);
	assert_clientes_(nclients);
}

static void case_25_clients_(void)
{
	int nclients = 25;
	client_send_(nclients, 125);
	TDELAY(20000);
	assert_clientes_(nclients);
}

static void case_crowd_clients_(void)
{
	int nclients = 50;
	client_send_(nclients, 125);
	TDELAY(20000);
	assert_clientes_(nclients);
}

static void task_(void *argument)
{
	test_start_();
	TRUN(case_without_clients_);
	TRUN(case_one_client_);
	TRUN(case_two_clients_);
	TRUN(case_25_clients_);
	TRUN(case_crowd_clients_);
	test_ok_();
}

/********************** external functions definition ************************/

void test_init(void)
{
	testharness_init_();
}

void client_set_callback(client_callback_t callback)
{
	callback_ = callback;
}

void client_process(client_t *pclient)
{
	client_process_(pclient);
	TDELAY(500);
}

/********************** end of file ******************************************/
