/*
 * memory_repository.h
 *
 *  Created on: 29 Mar 2023
 *      Author: gonzalo
 */

#ifndef APP_INC_MEMORY_REPOSITORY_H_
#define APP_INC_MEMORY_REPOSITORY_H_

#define MALLOC_MAX_LEN_         (64)
#define MEMORY_POOL_SIZE(nblocks, block_size)    ((nblocks)*(block_size))

void memory_repository_initialize();

void* memory_repository_allocate();

void memory_repository_release(void *obj);

#endif /* APP_INC_MEMORY_REPOSITORY_H_ */
