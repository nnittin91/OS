#ifndef __KMALLOC_H__
#define __KMALLOC_H__

#include<sys/kprintf.h>
#include<sys/defs.h>
//#include<sys/dataTypes.h>
//#include<sys/virtual_memory.h>
//#include<sys/page_init.h>

#define PAGE_SIZE 4096
#define BLOCK_ALIGNMENT_SIZE 16
#define BLOCK_ALIGNMENT_SIZE_PWR_OF_2 4

static uint32_t total_curr_free_memory_available;
static uint64_t ptr_to_current_free_memory;

void initialize_local_kernel_malloc();
void *kmalloc(int sz);

#endif
