#ifndef __PHYSICAL_MEMORY_HELP_H__
#define __PHYSICAL_MEMORY_HELP_H__

#include <sys/help.h>
#include <sys/virtual_memory_help.h>

#include <sys/defs.h>

#define I_OR_D_REF_CNT_OF_ADDR(A,I) \
do {\
if(I)  \
addr_ref_cnt_arr[(A-base)/4096]+=1;\
else \
addr_ref_cnt_arr[(A-base)/4096]-=1;\
}while(0);

#define GET_REF_CNT_FOR_ADDR(A) (addr_ref_cnt_arr[(A-BASE)/4096])

static int * addr_ref_cnt_arr;
static int memory_map[][64];
static uint64_t base = 0x400000UL;
static uint64_t mem_size = 0x5ccb000UL;
static int max_blocks ;
static int used_blocks;
#define insert_in_memory_map(L)  \
memory_map[L/64][L%64] = 1;

#define delete_from_memory_map(L) \
memory_map[L/64][L%64] = 0;

void initialize_physical_memory_blocks( uint64_t physFREE);
static int get_first_free_addr_from_mem_map();
uint64_t allocate_physical_block_address();
void free_physical_block_of_memory(uint64_t physical_address,int markZero);


#endif
