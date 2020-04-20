all:
	gcc -I. -g -o stub -Wl,--wrap=malloc,--wrap=free stub.c asm/*.S allocator/*.c

poc: *.c pass.so
	clang -Xclang -load -Xclang ./llvm/pass.so ../examples/poc/poc.c -o ../examples/poc/poc

pass.so:
	clang++ -fPIC  ./llvm/pass.cpp -o ./llvm/pass.so `llvm-config --cxxflags` -shared