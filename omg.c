#include <stdio.h>
#include <time.h>

int	main(int argc,char** argv)
{
		char buffer[80];
		int vinte = 20;
		char bla[6] = "slave\0";


		sprintf(buffer,"%s%d.txt",bla,vinte);

		printf("%s\n",buffer);

		return 0;
}