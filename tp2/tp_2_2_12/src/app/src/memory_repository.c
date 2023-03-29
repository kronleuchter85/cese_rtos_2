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

static uint8_t memory_pool_memory_[MEMORY_POOL_SIZE(10, MALLOC_MAX_LEN_)];
QMPool memory_pool_;
QMPool *const hmp = &memory_pool_;

void memory_repository_initialize() {
	QMPool_init(hmp, memory_pool_memory_, MEMORY_POOL_SIZE(10, MALLOC_MAX_LEN_), MALLOC_MAX_LEN_);
}

uint8_t* memory_repository_allocate() {
	return (uint8_t*) QMPool_get(hmp, 0);
}

void memory_repository_release(uint8_t *pmsg) {
	QMPool_put(hmp, (void*) pmsg);
}
