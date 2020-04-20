.file "load.S"
.text
.globl load 
.type load, @function

load:
	mov %gs:(%rdi), %rax
	retq

