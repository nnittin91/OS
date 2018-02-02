#ifndef __VIRTUAL_MEMORY_HELP_H__
#define __VIRTUAL_MEMORY_HELP_H__

#include<sys/defs.h>

#include<sys/kprintf.h>

#include<sys/help.h>

#include<sys/physical_memory_help.h>
#include<sys/paging_support_help.h>
uint64_t highestVirtual_ADDR;

uint64_t get_highestVirtual_ADDR();
uint64_t set_highestVirtual_ADDR(uint64_t address);
uint64_t inc_highestVirtual_ADDR(uint64_t size);
void* allocate_n_virtual_pages_memory(uint32_t number_of_virt_pages, uint64_t perm_flags);

void release_virtual_page_mem(void * virtual_mem_addr);
uint64_t get_virtual_for_given_physical_ADDR(uint64_t address);

#endif
