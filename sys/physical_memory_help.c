
#include <sys/physical_memory_help.h>


void initialize_physical_memory_blocks( uint64_t physFREE)
{
max_blocks = mem_size/4096;
used_blocks = 0;
memset(base,0x0,mem_size/8);
memory_map = (int**)(KERNEL_START_VIRTUAL_ADDRESS + physFREE);

int i =0,j=0;
for(i=0;i<max_blocks/64;i++)
{
for(j=0;j<64;j++)
{
memory_map[i][j]=0;

}
}
addr_ref_cnt_arr = (int*)(memory_map + sizeof(memory_map) + 1);
for(i=0;i<max_blocks;i++)
{
addr_ref_cnt_arr[i] = 0;
}

}



static int get_first_free_addr_from_mem_map()
{
								int i=0,j=0;

								for(i=0;i<(max_blocks/64);i++)
								{
																for(j=0;j<64;j++)
																{
																								if(memory_map[i][j] != 1)
																																return (64*i + j);
																}
								}
}

uint64_t allocate_physical_block_address()
{
if((max_blocks - used_blocks)<=2)
return NULL;//memory exhausted

int frame = get_first_free_addr_from_mem_map();
if(frame == -1)
return NULL;//no free memory

insert_in_memory_map(frame);
used_blocks = used_blocks + 1;
//uint64_t retAddr = base + frame<<12;
addr_ref_cnt_arr[frame] = addr_ref_cnt_arr[frame] + 1;
return (base + frame<<12);
} 

void free_physical_block_of_memory(uint64_t physical_address,int markZero)
{
I_OR_D_REF_CNT_OF_ADDR(physical_address,0);
if(markZero && (addr_ref_cnt_arr[(physical_address-base)/4096] == 0))
{
memset((void*)get_virtual_for_given_physical_ADDR(physical_address),0,4096);
*get_page_table_entry(get_virtual_for_given_physical_ADDR(physical_address)) = 0x0UL;
}
else
kprintf("Memory freed but not zeroed\n");
delete_from_memory_map((physical_address-base)/4096);
used_blocks = used_blocks - 1;
}

