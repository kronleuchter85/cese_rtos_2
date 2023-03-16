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
 * @file   : app.c
 * @date   : Feb 17, 2023
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 * @version	v1.0.0
 */

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "driver.h"
#include "test.h"
#include "test_mock.h"
#include "app.h"

#define QUEUE_LENGTH_            (5)
#define QUEUE_ITEM_SIZE_         (sizeof(event_t_))

/********************** internal data declaration ****************************/

/********************** macros and definitions *******************************/

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

/********************** external functions definition ************************/

/************************** Tasks ***************************/

#define TASK_PERIOD_MS_         (100)

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

SemaphoreHandle_t MutexHandle;

//
// instancia del objeto activo
//
static ao_led_t ao_led_;

//
// estado de los leds
//
static bool led_state_[3];

void task_button(void *argument) {

	button_t button = (button_t) argument;

	while (true) {

		ELOG("loop, button:%d - led:%d", button, led_state_[button]);

		ELOG("read and send");

		//
		// leemos el boton 'presionado' y enviamos el evento
		//
		bool state = button_read(button);

		ao_led_write(&ao_led_, state, button);

		vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
	}
}

/********************** OA operations ************************/

void led_state_callback_(ao_led_t *hao, bool state, button_t button) {

	//
	// definicion de la zona critica: el acceso al estado de los leds
	//

	xSemaphoreTake(MutexHandle, portMAX_DELAY);

	led_state_[button] = state;

	xSemaphoreGive(MutexHandle);

	ELOG("callback, button:%d - state:%d", button, state);
}

static void task_(void *argument) {
	ao_led_t *hao = (ao_led_t*) argument;
	while (true) {
		event_t_ event;
		if (pdPASS == xQueueReceive(hao->hqueue, &event, portMAX_DELAY)) {
			ELOG("ao,  button:%d - state:%d", event.button, event.state);

			//
			// cuando se recibe un evento se determina que led debe prenderse en base al boton indicado
			//
			if (event.button == BUTTON_BLUE) {
				eboard_led_blue(event.state);
			} else if (event.button == BUTTON_RED) {
				eboard_led_red(event.state);
			} else if (event.button == BUTTON_GREEN) {
				eboard_led_green(event.state);
			}

			//
			// se invoca al callback para actualizar el estado
			//
			hao->callback(hao, event.state, event.button);
		}
	}
}

bool ao_led_write(ao_led_t *hao, bool state, button_t button) {
	event_t_ event;
	event.state = state;
	event.button = button;
	return (pdPASS == xQueueSend(hao->hqueue, (void* )&event, 0));
}

void ao_led_init(ao_led_t *hao, ao_led_callbak_t callback) {

	eboard_led_red(false);
	eboard_led_green(false);
	eboard_led_blue(false);

	hao->callback = callback;

	hao->hqueue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
	while (NULL == hao->hqueue) {
		// error
	}

	BaseType_t status;
	status = xTaskCreate(task_, "task_ao_led", 128, (void* const ) hao, tskIDLE_PRIORITY, NULL);
	while (pdPASS != status) {
		// error
	}
}

/********************** End Tasks ************************/

void app_init(void) {
	// drivers
	{
		driver_init();
		ELOG("drivers init");
	}

	// test
	{
		test_init();
		ELOG("test init");
	}

	// general init
	//
	// Inicializo el mutex para cuidar el accesso al recurso compartido (el estado del objeto activo)
	//
	//
	{

		MutexHandle = xSemaphoreCreateMutex();

		configASSERT(MutexHandle != NULL);
	}

	// OA
	//
	// inicializo el estado del objeto activo, su tarea y la cola
	//
	{

		ELOG("ao init");

		led_state_[BUTTON_RED] = false;
		led_state_[BUTTON_GREEN] = false;
		led_state_[BUTTON_BLUE] = false;

		ao_led_init(&ao_led_, led_state_callback_);

	}

	// tasks
	//
	// inicializo las tareas usando el mismo componente de codigo pero pasando como parametro
	// el boton al que referencia cada una
	//
	{
		ELOG("tasks init");

		BaseType_t status;

		status = xTaskCreate(task_button, "task_button_red", 128, BUTTON_RED, tskIDLE_PRIORITY, NULL);
		while (pdPASS != status) {
			// error
		}
		status = xTaskCreate(task_button, "task_button_blue", 128, BUTTON_GREEN, tskIDLE_PRIORITY, NULL);
		while (pdPASS != status) {
			// error
		}
		status = xTaskCreate(task_button, "task_button_green", 128, BUTTON_BLUE, tskIDLE_PRIORITY, NULL);
		while (pdPASS != status) {
			// error
		}
		ELOG("tasks init");

	}

	ELOG("app init");
}

/********************** end of file ******************************************/
