#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <asm/prctl.h>
#include <sys/prctl.h>
#include "basic.h"
#include "application.h"


extern void store(unsigned long, unsigned long);
extern unsigned long load(unsigned long);

extern int arch_prctl(int code, unsigned long addr);

void setup_state(elem*);
void audit(char*);
void process_task(char*);

char* buffer;
char* state_pointer;
unsigned long shadow_state_offset;
unsigned long offset;

int main(int argc, char** arv){

	int cycles;

        buffer = (char*)mmap(NULL,TOTAL_MEMORY,PROT_READ|PROT_WRITE,MAP_ANONYMOUS|MAP_PRIVATE,0,0);
        if (buffer == NULL){
                printf("mmap error\n");
                return EXIT_FAILURE;
        }

	state_pointer = buffer+TOTAL_MEMORY-STATE_SIZE;//the very last state buffer is the initial "current" - the sliding window of buffers moves towads higher virtual addresses


	//application specific stuff
	setup_state((elem*)buffer);
	memcpy((char*)state_pointer,(char*)buffer,STATE_SIZE);
	//audit(state_pointer);

	//simulate the processing of NUM_CYLCES tasks
	for(cycles = 0 ; cycles < NUM_CYCLES; cycles++){

		//this is to setup the future state buffer (I repeat this in the loop but it can be also put outside in this testbed)
		shadow_state_offset = TOTAL_MEMORY - (cycles+1)*STATE_SIZE;
		printf("shadow state offset is %lu - set returned %d\n",shadow_state_offset,arch_prctl(ARCH_SET_GS,shadow_state_offset));	
		arch_prctl(ARCH_GET_GS,(unsigned long)&offset);
		printf("GS set to %lu\n",offset);
		fflush(stdout);

		state_pointer = buffer; 
		//just audit
		printf("cycle %d\n",cycles);
		fflush(stdout);
		
		//just audit
		printf("shadow state buffer setup done\n");
		fflush(stdout);
	
		//application specific stuff
		process_task(state_pointer);

		printf("task ended\n");
		fflush(stdout);
		sleep(1);
	
	}

        return 0;
}

void setup_state(elem* p){
	int i;
	elem* aux;

	printf("initilizing the state at address %p\n",p);
	fflush(stdout);
	//just writing the 'a' char into the state bytes
	//memset(p,97,STATE_CONTENT);
	for (i = 0; i< STATE_CONTENT; i++){
		p[i].val = 0;	
		p[i].next = (p + i + 1);
	}
	p[--i].next = NULL;
	aux = p;	
 	while(1){	
		printf("val is %lu\n",aux->val);	
		aux= aux->next;
	       	if (aux == NULL) break;
		}
	
}


//this simulates the processing of the event at the object, with the whole state being updated (list pointers are simply rewrittenwih the same value so as to post them in the future buffer
void process_task(char *p){
	int i = 0;
	unsigned long c,b;
	elem* aux;
	unsigned long next, next_elem;

	aux = p;
	printf("updating state at address %p - (this is the placeholder address - actual operations apply to current/future instances)\n",p);
	fflush(stdout);

	next = aux;	
	while(next != NULL){
		c = load(next);
		printf("found in state list %lu - updating to %lu\n",c,c+1);
		fflush(stdout);
		store(c+1,next);
		next_elem = load((unsigned long)((char*)next+sizeof(unsigned long)));
		store(next_elem,(unsigned long)((char*)next+sizeof(unsigned long)));
		next = next_elem;
	}

}
