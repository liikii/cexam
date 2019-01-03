// replace string's blank to underline.
#include <stdio.h>


int main(int argc, char const *argv[])
{
	int f = argc - 1;
	for (int i = 1; i < argc; ++i)
	{	
		if (f != i)
		{
			printf("%s_", argv[i]);
		}else{
			printf("%s\n", argv[i]);
		}
		
	}
	return 0;

}
