/*
 * memory_repository.c
 *
 *  Created on: 29 Mar 2023
 *      Author: gonzalo
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "qmpool.h"
#include "memory_repository.h"

#include "client.h"

#define A 25
#define B sizeof(client_t)

static uint8_t memory_pool_memory_[MEMORY_POOL_SIZE(A, B)];
QMPool memory_pool_;
QMPool *const hmp = &memory_pool_;

void memory_repository_initialize() {
	QMPool_init(hmp, memory_pool_memory_, MEMORY_POOL_SIZE(A, B), B);
}

void* memory_repository_allocate() {
	return QMPool_get(hmp, 0);
}

void memory_repository_release(void *obj) {
	QMPool_put(hmp, (void*) obj);
}
