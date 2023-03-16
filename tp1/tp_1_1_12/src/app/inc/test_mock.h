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
 * @file   : test_mock.h
 * @date   : Mar 5, 2023
 * @author : Sebastian Bedin <sebabedin@gmail.com>
 * @version	v1.0.0
 */

#ifndef APP_INC_TEST_MOCK_H_
#define APP_INC_TEST_MOCK_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

	/********************** inclusions *******************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "app.h"

	/********************** macros ***********************************************/

	/********************** typedef **********************************************/

	/********************** external data declaration ****************************/

	/********************** external functions declaration ***********************/

	/**
	 * @brief Retorna el estado del botón consultado.
	 * @param button es el número de botón definido en button_t
	 * @return true si el botón está presionado, false sino
	 */
	bool button_read(button_t button);

	/**
	 * @brief Envia el comando de control a los LEDs.
	 * @param command es el byte de control a enviar.
	 *      Los bits 0, 1 y 2 se corresponden con los colores Rojo, Verde y Azul.
	 *      El resto de los bits deben ser ceros.
	 *      Un bit en 1 enciende un LED, un bit en 0 lo apaga.
	 *
	 * Ejemplo: Para encender el LED Rojo, se debe enviar 0x01
	 */
	void led_command_send(uint8_t command);

	/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* APP_INC_TEST_MOCK_H_ */
	/********************** end of file ******************************************/

