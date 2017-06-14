//////////////////////////////////////////////////////////////
/*
* Programmed twice
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-05-24
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project2 Problem2 virtual address translation, 
* translate a specific virtual address of some process
* 
* receive 2 arguments: 
* 1. pid_t pid
* 2. unsigned long virtual address
*/
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include "pagetable_layout_info.h"

#define PGD_COUNT 1528
#define PGD_PAGE_COUNT 3

#define PGDIR_SHIFT             21
#define pgd_index(addr)         ((addr) >> PGDIR_SHIFT)
#define pte_index(addr)			((addr >> PAGE_SHIFT) & 511)
#define page_offset(addr)		(addr & 4095)

#define PHYS_MASK 0xFFFFF000

int main(int argc, char **argv)
{
    struct pagetable_layout_info info;
	int pid, ret;
	char *ptr;
	void *addr, *fake_pgd_addr;
	unsigned long begin_vaddr = 0;
	unsigned long end_vaddr = 0;
	unsigned long page_table_addr, fake_pgd, index;
	unsigned long **fake_pgd_new;

	if (argc == 3) 
	{
		begin_vaddr = strtoul(argv[2], &ptr, 16);
		end_vaddr = begin_vaddr + 1;
	} 
	else if (argc < 2 || argc > 4)
	{
		printf("USAGE: ./VATranslate pid va\n");
		return -1;
	}

    if(syscall(378, &info, sizeof(struct pagetable_layout_info)))
    {
    	printf("GET PAGE TABLE LAYOUT INFO FAIL!\n");
    	return -1;
    };
	
    printf("pgdir_shift: %u\t", info.pgdir_shift);
    printf("pmd_shift: %u\t", info.pmd_shift);
    printf("page_shift: %u\n", info.page_shift);

	int ctr = 0;

	pid = atoi(argv[1]);

	addr = mmap(0, PGD_COUNT * PAGE_SIZE, PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (addr == MAP_FAILED)
	{
		printf("MMAP ADDR FAIL: %s\n", strerror(errno));
		return -1;
	}

	page_table_addr = (unsigned long) addr;

	fake_pgd_addr = mmap(0, PGD_PAGE_COUNT * PAGE_SIZE,
	PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	if (fake_pgd_addr == MAP_FAILED) 
	{
		printf("MMAP FAKE FAIL: %s\n", strerror(errno));
		goto free_addr;
	}

	fake_pgd = (unsigned long) fake_pgd_addr;

	if (syscall(379, pid, fake_pgd, 
		page_table_addr, begin_vaddr, end_vaddr)) 
	{
		printf("EXPOSE PAGE TABLE FAIL: %s\n", strerror(errno));
		goto free_all;
	}

	fake_pgd_new = (unsigned long **) fake_pgd_addr;

	index = pgd_index(begin_vaddr);

	printf("virtual address:0x%08lx ====> PA: 0x%08lx\n", begin_vaddr, 
		(*(fake_pgd_new[index] + pte_index(begin_vaddr)) & PHYS_MASK) 
		+ page_offset(begin_vaddr));


free_addr:
	munmap(addr, PGD_COUNT * PAGE_SIZE);
	return -1;
free_fake_pgd:
	munmap(fake_pgd_addr, PGD_PAGE_COUNT * PAGE_SIZE);
	return -1;
free_all:
	munmap(addr, PGD_COUNT * PAGE_SIZE);
	munmap(fake_pgd_addr, PGD_PAGE_COUNT * PAGE_SIZE);
	return -1;

    return 0;
}
