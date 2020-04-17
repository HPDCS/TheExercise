#define STATE_SIZE 16384
.file "load.S"
.text
.globl asm_load
.type asm_load, @function

asm_load:
	test %rsi, %rsi
	jnz clean
	add $STATE_SIZE, %rdi
clean:
	mov %gs:(%rdi), %rax
	retq

