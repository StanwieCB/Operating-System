//////////////////////////////////////////////////////////////
/*
* Programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-3-28	
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project1 Problem1 a system call of linux, 
* traverse process running in system and store them into a size limited 
* buffer in DFS order 

* receive two arguments: prinfo *buffer and int *size_of_buffer 
* usage:
* ptree(struct prinfo* buf, int *nr)
*/
//////////////////////////////////////////////////////////////
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/pid.h>
#include <linux/init_task.h>
#include <linux/spinlock.h>
#include <linux/rwlock_types.h>
#include <linux/rwlock.h>

#include "prinfo.h"

MODULE_LICENSE("Dual BSD/GPL");
#define __NR_ptree 233
DEFINE_RWLOCK(buf_lock);

static int (*oldcall)(void);

//copy process info into the buffer
static int info_copy(struct prinfo *p, struct task_struct *ts)
{
	if (p == NULL)
	{
		printk("miss of address of prinfo/ wrong pointer!\n");
		return -1;
	}
	if (ts == NULL)
	{
		printk("miss of address of task_struct/ wrong pointer!\n");
		return -1;
	}
	//copy parent pid
	p->parent_pid = ts->parent->pid;
	//copy pid
	p->pid = ts->pid;
	//copy first child pid
	if (list_empty(&(ts->children)))
		p->first_child_pid = 0;
	else
		p->first_child_pid = (list_entry((ts->children).next, struct task_struct, sibling))->pid;
	//copy next sibling pid
	if (list_empty(&(ts->sibling)))
		p->next_sibling_pid = 0;
	else
		p->next_sibling_pid = (list_entry((ts->sibling).next, struct task_struct, sibling))->pid;
	//copy state
	p->state = ts->state;
	//copy uid
	p->uid = ts->real_cred->uid;
	//copy process name
	memset((void*)p->comm, 0, 64);

	//printk("\n%s\n", p->comm);	//debug code

 	get_task_comm(p->comm, ts);

 	return 0;
}

//the DFS function
void DFS(struct prinfo *buf, 
		 struct task_struct *task,
		 int *buf_ind,
		 const int size_limit)
{
	struct task_struct *child;

	//buffer is full or no more process 
	if (*buf_ind >= size_limit || task == NULL)
		return;

	write_lock(&buf_lock);
	info_copy(buf + (*buf_ind), task);
	(*buf_ind)++;
	write_unlock(&buf_lock);

	if (!list_empty(&task->children))
	{
        list_for_each_entry(child, &task->children, sibling)
        {
        	DFS(buf, child, buf_ind, size_limit);
        }
    }
}

//definition of system call ptree
//taverse through processes
static int ptree(struct prinfo __user *buf, int __user *nr)
{
	int __kernel kn_nr = 0;
	int __kernel kn_lim = 0;
	struct prinfo __kernel *kn_buf;

	//*kn_nr = 0;
	//*kn_lim = 0;

	//if arguments are not valid then return
	if (buf == NULL)
	{
		printk("invalid buf argument!\n");
		printk("ptree fail!\n");
		return -1;
	}
	if (nr == NULL)
	{
		printk("invalid nr argument!\n");
		printk("ptree fail!\n");
		return -1;
	}

	//copy limit
	if (copy_from_user(&kn_lim, nr, sizeof(int)))
	{
		printk("copy nr error!\n");
		printk("ptree fail!\n");
		return -1;
	}
	else
		printk("copy nr success!\n");

	//allocate space for kernel buffer
	kn_buf = kmalloc(kn_lim * sizeof(struct prinfo), GFP_KERNEL);
	if (kn_buf == NULL)
	{
		printk("allocate space for kernel buffer error!\n");
		printk("ptree fail!\n");
		return -1;
	}
	else
		printk("allocate space for kernel buffer success!\n");

	read_lock(&tasklist_lock);
	DFS(kn_buf, &init_task, &kn_nr, kn_lim);
	read_unlock(&tasklist_lock);

	//copy back to user buffer
	if (copy_to_user(buf, kn_buf, kn_nr * sizeof(struct prinfo)))
	{
		printk("copy back to user buffer error!\n");
		printk("ptree fail!\n");
		return -1;
	}

	//copy back to user nr
	if (copy_to_user(nr, &kn_nr, sizeof(int)))
	{
		printk("copy back to user nr error!\n");
		printk("ptree fail!\n");
		return -1;
	}

	//free kernel space;
	kfree(kn_buf);
	//kfree(kn_nr);
	//kfree(kn_lim);

	printk("ptree success!\n");
	return 0;
}

static int addsyscall_init(void)
{
	long *syscall = (long*)0xc000d8c4;
	oldcall = (int(*)(void))(syscall[__NR_ptree]);
	syscall[__NR_ptree] = (unsigned long )ptree;
	printk(KERN_INFO "ptree module load!\n");
	return 0;
}

static void addsyscall_exit(void)
{
	long *syscall = (long*)0xc000d8c4;
	syscall[__NR_ptree] = (unsigned long )oldcall;
	printk(KERN_INFO "ptree module_exit!\n");	
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);
