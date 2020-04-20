.file "load.S"
.text
.globl load 
.type load, @function

load:
	call is_dirty
	mov %rax, %rcx
	test %rcx, %rcx
	mov %gs:(%rbx, %rcx, 1), %rax
	retq