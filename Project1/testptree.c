//////////////////////////////////////////////////////////////
/*
* Programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-3-30	
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project1 Problem2 a test programm for system call ptree. 
* traverse process running in system and print their info out 
* in DFS order 
*/
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prinfo.h"

//function to print out info of one process in buffer
void print_info(struct prinfo *itr)
{
	printf("%s,%d,%ld,%d,%d,%d,%ld\n", itr->comm, itr->pid, itr->state, 
		itr->parent_pid, itr->first_child_pid, itr->next_sibling_pid, itr->uid);
}

//function to print num tabs
void print_indent(int num)
{
	while(num--)
		printf("\t");
}

//print all process in buffer
void print_out(struct prinfo *buf, int size)
{
	int i, j;
	int *indent;

	indent = malloc(size*sizeof(int));
	memset(indent, 0, size);

	//print first process info
	print_info(buf);
	for(i = 1; i < size; i++)
	{
		//is son
		if (buf[i].parent_pid == buf[i-1].pid)
			indent[i] = indent[i-1] + 1;
		//find sibling
		else
		{
			j = i - 1;
			while (j >= 0 && buf[j].parent_pid != buf[i].parent_pid)
				j--;
			indent[i] = indent[j];
		}
		print_indent(indent[i]);
		print_info(buf+i);
	}

	free(indent);
}	

int main(int argc, char **argv)
{
	struct prinfo *buf;
	int buf_size;

	printf("This is a test of pstree!\n");

	//wrong argument test
	if (argc != 2)
	{
		printf("invalid number of arguments!\n");
		return -1;
	}
	//wrong buf_size test
	if (!(buf_size = (int)strtol(argv[1], NULL, 10)))
	{
		printf("invalid buffer size!\n");
		return -1;
	}

	buf = malloc(buf_size*sizeof(struct prinfo));
	//call system call 233
	if (syscall(233, buf, &buf_size))
	{
		printf("error occurs in ptree!\n");
		return -1;
	}

	print_out(buf, buf_size);
	printf("Test End!\n");

	free(buf);

	return 0;
}