//////////////////////////////////////////////////////////////
/*
* Programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-05-22s	
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project2 Problem2 a system call of linux, 
* to get the page table layout information of current system
* 
* receive two arguments: 
* 1. struct pagetable_layout_info __user * pgtbl_infotand 
* 2. int size
* usage:
* get_pagetable_layout(
* struct pagetable_layout_info __user * 
* pgtbl_info, int size)
*/
//////////////////////////////////////////////////////////////
#include <linux/init.h>
#include <linux/init_task.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/mman.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/rwlock_types.h>
#include <linux/rwlock.h>
#include <linux/spinlock.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <asm/pgtable.h>
#include <asm/page.h>
#include <asm/memory.h>

#include "pagetable_layout_info.h"

MODULE_LICENSE("Dual BSD/GPL");

#define __NR_get_pagetable_layout 378

static int (*oldcall)(void);

//definition of system call get_pagetable_layout
static int get_pagetable_layout(
	struct pagetable_layout_info __user * pgtbl_info,
	int size)
{
	struct pagetable_layout_info kernel_info;

	if (size < sizeof(struct pagetable_layout_info))
	{
		printk(KERN_INFO "THE SIZE OF PGTBL_INFO IS TOO SMALL!\n");
		return -EINVAL;
	}
	kernel_info.pgdir_shift = PGDIR_SHIFT;
	kernel_info.pmd_shift = PMD_SHIFT;
	kernel_info.page_shift = PAGE_SHIFT;
	if(copy_to_user(pgtbl_info, &kernel_info, size))
	{
		printk(KERN_INFO "COPY BACK TO USER SPACE FAIL!\n");
		return -EFAULT;
	}
	return 0;
}

static int addsyscall_init(void)
{
	long *syscall = (long*)0xc000d8c4;
	oldcall = (int(*)(void))(syscall[__NR_get_pagetable_layout]);
	syscall[__NR_get_pagetable_layout] = (unsigned long)get_pagetable_layout;
	printk(KERN_INFO "GET_PAGETABLE_LAYOUT MODULE LOAD!\n");
	return 0;
}

static void addsyscall_exit(void)
{
	long *syscall = (long*)0xc000d8c4;
	syscall[__NR_get_pagetable_layout] = (unsigned long )oldcall;
	printk(KERN_INFO "GET_PAGETABLE_LAYOUT MODULE EXIT!\n");	
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);
