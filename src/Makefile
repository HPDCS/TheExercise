all:
	gcc -I. -g -o stub -Wl,--wrap=malloc,--wrap=free stub.c asm/*.S allocator/*.c
