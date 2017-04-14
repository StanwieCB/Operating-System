//////////////////////////////////////////////////////////////
/*
* programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 03-28-2017
*/
//////////////////////////////////////////////////////////////
/*
* header of prinfo struct
* required in the project
*/
//////////////////////////////////////////////////////////////

#ifndef _LINUX_PRINFO_H
#define _LINUX_PRINFO_H

struct prinfo
{
	pid_t parent_pid;			///process id of parent
	pid_t pid;					//process id
	pid_t first_child_pid;		//pid of youngest child
	pid_t next_sibling_pid;		//pid of older sibling
	long state;					//current state of process
	long uid;					//user id of process owner
	char comm[64];				//name of program executed
};

#endif