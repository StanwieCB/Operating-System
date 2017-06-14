//////////////////////////////////////////////////////////////
/*
* programmed once
* author:hank zhang(StanwieCB)
* student number: 515030910216
* mail: hankzhangcb@gmail.com
* 05-22-2017
*/
//////////////////////////////////////////////////////////////
/*
* header of pagetable_layout_info struct
* required in the project
*/
//////////////////////////////////////////////////////////////
#ifndef _LINUX_PRINFO_H
#define _LINUX_PRINFO_H

struct pagetable_layout_info
{
	unit32_t pgdir_shift 		//physical address of second-level page table
	unit32_t pmd_shift 			//physical address of third-level page table
	unit32_t page_shift 		//shift amount
};
#endif