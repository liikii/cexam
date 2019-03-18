#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

int main(int argc, char const *argv[])
{
	/* code */
	int *p = malloc(sizeof(int));
	assert(p != NULL);
	printf("%d --  %08x\n", getpid(), (unsigned) p);
	*p = 0;
	while(1){
		Spin(1);
		*p = *p + 1;
		printf("%d  %d\n", getpid(), *p);
	}
	return 0;
}
