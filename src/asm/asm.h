#pragma once

#include <stdint.h>
#include <stdbool.h>

/// this must be the same as 1 << B_TOTAL_EXP
#define STATE_SIZE 16384

extern void asm_store(uint64_t *addr, uint64_t val);
extern uint64_t asm_load(uint64_t *addr, bool is_clean);
