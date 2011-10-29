#include <stdio.h>

void bla(void* data);

int main(argc,argv) 
{
	char bli[4][4];

	bli[0][0] = '5';
	bla(&bli);
	printf("valor de bli %c\n",bli[0][0]);
	
	return 0;
}

void bla(void* data)
{
	char ** test = (char**)data;
	test[0][0] = '9';
	printf("valor de bli dentro da funcao %c\n",test[0][0]);
}