//////////////////////////////////////////////////////////////
/*
* Programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-06-05
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project2 Problem4 test program
* occupy 1 GB memory to activate my linux
* page replacement algorithm
*/
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/wait.h>

int main()
{
	unsigned long **mem;
	unsigned long i,j;
	printf("Start Problem4 testing program!\n");

	mem = (unsigned long **)malloc(1<<15);
	if(!mem)
	{
		printf("testing program failed to occupy memory\n");
		return -1;
	}

	for (i = 0; i < (1<<13); i++)
	{
		mem[i] = (unsigned long*)malloc(1<<17);
		for (j = 0; j < 32; ++j)
		{
			mem[i][j<<10] = 1;
		}
	}
	printf("Hold for 2s\n");
	sleep(2);
	printf("Start releasing!\n");
	for (i = 0; i < (1<<13); i++)
	{
		free(mem[i]);
	}
	free(mem);
	return 0;
}