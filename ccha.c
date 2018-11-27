#include <stdio.h>


int main(int argc, char const *argv[])
{
	/* code */
	long nc;
	nc = 0;

	while (getchar() != EOF){
		++nc;
	}
	printf("%ld\n", nc);
	return 0;
}
