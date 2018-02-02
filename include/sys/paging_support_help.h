#ifndef __PAGING_SUPPORT_HELP_H__
#define __PAGING_SUPPORT_HELP_H__

#include<sys/defs.h>
#include<sys/kmalloc.h>
#include<sys/physical_memory_help.h>
#include<sys/virtual_memory_help.h>
#include<sys/process_memory_help.h>
#define ALLOCATE_X_TABLE(X,OFFSET,OUTPUT) \
do { \
\
uint64_t Y = allocate_physical_block_address(); \
X[OFFSET] = Y | 7UL;\
uint64_t Y_ALIGN = Y >> 12 << 12 ; \
OUTPUT = (uint64_t*)Y_ALIGN; \
}while(0);

#define PAGE_TABLE_BIT_OFF 12
#define PAGE_DIR_BIT_OFF 21
#define PDP_BIT_OFF 30
#define PML4_TABLE_OFF 39

static uint64_t * kernel_pml4;
static uint64_t cr3_ADDR;

//FUNCTIONS
uint64_t * get_reference_to_kernel_pml4();
uint64_t* get_page_table_entry(uint64_t target_ADDR);
uint64_t* get_page_dir_table_entry(uint64_t target_ADDR);
uint64_t* get_pdpe_table_entry(uint64_t target_ADDR);
uint64_t* get_pml4_table_entry(uint64_t target_ADDR);
int check_if_physical_presence_bit_set(uint64_t ADDR);
void initialize_virtual_2_physical_maps(uint64_t phys_ADDR, uint64_t v_ADDR, uint64_t size_in_pages);
void enable_paging_support(uint64_t kernel_memory_ADDR, uint64_t physical_base_ADDR);
void destroy_Page_Table_hierarchy(uint64_t target_pml4_ADDR);
uint64_t create_new_pml4_entry();
void do_virtual_2_physical_mapping(uint64_t target_virt_ADDR, uint64_t target_phys_ADDR, uint64_t permissions);



#endif
