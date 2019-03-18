// c corourines
//  gcc -Wall ctst2.c -o tst2;./tst2
#include "stdio.h"


int range(int a, int b){
	printf("at range\n");
	static int i, state = 0;
    switch (state) {
        case 0: goto LABEL0;
        case 1: goto LABEL1;
    }
    LABEL0: /* start of function */
    for (i = a; i < b; i++) {
        state = 1; /* so we will come back to LABEL1 */
        return i;
        LABEL1:; /* resume control straight after the return */
    }
}

int main(int argc, char const *argv[])
{
	/* code */
	int i;
	for(;i=range(1, 10);)
		printf("at main: %d\n", i);
	return 0;
}
