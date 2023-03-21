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
 * @file   : app.h
 * @date   : Feb 17, 2023
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 * @version	v1.0.0
 */

#ifndef APP_INC_APP_H_
#define APP_INC_APP_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "test.h"

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/

/********************** external functions declaration ***********************/

typedef struct {

	//
	// ID del vehiculo
	//
	uint16_t id;

	//
	// por donde es el acceso: ESTE u OESTE
	//
	access_t access;

	//
	// 1 si es un vehiculo de emergencia
	// 0 (default) si es un vehiculo normal
	//
	uint16_t is_emergency_vehicle;
} car;

typedef void (*ao_tunnel_queue_car_callback_t)(car);

typedef struct {

	//
	// cola para autos sin prioridad
	//
	QueueHandle_t both_sides_queue;

	//
	// cola para vehiculos con prioridad
	//
	QueueHandle_t emergency_vehicles_queue;

	//
	// callback definition para vehiculos con prioridad normal
	//
	ao_tunnel_queue_car_callback_t regular_vehicle_callback;

	//
	// callbacks definition para vehiculos prioritarios
	//
	ao_tunnel_queue_car_callback_t emergency_vehicle_callback;

	size_t task_cnt;

	//
	// cantidad de autos pasando por el tunel
	//
	size_t cars_passing_by;

} traffic_system_t;

void app_init(void);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* APP_INC_APP_H_ */
/********************** end of file ******************************************/

