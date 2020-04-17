#include <allocator/allocator.h>

#include <asm/prctl.h>
#include <sys/prctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

struct mm_state {
	void *base_mem;
	uint_fast8_t current;
	uint_fast32_t used_mem;
	block_bitmap clean[bitmap_required_size(1 << (B_TOTAL_EXP - B_BLOCK_EXP))];
	block_bitmap aggr_written[bitmap_required_size(1 << (B_TOTAL_EXP - B_BLOCK_EXP))];
	uint_least8_t longest[(1 << (B_TOTAL_EXP - B_BLOCK_EXP + 1)) - 1];
} self;

#define max(a, b) 			\
__extension__({				\
	__typeof__ (a) _a = (a);	\
	__typeof__ (b) _b = (b);	\
	_a > _b ? _a : _b;		\
})

#define left_child(i) (((i) << 1U) + 1U)
#define right_child(i) (((i) << 1U) + 2U)
#define parent(i) ((((i) + 1) >> 1U) - 1U)
#define is_power_of_2(i) (!((i) & ((i) - 1)))
#define next_exp_of_2(i) (sizeof(i) * CHAR_BIT - SAFE_CLZ(i))

extern void *__real_malloc(size_t mem_size);
extern void __real_free(void *ptr);

extern int arch_prctl(int code, unsigned long addr);

void allocator_init(void)
{
	uint_fast8_t node_size = B_TOTAL_EXP;

	for (uint_fast32_t i = 0;
		i < sizeof(self.longest) / sizeof(*self.longest); ++i) {
		self.longest[i] = node_size;
		node_size -= is_power_of_2(i + 2);
	}

	memset(self.clean, 0, sizeof(self.clean));
	memset(self.aggr_written, 0, sizeof(self.aggr_written));

	self.current = 0;
	self.used_mem = 0;
	self.base_mem = __real_malloc((1 << B_TOTAL_EXP) * (B_LOGS_COUNT + 2));
}

void allocator_fini(void)
{
	__real_free(self.base_mem);
}

void allocator_start_processing(void)
{
	arch_prctl(ARCH_SET_GS, self.current * (1 << B_TOTAL_EXP));
}

void allocator_done_processing(void)
{
	if(self.current == B_LOGS_COUNT){
		// TODO handle buffering cycle
	}

	unsigned char *old_ptr = ((unsigned char *)self.base_mem) + self.current * (1 << B_TOTAL_EXP);
	unsigned char *new_ptr = old_ptr + (1 << B_TOTAL_EXP);

#define copy_set_chunks(i) 				\
	memcpy(						\
		new_ptr + i * (1 << B_BLOCK_EXP), 	\
		old_ptr + i * (1 << B_BLOCK_EXP), 	\
		(1 << B_BLOCK_EXP)			\
	)

	bitmap_foreach_set(self.clean, sizeof(self.clean), copy_set_chunks);

#undef copy_set_chunks

	memcpy(self.clean, self.aggr_written, sizeof(self.clean));

	++self.current;
}

void allocator_rollback(unsigned steps)
{
	if(steps > B_LOGS_COUNT){
		printf("You are trying to rollback too much my friend");
		abort();
	}

	// XXX this obviously is wrong if we wrap around
	self.current -= steps;
}

void *__wrap_malloc(size_t req_size)
{
	if(!req_size)
		return NULL;

	uint_fast8_t req_blks = max(next_exp_of_2(req_size - 1), B_BLOCK_EXP);

	if (self.longest[0] < req_blks) {
		errno = ENOMEM;
		return NULL;
	}

	/* search recursively for the child */
	uint_fast8_t node_size;
	uint_fast32_t i;
	for (
		i = 0, node_size = B_TOTAL_EXP;
		node_size > req_blks;
		--node_size
	) {
		/* choose the child with smaller longest value which
		 * is still large at least *size* */
		i = left_child(i);
		i += self.longest[i] < req_blks;
	}

	/* update the *longest* value back */
	self.longest[i] = 0;
	self.used_mem += 1 << node_size;

	uint_fast32_t offset = ((i + 1) << node_size) - (1 << B_TOTAL_EXP);

	while (i) {
		i = parent(i);
		self.longest[i] = max(
			self.longest[left_child(i)],
			self.longest[right_child(i)]
		);
	}

	return ((char *)self.base_mem) + offset;
}

void *__wrap_calloc(size_t nmemb, size_t size)
{
	size_t tot = nmemb * size;
	void *ret = __wrap_malloc(tot);

	if(ret)
		memset(ret, 0, tot);

	return ret;
}

void __wrap_free(void *ptr)
{
	if(!ptr)
		return;

	uint_fast8_t node_size = B_BLOCK_EXP;
	uint_fast32_t i =
		(((uintptr_t)ptr - (uintptr_t)self.base_mem) >> B_BLOCK_EXP) +
		(1 << (B_TOTAL_EXP - B_BLOCK_EXP)) - 1;

	for (; self.longest[i]; i = parent(i)) {
		++node_size;
	}

	self.longest[i] = node_size;
	self.used_mem -= 1 << node_size;

	while (i) {
		i = parent(i);

		uint_fast8_t left_longest = self.longest[left_child(i)];
		uint_fast8_t right_longest = self.longest[right_child(i)];

		if (left_longest == node_size && right_longest == node_size) {
			self.longest[i] = node_size + 1;
		} else {
			self.longest[i] = max(left_longest, right_longest);
		}
		++node_size;
	}

	// little "optimization", we reset the bitmaps so we don't keep copying stuff
	uint_fast32_t off_b = ((uintptr_t)ptr - (uintptr_t)self.base_mem) >> B_BLOCK_EXP;
	i = 1 << (node_size - B_BLOCK_EXP);
	while(i--){
		bitmap_reset(self.clean, i + off_b);
		bitmap_reset(self.aggr_written, i + off_b);
	}
}

void write_mem(void *ptr)
{
	uint_fast32_t i =
		(((uintptr_t)ptr - (uintptr_t)self.base_mem) >> B_BLOCK_EXP);
	bitmap_reset(self.clean, i);
	bitmap_set(self.aggr_written, i);
}

/// the instrumentor calls this function to know if the address has been written
bool is_clean(void *ptr)
{
	uint_fast32_t i =
		(((uintptr_t)ptr - (uintptr_t)self.base_mem) >> B_BLOCK_EXP);
	return bitmap_check(self.clean, i);
}
