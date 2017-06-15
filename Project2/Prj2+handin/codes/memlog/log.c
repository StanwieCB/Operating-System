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
* CS356 Project2 Problem 4 test program
* record active list and inactive list mem info
*/
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int main()
{
	int i,j;
	char str[256] = { 0 };

	freopen("log.out","w",stdout);
	for (i = 0; i < 15; ++i)
	{
		printf("Second: %d\n", i);
		freopen("/proc/meminfo","r", stdin);

		//jump to line 6
		for (j = 0; j < 5; ++j)
		{
			fgets(str, 256, stdin);
		}
		
		//output line 6 and line 7
		for (j = 0; j < 2; ++j)
		{
			fgets(str, 256, stdin);
			printf("%s", str);
		}
		fclose(stdin);
		sleep(1);
	}
	fclose(stdout);
	return 0;
}