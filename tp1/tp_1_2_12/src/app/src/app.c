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

#define QUEUE_LENGTH_           (1)
#define QUEUE_ITEM_SIZE_        (sizeof(char))
#define MAX_THREADS_            (5)
#define TASK_PERIOD_MS_         (50)

/********************** macros and definitions *******************************/

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

/************************ Tasks ************************/

SemaphoreHandle_t MutexHandle;

uint16_t last_car_id = 0;

uint16_t get_new_car_id() {
	uint16_t new_car_id = last_car_id;
	last_car_id++;
	return new_car_id;
}

//
// implementacion del comportamiento de acceso de autos al tunel
//
static void callback_(car new_c) {
	if (new_c.access == ACCESS_EAST) {

		//
		// abrimos la barrera este para q el auto entre
		//
		access_open(ACCESS_EAST);
		ELOG("El auto entra al tunel: [%d]", new_c.id);
		access_close(ACCESS_EAST);

		traffic_system.cars_passing_by++;
		ELOG("Cantidad de autos pasando: %d", traffic_system.cars_passing_by);

		//
		// abrimos la barrera oeste para que el auto salga
		//
		access_open(ACCESS_WEST);
		ELOG("El auto libera el tunel: [%d]", new_c.id);
		access_close(ACCESS_WEST);

	} else if (new_c.access == ACCESS_WEST) {

		//
		// abrimos la barrera oeste para q el auto entre
		//
		access_open(ACCESS_WEST);
		ELOG("El auto entra al tunel: [%d]", new_c.id);
		access_close(ACCESS_WEST);

		traffic_system.cars_passing_by++;
		ELOG("Cantidad de autos pasando: %d", traffic_system.cars_passing_by);

		///
		// abrimos la barrera este para q el auto salga
		//
		access_open(ACCESS_EAST);
		ELOG("El auto libera el tunel: [%d]", new_c.id);
		access_close(ACCESS_EAST);
	}

	traffic_system.cars_passing_by--;
	ELOG("Cantidad de autos en el tunel: %d", traffic_system.cars_passing_by);

	return;
}

static void tunnel_subscriber_task(void *argument) {
	car car;
	while (true) {
		if (pdPASS == xQueueReceive(traffic_system.both_sides_queue, &car, 0)) {

			//
			// bloqueamos la zona critica: el acceso al tunel
			//
			xSemaphoreTake(MutexHandle, portMAX_DELAY);

			traffic_system.callback(car);
			vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));

			xSemaphoreGive(MutexHandle);
		}
	}
}

bool ao_tunnel_queue_new_car_arrived(access_t access) {

	//
	// un nuevo auto llega a alguna de las entradas del tunel y se encola
	//

	car new_car;
	new_car.id = get_new_car_id();
	new_car.access = access;

	if (pdPASS == xQueueSend(traffic_system.both_sides_queue, (void* )&new_car, 0)) {

		return true;
	}

	return false;
}

void task_tunnel_entry_control_producer_task(void *argument) {

	while (true) {

		//
		// se determina si hay autos nuevos en ambas entradas y se agregan a la cola
		//
		bool car_east_side = car_sensor_read(ACCESS_EAST);
		bool car_west_side = car_sensor_read(ACCESS_WEST);

		if (car_east_side) {
			ELOG("Llego un auto al extremo EAST");
			ao_tunnel_queue_new_car_arrived(ACCESS_EAST);
		}
		if (car_west_side) {
			ELOG("Llego un auto al extremo WEST");
			ao_tunnel_queue_new_car_arrived(ACCESS_WEST);
		}
		vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
	}
}

static bool create_subscriber_task(void) {

	if (traffic_system.task_cnt < MAX_THREADS_) {
		ELOG("Creo una tarea ");
		traffic_system.task_cnt++;
		ELOG("Cantidad de tareas: %d", traffic_system.task_cnt);
		BaseType_t status;
		status = xTaskCreate(tunnel_subscriber_task, "tunnel_subscriber_task", 128, NULL, (tskIDLE_PRIORITY + 10), NULL);
		while (pdPASS != status) {
			ELOG("Error!!!");
			// error
		}
		return true;
	} else {
		ELOG("No puedo crear nuevas tareas");
		return false;
	}
}

void ao_tunnel_queue_init(void) {
	traffic_system.cars_passing_by = 0;
	traffic_system.task_cnt = 0;
	traffic_system.callback = callback_;

	traffic_system.both_sides_queue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);

	//
	// si bien el enunciado indica que hay dos entradas y los autos pueden entrar por ambas,
	// tambien indica que se debe respetar el orden temporal en el q arriban a cualquiera de ellas
	// por lo cual tiene mas sentido modelizar una sola cola FIFO (en lugar de dos colas) y colocar todos los autos
	// incluyendo la informacion de a que entrada pertenecen
	//
	while (NULL == traffic_system.both_sides_queue) {
		// error
	}

	if (0 == traffic_system.task_cnt) {
		create_subscriber_task();
	}
}

/********************** external functions definition ************************/

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
	// Inicializo el estado del objeto activo, su tarea (subscriber) y la cola
	{
		ELOG("ao init");
		ao_tunnel_queue_init();
	}

	// tasks
	//
	// Inicializo la tarea productora: la que determina si hay nuevos autos para agregar a la cola
	{
		ELOG("tasks init");

		BaseType_t status;
		status = xTaskCreate(task_tunnel_entry_control_producer_task, "task_uart", 128, NULL, (tskIDLE_PRIORITY + 10), NULL);
		while (pdPASS != status) {
			// error
		}
		ELOG("tasks init");

	}

	ELOG("app init");
}

/********************** end of file ******************************************/
