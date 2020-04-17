#pragma once

#include <datatypes/bitmap.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define B_TOTAL_EXP 14U
#define B_BLOCK_EXP 6U
#define B_LOGS_COUNT 16U

extern void allocator_init(void);
extern void allocator_fini(void);

extern void allocator_start_processing(void);
extern void allocator_done_processing(void);
extern void allocator_rollback(unsigned steps);

extern void* __wrap_malloc(size_t req_size);
extern void *__wrap_calloc(size_t nmemb, size_t size);
extern void __wrap_free(void *ptr);


// the instrumentor has to place this call before each write to memory
void write_mem(void *ptr);

/// the instrumentor calls this function to know if the address is clean
bool is_clean(void *ptr);
