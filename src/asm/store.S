#define STATE_SIZE 16384
.file "store.S"
.text
.globl asm_store
.type asm_store, @function

asm_store:
	add $STATE_SIZE, %rdi
	mov %rsi, %gs:(%rdi)
	retq
