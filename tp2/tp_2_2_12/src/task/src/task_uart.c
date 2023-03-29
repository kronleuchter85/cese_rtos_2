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
 * @file   : task_uart.c
 * @date   : Mar 14, 2023
 * @author : NOMBRE <MAIL>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "hal.h"
#include "driver.h"
#include "serial.h"

#include "memory_repository.h"

/********************** macros and definitions *******************************/

#define TASK_PERIOD_MS_         (1000)
/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

extern QueueHandle_t hqueue;

/********************** internal functions definition ************************/

/********************** external functions definition ************************/

void task_uart(void *args)
{
	uint8_t *pmsg;

	while (true)
	{

		//
		// obtenemos un bloque de memoria del pool
		//
		pmsg = (uint8_t*) memory_repository_allocate();

		if (NULL != pmsg)
				{
			size_t len = serial_uart_read(pmsg, MALLOC_MAX_LEN_);
			if (0 < len)
					{
				serial_message_t serial_message;
				serial_message.pmsg = pmsg;
				serial_message.len = len;
				if (pdPASS != xQueueSend(hqueue, (void* )&serial_message, 0))
				{
					//
					// devolvemos el bloque al pool
					//
					memory_repository_release(pmsg);
					vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
				}
			}
			else
			{
				//
				// devolvemos el bloque al pool
				//
				memory_repository_release(pmsg);

				vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
			}
		}
		else
		{
			vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
		}
	}
}

/********************** end of file ******************************************/
