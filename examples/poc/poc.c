#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct _object {
	int x;
} object;

int main(int argc, char **argv) {
	object *obj;

	obj = malloc(sizeof(object));
	obj->x = 128;

	printf("Object : %d\ncl", obj->x);

	return 0;
}
