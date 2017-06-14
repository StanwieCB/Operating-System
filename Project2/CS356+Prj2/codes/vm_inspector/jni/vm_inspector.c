//////////////////////////////////////////////////////////////
/*
* Programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 2017-05-24
*/
//////////////////////////////////////////////////////////////
/*
* CS356 Project2 Problem3 virtual address investigation, 
* investigate some process's virtual address and coressponding
* physical address in a given range
* 
* receive 3 arguments: 
* 1. pid_t pid
* 2. unsigned long begin virtual address
* 3. unsigned long end virtual address
*/
//////////////////////////////////////////////////////////////

#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>

#define PGD_COUNT 						1528
#define PGD_PAGE_COUNT 					3
#define PGDIR_SHIFT						21
#define PHYS_MASK 0xFFFFF000

#define pgd_index(page_table_addr)      ((page_table_addr) >> PGDIR_SHIFT)

void print_out(unsigned long *addr, int pgd_index, int pte_index, 
	unsigned long begin_vaddr, unsigned long end_vaddr)
{
	unsigned long pte;
	unsigned long va;
	unsigned long phys_addr;
	unsigned long pgd_address;


	if (addr == NULL)
		return;

	pte = *addr;

	pgd_address = pgd_index * 512 * PAGE_SIZE;
	va = pgd_address + pte_index * PAGE_SIZE;

	phys_addr = pte & PHYS_MASK;

	if (phys_addr == 0)
		return;

	if (va + (1 << 12) >= begin_vaddr) 
	{
		if (va > end_vaddr)
		 	return;
		printf("0x%x\t0x%08lx\t0x%08lx\n", pgd_index, va, phys_addr);
	}
}


void print_pte_table(unsigned long *addr, int index, 
	unsigned long begin_vaddr, unsigned long end_vaddr)
{
	int i;

	if (addr == NULL)
		return;

	for (i = 0; i < 512; i++)
		print_out(addr++, index, i, begin_vaddr, end_vaddr);
}

int main(int argc, char **argv)
{
	int pid;
	char *ptr;
	void *addr, *fake_pgd_addr;
	unsigned long begin_vaddr = 0;
	unsigned long end_vaddr = 0;
	unsigned long page_table_addr, fake_pgd, index;
	unsigned long **fake_pgd_new;
	int verbose = 0, i = 0;

	if (argc == 4)
	{
		begin_vaddr = strtoul(argv[2], &ptr, 16);
		end_vaddr = strtoul(argv[3], &ptr, 16);
	}
	else if (argc == 3 || argc < 2 || argc > 4) 
	{
		printf("USAGE: ./vm_inspector pid va_begin va_end\n");
		return -1;
	}
	if (begin_vaddr > end_vaddr) {
		printf("INVALID BEGIN_VADDR AND END_VADDR\n");
		return -1;
	}

	pid = atoi(argv[1]);

	addr = mmap(0, PGD_COUNT * PAGE_SIZE, PROT_READ,
		MAP_SHARED|MAP_ANONYMOUS, -1, 0);

	if (addr == MAP_FAILED) {
		printf("ERROR: %s\n", strerror(errno));
		return -1;
	}

	page_table_addr = (unsigned long) addr;

	fake_pgd_addr = mmap(0, PGD_PAGE_COUNT * PAGE_SIZE,
	PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	if (fake_pgd_addr == MAP_FAILED) 
	{
		printf("ERROR: %s\n", strerror(errno));
		goto free_addr;
	}

	fake_pgd = (unsigned long) fake_pgd_addr;

	if (syscall(379, pid, fake_pgd, page_table_addr, begin_vaddr, end_vaddr))
	{
		printf("ERROR: %s\n", strerror(errno));
		goto free_all;
	}

	fake_pgd_new = (unsigned long **) fake_pgd_addr;

	index = pgd_index(page_table_addr);

	printf("================== Beginning Dump ==================\n");

	for (i = 0; i < PGD_COUNT; i++) 
	{
		if (fake_pgd_new[i] != NULL) 
			print_pte_table(fake_pgd_new[i], i, begin_vaddr, end_vaddr);
	}

	munmap(addr, PGD_COUNT * PAGE_SIZE);
	munmap(fake_pgd_addr, PGD_PAGE_COUNT * PAGE_SIZE);

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
