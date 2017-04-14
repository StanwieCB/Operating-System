//////////////////////////////////////////////////////////////
/*
* Programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-3-31
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project1 Problem3 a test programm to test the relation 
* beteen parent process and child process with execl ptree
*/
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


//main test function
int main()
{
	pid_t pid;
	pid_t pidc;
	pid_t pidp;

	printf("testing parent and child relation!\n");

	pid = fork();

	//fork child process failure
	if (pid < 0)
	{
		printf("fork child process failure!\n");
		return -1;
	}
	else if (pid == 0)
	{
		printf("This is a child process!\n");
		pidc = getpid();
		printf("515030910216 Child: %d\n", pidc);
		printf("execl pstree in child process!\n");
		//NOTE: please change the path to corresponding file!!!!
		execl("/data/osprj/problem2/./testptreeARM","testptreeARM","100",(char*) NULL);
		exit(EXIT_SUCCESS);
	}
	else
	{
		wait(NULL);
		printf("Child process finish!\nThis is a parent process\n");
		pidp = getpid();
		printf("515030910216 Parent: %d\n", pidp);
	}

	return 0;
}