#include <asm/asm.h>
#include <allocator/allocator.h>

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define B_SIZ 5

void write_sequence(uint64_t *buffer, unsigned count, unsigned start_from)
{
	while(count--){
		asm_store(buffer, start_from++);
		write_mem(buffer);
		++buffer;
	}
}

void print_sequence(uint64_t *buffer, unsigned count)
{
	while(count--){
		bool clean = is_clean(buffer);
		printf("%s read ", clean ? "clean" : "dirty");
		printf("%" PRIu64 "\n", asm_load(buffer, clean));
		++buffer;
	}
}

int main(void)
{
	allocator_init();

	allocator_start_processing();
	uint64_t *buffer_a = malloc(B_SIZ * sizeof(uint64_t));
	write_sequence(buffer_a, B_SIZ, 10);
	print_sequence(buffer_a, B_SIZ);
	allocator_done_processing();

	allocator_start_processing();
	print_sequence(buffer_a, B_SIZ);
	write_sequence(buffer_a, B_SIZ, 34);
	print_sequence(buffer_a, B_SIZ);
	allocator_done_processing();

	allocator_start_processing();
	print_sequence(buffer_a, B_SIZ);

	allocator_done_processing();

	allocator_rollback(1);
	allocator_start_processing();
	print_sequence(buffer_a, B_SIZ);

	allocator_rollback(1);
	allocator_start_processing();
	print_sequence(buffer_a, B_SIZ);

	allocator_fini();
}
