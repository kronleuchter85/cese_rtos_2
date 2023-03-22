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

/********************** macros and definitions *******************************/

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/

/********************** external data definition *****************************/

/********************** internal functions definition ************************/

/********************** external functions definition ************************/

#define QUEUE_LENGTH_           (20)
#define QUEUE_ITEM_SIZE_        (sizeof(char))
#define MAX_THREADS_            (5)
#define TASK_PERIOD_MS_         (2*1000)

static traffic_system_t traffic_system;

uint16_t last_car_id = 0;

uint16_t get_new_car_id() {
	uint16_t new_car_id = last_car_id;
	last_car_id++;
	return new_car_id;
}

//
// ************************************************************ SUBSCRIBER *******************************************************************************
//

//
// implementacion de comportamiento de acceso
// para vehiculos de emergencia
//
static void emergency_vehicle_callback_impl(car new_c) {

	//
	// abrimos la barrera este para q el auto entre
	//
	emergency_access_open(new_c.access);
	ELOG("El auto entra al tunel: [%d]", new_c.id);
	emergency_access_close(new_c.access);

	traffic_system.cars_passing_by++;
	ELOG("Cantidad de autos pasando: %d", traffic_system.cars_passing_by);

	//
	// abrimos la barrera oeste para que el auto salga
	//
	ELOG("El auto libera el tunel: [%d]", new_c.id);

	traffic_system.cars_passing_by--;
	ELOG("Cantidad de autos en el tunel: %d", traffic_system.cars_passing_by);
}

//
// implementacion del comportamiento de acceso de autos al tunel para autos sin prioridad
//
static void regular_vehicle_callback_impl(car new_c) {

	//
	// abrimos la barrera este para q el auto entre
	//
	access_open(new_c.access);
	ELOG("El auto entra al tunel: [%d]", new_c.id);
	access_close(new_c.access);

	traffic_system.cars_passing_by++;
	ELOG("Cantidad de autos pasando: %d", traffic_system.cars_passing_by);

	//
	// abrimos la barrera oeste para que el auto salga
	//
	ELOG("El auto libera el tunel: [%d]", new_c.id);

	traffic_system.cars_passing_by--;
	ELOG("Cantidad de autos en el tunel: %d", traffic_system.cars_passing_by);

}

static void tunnel_subscriber_task(void *argument) {
	car car;
	while (true) {

		//
		// valido que el tunel este vacio..
		// tecnicamente como es el recurso del objeto activo y unico hilo, todo auto anterior ya se deberia
		// haber procesado y haber salido del tunel, entonces el tunel siempre deberia estar
		// disponible en este punto
		//
		if (traffic_system.cars_passing_by == 0) {

			//
			// si hay algun vehiculo con prioridad se lo procesa.
			//
			if (pdPASS == xQueueReceive(traffic_system.emergency_vehicles_queue, &car, 0)) {

				traffic_system.emergency_vehicle_callback(car);
			}

			//
			// SI NO hay ningun otro vehiculo con prioridad, entonces se checkea la cola
			// para vehiculos regulares. Caso contrario se continua (volviendo a checkear la cola con prioridad)
			//
			else if (pdPASS == xQueueReceive(traffic_system.both_sides_queue, &car, 0)) {

				traffic_system.regular_vehicle_callback(car);
			}

		} else {

			ELOG("Error: El tunel quedo ocupado");
		}

		vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
	}
}

//
// ************************************************************ PUBLISHER *************************************************************************************
//

bool ao_tunnel_queue_new_car_arrived(access_t access, uint16_t emergency_vehicle) {

	//
	// un nuevo auto llega a alguna de las entradas del tunel y se encola
	//

	car new_car;
	new_car.id = get_new_car_id();
	new_car.access = access;
	new_car.is_emergency_vehicle = emergency_vehicle;

	if (emergency_vehicle) {

		if (pdPASS == xQueueSend(traffic_system.emergency_vehicles_queue, (void* )&new_car, 0)) {
			return true;
		}
	} else {

		if (pdPASS == xQueueSend(traffic_system.both_sides_queue, (void* )&new_car, 0)) {
			return true;
		}
	}

	return false;
}

void task_tunnel_entry_control_regular_vehicles_producer_task(void *argument) {

	while (true) {

		//
		// se determina si hay autos nuevos en ambas entradas y se agregan a la cola
		//
		bool car_east_side = car_sensor_read(ACCESS_EAST);
		bool car_west_side = car_sensor_read(ACCESS_WEST);

		if (car_east_side) {
			ELOG("Llego un auto al extremo EAST");
			ao_tunnel_queue_new_car_arrived(ACCESS_EAST, 0);
		}
		if (car_west_side) {
			ELOG("Llego un auto al extremo WEST");
			ao_tunnel_queue_new_car_arrived(ACCESS_WEST, 0);
		}
		// vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
	}
}

void task_tunnel_entry_control_emergency_vehicles_producer_task(void *argument) {

	while (true) {

		//
		// se determina si hay autos nuevos en ambas entradas y se agregan a la cola
		//
		bool car_east_side = emergency_button_read(ACCESS_EAST);
		bool car_west_side = emergency_button_read(ACCESS_WEST);

		if (car_east_side) {
			ELOG("Llego un veniculo de emergencia al extremo EAST");
			ao_tunnel_queue_new_car_arrived(ACCESS_EAST, 1);
		}
		if (car_west_side) {
			ELOG("Llego un veniculo de emergencia al extremo WEST");
			ao_tunnel_queue_new_car_arrived(ACCESS_WEST, 1);
		}
		// vTaskDelay((TickType_t) (TASK_PERIOD_MS_ / portTICK_PERIOD_MS));
	}
}

//
// ************************************************************ inicializacion *************************************************************************************
//

static bool create_subscriber_task(void) {

	if (traffic_system.task_cnt < MAX_THREADS_) {
		ELOG("Creo una tarea ");
		traffic_system.task_cnt++;
		ELOG("Cantidad de tareas: %d", traffic_system.task_cnt);
		BaseType_t status;
		status = xTaskCreate(tunnel_subscriber_task,
				"tunnel_subscriber_task", 128, NULL, (tskIDLE_PRIORITY + 10), NULL);
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

void traffic_control_system_init(void) {
	traffic_system.cars_passing_by = 0;
	traffic_system.task_cnt = 0;

	//
	// inicializamos los callbacks
	//
	traffic_system.regular_vehicle_callback = regular_vehicle_callback_impl;
	traffic_system.emergency_vehicle_callback = emergency_vehicle_callback_impl;

	//
	// inicializamos las colas
	//
	traffic_system.both_sides_queue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);
	traffic_system.emergency_vehicles_queue = xQueueCreate(QUEUE_LENGTH_, QUEUE_ITEM_SIZE_);

	//
	// si bien el enunciado indica que hay dos entradas y los autos pueden entrar por ambas,
	// tambien indica que se debe respetar el orden temporal en el q arriban a cualquiera de ellas
	// por lo cual tiene mas sentido modelizar una sola cola FIFO (en lugar de dos colas) y colocar todos los autos
	// incluyendo la informacion de a que entrada pertenecen
	//
	if (NULL == traffic_system.both_sides_queue || NULL == traffic_system.emergency_vehicles_queue) {
		ELOG("Error inicializando las colas!!!");
		// error
	}

	if (0 == traffic_system.task_cnt) {
		create_subscriber_task();
	}
}

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

	// OA
	//
	// Inicializo el estado del objeto activo, su tarea (subscriber) y la cola
	{
		ELOG("ao init");
		traffic_control_system_init();
	}

	// tasks
	//
	// Inicializo la tarea productora: la que determina si hay nuevos autos para agregar a la cola
	{
		ELOG("tasks init");

		BaseType_t status;
		status = xTaskCreate(task_tunnel_entry_control_regular_vehicles_producer_task,
				"task_tunnel_entry_control_regular_vehicles_producer_task", 128, NULL,
				(tskIDLE_PRIORITY + 10), NULL);
		while (pdPASS != status) {
			// error
		}
		ELOG("tasks init");
//		status = xTaskCreate(task_tunnel_entry_control_emergency_vehicles_producer_task,
//				"task_tunnel_entry_control_emergency_vehicles_producer_task", 128,
//				NULL, (tskIDLE_PRIORITY + 10), NULL);
//		while (pdPASS != status) {
//			// error
//		}
//		ELOG("tasks init");

	}

	ELOG("app init");
}

/********************** end of file ******************************************/
