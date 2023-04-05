/*
 * Copyright (c) YEAR NOMBRE <MAIL>.
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
 * @file   : task_bank.c
 * @date   : Mar 21, 2023
 * @author : NOMBRE <MAIL>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver.h"
#include "bank.h"
#include "memory_repository.h"

/********************** macros and definitions *******************************/
#define MAX_THREADS_            (2)
#define QUEUE_LENGTH_           (25)
#define QUEUE_ITEM_SIZE_        (sizeof(client_t))

/********************** internal data declaration ****************************/

static struct
{
	QueueHandle_t hclient_queue;
	size_t task_cnt;
	size_t client_cnt;

} bank_;

client_callback_t onClientArrivalHandler;
client_callback_t onClientAttentionHandler;

static void client_callback_(client_t *pnew_client);
static void task_delete_(void);
static void task_(void *args);
static bool task_create_(void);

static void task_delete_(void)
{
	ELOG("Borro una tarea");
	bank_.task_cnt--;
	ELOG("Cantidad de tareas: %d", bank_.task_cnt);

	vTaskDelete(NULL);
}

static void task_(void *args)
{

	client_set_callback(client_callback_);

	while (true)
	{

		client_t *client = (client_t*) memory_repository_allocate();

		if (pdPASS == xQueueReceive(bank_.hclient_queue, client, portMAX_DELAY)) {

			bank_.client_cnt++;
			onClientAttentionHandler(client);

			memory_repository_release(client);
			bank_.client_cnt--;
		}

		else
		{
			task_delete_();
		}

	}
}

static bool task_create_(void)
{

	if (bank_.task_cnt < MAX_THREADS_) {

		ELOG("Creo una tarea");
		bank_.task_cnt++;
		ELOG("Cantidad de tareas: %d", bank_.task_cnt);
		BaseType_t status;
		status = xTaskCreate(task_, "task_bank", 128, NULL, tskIDLE_PRIORITY + 1, NULL);
		while (pdPASS != status)
		{
			ELOG("Error!!!");
			// error
		}
		return true;
	}
	else
	{
		ELOG("No puedo crear nuevas tareas");
		return false;
	}
}

static void client_callback_(client_t *pnew_client)
{
	client_t *client = (client_t*) memory_repository_allocate();
	client->id = pnew_client->id;
	client->age = pnew_client->age;

	if (pdPASS == xQueueSend(bank_.hclient_queue, (void* )client, 0))
	{
		onClientArrivalHandler(client);
		if (0 == bank_.task_cnt) {
			task_create_();
		}
		memory_repository_release(client);
	}

	//
	// si cuando intenta poner el cliente en la cola no puede por falta de capacidad
	//
	else if (task_create_())
	{
		if (pdPASS == xQueueSend(bank_.hclient_queue, (void* )client, 0))
		{
			onClientArrivalHandler(client);

			memory_repository_release(client);
		}
	}

	else
	{
		ELOG("Error, el cliente no tiene lugar en la fila");
	}

}

/********************** external functions definition ************************/

void bank_init(client_callback_t onArrivalHandler, client_callback_t onAttentionHandler)
{

	onClientArrivalHandler = onArrivalHandler;
	onClientAttentionHandler = onAttentionHandler;
	bank_.client_cnt = 0;
	bank_.task_cnt = 0;
	bank_.hclient_queue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
	while (NULL == bank_.hclient_queue)
	{
		// error
	}

	BaseType_t status;
	status = xTaskCreate(task_, "task_bank", 128, NULL, tskIDLE_PRIORITY, NULL);
	while (pdPASS != status)
	{
		// error
	}
}

/********************** end of file ******************************************/
