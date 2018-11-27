#include <stdio.h>


int main()
{
	/* c = getchar();
 	while(c!= EOF){
 		putchar(c);
 		c = getchar();
 	}  
 	*/
    int c;

    while((c = getchar()) != EOF)
    	putchar(c);
    return 0;
}
