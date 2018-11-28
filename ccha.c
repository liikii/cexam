#include <stdio.h>
/*
 Ctrl-D or Ctrl-Z for EOF
*/

int main(int argc, char const *argv[])
{
	/* code */
	// long nc;
	// nc = 0;

	// while (getchar() != EOF){
	// 	++nc;
	// }
	// printf("%l\n", nc);

	double nc;
	for (nc = 0.0; getchar() != EOF; ++nc)
		;
	printf("%.0f\n", nc);
	return 0;
}
