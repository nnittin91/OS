
#include<sys/virtual_memory_help.h>


uint64_t highestVirtual_ADDR;

uint64_t get_highestVirtual_ADDR()
{
return highestVirtual_ADDR;
}

void inc_highestVirtual_ADDR(uint64_t size)
{
highestVirtual_ADDR+=size;
}
uint64_t set_highestVirtual_ADDR(uint64_t targetAddress)
{
highestVirtual_ADDR = targetAddress;
}

void* allocate_n_virtual_pages_memory(uint32_t number_of_virt_pages, uint64_t perm_flags)
{
void * return_ADDR = (void *)get_highestVirtual_ADDR();

while(number_of_virt_pages > 0)
{
uint64_t local_physical_ADDR = allocate_physical_block_of_memory_space();
do_virtual_2_physical_mapping(highestVirtual_ADDR,local_physical_ADDR,perm_flags);
inc_highestVirtual_ADDR(4096);
number_of_virt_pages --;
}
return return_ADDR;
}

void release_virtual_page_mem(void * virtual_mem_addr)
{
uint64_t local_var = (uint64_t)(virtual_mem_addr);
free_physical_block_of_pages(*(get_page_table_entry(local_var)) &  0x000FFFFFFFFFF000UL,1);

*(get_page_table_entry(local_var)) = 0;
}

uint64_t get_virtual_for_given_physical_ADDR(uint64_t address)
{
do_virtual_2_physical_mapping(highestVirtual_ADDR,address,0x7UL);
return  highestVirtual_ADDR;
}

//write zero_out_phys_block as it is

