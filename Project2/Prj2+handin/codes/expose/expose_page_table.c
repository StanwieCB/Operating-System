//////////////////////////////////////////////////////////////
/*
* Programmed twice
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-05-22
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project2 Problem2 a system call of linux, 
* to map a target process's page table into the current 
* process's address space
* 
* receive six arguments: 
* 1. pid_t pid
* 2. unsigned long fake_pgd
* 3. unsigned long fake_pmds
* 4. unsigned long page_table_addr
* 5. unsigned long begin_vaddr
* 6. unsigned long end_vaddr
* usage:
* expose_page_table(
* pid_t pid,
* unsigned long fake_pgd
* unsigned long fake_pmds
* unsigned long page_table_addr
* unsigned long begin_vaddr
* unsigned long end_vaddr)
*/
//////////////////////////////////////////////////////////////
/*
* change log 0528
* remove argument pmd due to the property of 32-bit emulator
*/
//////////////////////////////////////////////////////////////
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
#include <linux/slab.h>
#include <asm/errno.h>
#include <asm/pgtable.h>

#include "pagetable_layout_info.h"

MODULE_LICENSE("Dual BSD/GPL");
#define __NR_expose_page_table 		379
#define PGD_PAGE_COUNT 		   		3

static int (*oldcall)(void);

struct walk_record
{
    unsigned long *pgd;
    unsigned long pte_base;
};

//originally defined in /include/linux/mm.h line 1225
//an interface for user to implement custom walk method
int user_pmd_entry(pmd_t *pmd, unsigned long addr,
			 unsigned long next, struct mm_walk *walk)
{
	struct walk_record *record = (struct walk_record*)walk->private;
	unsigned long index = pgd_index(addr);
	int pfn;
	int err;

	struct vm_area_struct *vma = find_vma(current->mm, record->pte_base);
	
	//argument check
	if(!vma)
	{
		printk(KERN_INFO "FAIL TO FIND VMA!\n");
		return -EFAULT;
	}
	if(!pmd)
	{
		printk(KERN_INFO "INVALID PMD!\n");
		return -EINVAL;
	}
	if(pmd_bad(*pmd))
	{
		printk(KERN_INFO "BAD PMD!\n");
		return -EINVAL;
	}

	pfn = page_to_pfn(pmd_page((unsigned long)*pmd));
	if (!pfn_valid(pfn))
	{
		printk(KERN_INFO "BAD PFN!\n");
		return -EINVAL;
	}

	err = remap_pfn_range(vma, record->pte_base, pfn, PAGE_SIZE, vma->vm_page_prot);
	if(err)
	{
		printk(KERN_INFO "COPY PTE PAGE TO USER SPACE FAIL!\n");
		return -EFAULT;
	}

	//we use the index of va to trace the pa
	record->pgd[index] = record->pte_base;
	record->pte_base += PAGE_SIZE;
	printk(KERN_INFO "pgd[%lu]:\t 0x%08lx\n", index, record->pte_base);

	return 0;
}

void print_out_all_vma(struct task_struct* task)
{
	struct vm_area_struct *vma;

	printk(KERN_INFO "========== Virtual Addresses ==========\n");
	for (vma = task->mm->mmap; vma; vma = vma->vm_next)
	{
		printk(KERN_INFO "0x%08lx \t 0x%08lx\n", vma->vm_start, vma->vm_end);
	}
}

static int expose_page_table(
 	pid_t pid,
	unsigned long fake_pgd,
 	unsigned long page_table_addr,
 	unsigned long begin_vaddr,
 	unsigned long end_vaddr)
{
	struct walk_record record;
	struct mm_walk walk;
	struct task_struct *target;
	struct pid *pid_tmp;

	//check argument 
	if (!fake_pgd)
	{
        printk(KERN_INFO "INVALID FAKE_PGD\n");
		return -EINVAL;
	}

	if (!page_table_addr)
	{
        printk(KERN_INFO "INVALID PAGE_TABLE_ADDR\n");
		return -EINVAL;
	}

	if(begin_vaddr >= end_vaddr)
	{
		printk(KERN_INFO "INVALID BEGIN AND END VA\n");
		return -EINVAL;
	}

	//find targer task struct by pid
	pid_tmp = find_get_pid(pid);
	if (!pid_tmp)
	{
		printk(KERN_INFO "INVALID PID!\n");
		return -EINVAL;
	}
	target = get_pid_task(pid_tmp, PIDTYPE_PID);
	printk(KERN_INFO "Target process name:%s\n", target->comm);

	//target process page table lock and read
	down_write(&target->mm->mmap_sem);
	print_out_all_vma(target);

	//set up my walk
    record.pgd = (unsigned long*)kmalloc(PGD_PAGE_COUNT * PAGE_SIZE, GFP_KERNEL);
    if (!record.pgd)
    {
    	printk(KERN_INFO "RECORD PGD SPACE ALLOCATE FAIL!\n");
        return -EFAULT;
    }
    memset(record.pgd, 0, PGD_PAGE_COUNT * PAGE_SIZE);
    record.pte_base = page_table_addr;
    walk.pgd_entry = NULL;
    walk.pud_entry = NULL;
    walk.pmd_entry = user_pmd_entry;
    walk.pte_entry = NULL;
    walk.pte_hole = NULL;
    walk.hugetlb_entry = NULL;
    walk.mm  = target->mm;
    walk.private = (void*)(&record);

    walk_page_range(begin_vaddr, end_vaddr, &walk);
    up_write(&target->mm->mmap_sem);

    // copy the level-one page table to fake pgd
    if (copy_to_user((void*)fake_pgd, record.pgd, PGD_PAGE_COUNT * PAGE_SIZE))
    {
    	printk(KERN_INFO "COPY PGD PAGE TO USER SPACE FAIL!\n");
        return -EFAULT;
    }

    kfree(record.pgd);
	return 0;
}

static int addsyscall_init(void)
{
	long *syscall = (long*)0xc000d8c4;
	oldcall = (int(*)(void))(syscall[__NR_expose_page_table]);
	syscall[__NR_expose_page_table] = (unsigned long)expose_page_table;
	printk(KERN_INFO "EXPOSE_PAGE_TABLE MODULE LOAD!\n");
	return 0;
}

static void addsyscall_exit(void)
{
	long *syscall = (long*)0xc000d8c4;
	syscall[__NR_expose_page_table] = (unsigned long )oldcall;
	printk(KERN_INFO "EXPOSE_PAGE_TABLE MODULE EXIT!\n");	
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);
