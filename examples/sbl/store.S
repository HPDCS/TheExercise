
#include "basic.h"

.file "store.S"
.text
.globl store 
.type store, @function

store:
	mov %rdi, %rax
	sub $STATE_SIZE, %rsi
	mov %rax, %gs:(%rsi)
	retq

