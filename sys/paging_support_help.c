#include<sys/paging_support_help.h>

uint64_t * get_reference_to_kernel_pml4()
{
return kernel_pml4;
}

uint64_t* get_page_table_entry(uint64_t target_ADDR)
{
return (uint64_t*)((target_ADDR <<16 >> 28 << 3) | 0xFFFFFF0000000000UL);// add the self reference offset to the desired pte offset derived from the target address
}
uint64_t* get_page_dir_table_entry(uint64_t target_ADDR)
{
return (uint64_t*)((target_ADDR <<16 >> 37 << 3) | 0xFFFFFF7F80000000UL);// add the self reference offset to the desired pde offset derived from the target address
}
uint64_t* get_pdpe_table_entry(uint64_t target_ADDR)
{
return (uint64_t*)((target_ADDR <<16 >> 46 << 3) | 0xFFFFFF7FBFC00000UL);// add the self reference offset to the desired pdpe offset derived from the target address
}
uint64_t* get_pml4_table_entry(uint64_t target_ADDR)
{
return (uint64_t*)((target_ADDR <<16 >> 55 << 3) |0xFFFFFF7FBFDFE000UL );// add the self reference offset to the desired pdpe offset derived from the target address
}


int check_if_physical_presence_bit_set(uint64_t ADDR)
{
if((ADDR & 0x1) == 0x1)
return 1;
else
return 0;
} 
 
void initialize_virtual_2_physical_maps(uint64_t phys_ADDR, uint64_t v_ADDR, uint64_t size_in_pages)
{
uint64_t * target_PDE_table = 0, *target_PDPE_table = 0, *target_PTE_table =0;
uint64_t x_pte_off,x_pde_off,x_pdpe_off,x_pml4_off;

uint64_t mask = 0x1FF;

x_pte_off = (v_ADDR >> PAGE_TABLE_BIT_OFF) & mask;
x_pde_off = (v_ADDR >> PAGE_DIR_BIT_OFF) & mask;
x_pdpe_off = (v_ADDR >> PDP_BIT_OFF) & mask;
x_pml4_off = (v_ADDR >> PML4_TABLE_OFF) & mask;

uint64_t local_physical_ADDR = (uint64_t) *(kernel_pml4 + x_pml4_off);

int pml4_exists = check_if_physical_presence_bit_set(local_physical_ADDR);
if(pml4_exists)
{
target_PDPE_table = (uint64_t*) (0xFFFFFFFF80000000UL + local_physical_ADDR);
local_physical_ADDR = (uint64_t) *(target_PDPE_table + x_pdpe_off);
if(check_if_physical_presence_bit_set(local_physical_ADDR))
{
target_PDE_table = (uint64_t*) (0xFFFFFFFF80000000UL + local_physical_ADDR);
local_physical_ADDR = (uint64_t) *(target_PDE_table + x_pde_off);
if(check_if_physical_presence_bit_set(local_physical_ADDR))
{
target_PTE_table = (uint64_t*) (0xFFFFFFFF80000000UL + local_physical_ADDR);

}
else
{
ALLOCATE_X_TABLE(target_PDE_table,x_pde_off,target_PTE_table);
}
}
else
{
ALLOCATE_X_TABLE(target_PDPE_table,x_pdpe_off,target_PDE_table);
ALLOCATE_X_TABLE(target_PDE_table,x_pde_off,target_PTE_table);
}
}
else
{
ALLOCATE_X_TABLE(kernel_pml4,x_pml4_off,target_PDPE_table);
ALLOCATE_X_TABLE(target_PDPE_table,x_pdpe_off,target_PDE_table);
ALLOCATE_X_TABLE(target_PDE_table,x_pde_off,target_PTE_table);
}

local_physical_ADDR = phys_ADDR;

//if current pte entry can hold the required no of pages .... best case
// max entries .... 2^ 9 === 512

int i =x_pte_off;

int bestCase = ((x_pte_off + size)<= 512);

if(bestCase)
{
for(;i<(x_pte_off + size);i++)
{
target_PTE_table[i] = (local_physical_ADDR | 0x3UL);
local_physical_ADDR = local_physical_ADDR + 4096;
}
}
else
{
//required no of pages is greater than what can fit in a single page table entry so need to allocate more pagetable entry as per use
int rem_pages = 512;
i= x_pte_off;
for(;i<512;i++)
{
target_PTE_table[i] = (local_physical_ADDR | 0x3UL);
local_physical_ADDR = local_physical_ADDR + 4096;
}
rem_pages = rem_pages - (512 - x_pte_off);
// calculate how many page table entries are required
int no_of_page_table_entries_reqd = rem_pages/512; // may be some remainder is still left

int j = 1;
for(;j<=no_of_page_table_entries_reqd;j++)
{
ALLOCATE_X_TABLE(target_PDE_table, x_pde_off + j, target_PTE_table);
i = 0;
for(;i<512;i++)
{
target_PTE_table[i] = (local_physical_ADDR | 0x3UL);
local_physical_ADDR = local_physical_ADDR + 4096;
}

}

// for remainder of pages still left allocate a single page table entry
// find the remaining pages
//rem_pages = rem_pages - (512 * x_pte_off); // buggy(modifying .... check LATER!!)
rem_pages = rem_pages - (512 * no_of_page_table_entries_reqd); // buggy(modifying .... check LATER!!)
ALLOCATE_X_TABLE(target_PDE_table, x_pde_off + j, target_PTE_table);
i = 0;
for(;i<rem_pages;i++)
{
target_PTE_table[i] = (local_physical_ADDR | 0x3UL);
local_physical_ADDR = local_physical_ADDR + 4096;

}

}


}

void enable_paging_support(uint64_t kernel_memory_ADDR, uint64_t physical_base_ADDR)
{
// for paging we need cr3 address .... allocate it first.... 
// then we need to initialize our page tables for kernel
// add a mapping to enable kprintf vga memory access
// post this function every address accessed would be treated as a virtual addr

cr3_ADDR = allocate_physical_block_address();
// kernel_pml4_addr has to be assigned to cr3 value and self referencing technique mentioned on OSDev used to assign this value to 510th entry.
kernel_pml4 = (uint64_t *) (0xFFFFFFFF80000000UL + cr3_ADDR);
*(kernel_pml4 + 510) = (cr3_ADDR | 0x3UL);

//initialize ---- setup 518 pages of kernel virtual to physical maps for use

initialize_virtual_2_physical_maps(kernel_memory_ADDR,physical_base_ADDR,518);

//provide mapping for vga buffer
initialize_virtual_2_physical_maps(/*v_addr*/ 0xFFFFFFFF800B8000,/*p_addr*/0xB8000,1);
__asm__ volatile   ("movq %0,%%cr3;"
																				:
																				:"r" (cr3_ADDR));

//upper bound on virtual pages

set_highestVirtual_ADDR(kernel_memory_ADDR + 4096*518);
initialize_local_kernel_malloc();
}


void destroy_Page_Table_hierarchy(uint64_t target_pml4_ADDR)
{
uint64_t pde_entry_index = 0, pte_entry_index =0, pdpe_entry_index =0,pml4_entry_index =0;
uint64_t * target_PDE_ADDR, * target_PTE_ADDR, *target_PDPE_ADDR, *target_PML4_ADDR;

//starting address of all tables is the self_ref_address

while(pml4_entry_index < 510)
{
				target_PML4_ADDR = (uint64_t *)(0xFFFFFF7FBFDFE000UL | pml4_entry_index << 3);
				int check1 = check_if_physical_presence_bit_set(*target_PML4_ADDR);
				if(check1)
				{
					while(pdpe_entry_index < 512)
				{
					target_PDPE_ADDR= (uint64_t *)(0xFFFFFF7FBFC00000UL | pml4_entry_index <<PAGE_TABLE_BIT_OFF | pdpe_entry_index << 3);
					int check2 = check_if_physical_presence_bit_set(*target_PDPE_ADDR);
					if(check2)
					{
						while(pde_entry_index <  512)
						{
							target_PDE_ADDR = (uint64_t*)(0xFFFFFF7F80000000UL | pml4_entry_index << PAGE_DIR_BIT_OFF | pdpe_entry_index <<PAGE_TABLE_BIT_OFF | pde_entry_index << 3);
							int check3 = check_if_physical_presence_bit_set(*target_PDE_ADDR);
							if(check3)
							{
								while(pte_entry_index <  512)
								{
									target_PTE_ADDR = (uint64_t*)(0xFFFFFF0000000000UL | pml4_entry_index << PDP_BIT_OFF | pdpe_entry_index <<PAGE_DIR_BIT_OFF | pde_entry_index <<PAGE_TABLE_BIT_OFF|pte_entry_index<< 3);
									int check4 = check_if_physical_presence_bit_set(*target_PTE_ADDR);
									if(check4)
									{
										set_highestVirtual_ADDR(get_highestVirtual_ADDR() +  4096); // removing one page
										uint64_t local_pte_ADDR = (*target_PTE_ADDR & 0x000FFFFFFFFFF000UL);
										free_physical_block_of_memory(local_pte_ADDR,1);
										*target_PTE_ADDR = 0x0;
									}
								pte_entry_index ++;
								}
								uint64_t local_pde_addr = (*target_PDE_ADDR & 0x000FFFFFFFFFF000UL);
								free_physical_block_of_memory(local_pde_addr,1);
								*target_PDE_ADDR = 0x0; 
									}
							pde_entry_index ++;
								}
								uint64_t local_pdpe_addr = (*target_PDPE_ADDR & 0x000FFFFFFFFFF000UL);
								free_physical_block_of_memory(local_pdpe_addr,1);
								*target_PDPE_ADDR = 0x0; 
									}
							pdpe_entry_index ++;
								}
								uint64_t local_pml4_addr = (*target_PML4_ADDR & 0x000FFFFFFFFFF000UL);
								free_physical_block_of_memory(local_pml4_addr,1);
								*target_PML4_ADDR = 0x0; 
									}
							pml4_entry_index ++;
								}
__asm__ volatile   ("movq %0,%%cr3;"
																				:
																				:"r" (cr3_ADDR));
			
}	

uint64_t create_new_pml4_entry()
{
//create a  new entry and store old table as a reference into some location say 511
uint64_t temp_physical_address, *target_pml4_entry;

//move the virtual space by 1     page size
set_highestVirtual_ADDR(get_highestVirtual_ADDR() + 4096);

temp_physical_address = allocate_physical_block_address();

do_virtual_2_physical_mapping(get_highestVirtual_ADDR,temp_physical_address,0x3UL);
target_pml4_entry = (uint64_t*)get_highestVirtual_ADDR();

target_pml4_entry[510] = temp_physical_address|0x3UL;
target_pml4_entry[511] = kernel_pml4[511];

return temp_physical_address;;

}

void do_virtual_2_physical_mapping(uint64_t target_virt_ADDR, uint64_t target_phys_ADDR, uint64_t permissions)
{

								//logic is similar to initialization of v2p mapping... just that now if entries are  present then just initialize its contents to 0 and if 
								//not present then make entry

								uint64_t* target_pml4_entry = get_pml4_table_entry(target_virt_ADDR);
								uint64_t* target_pdpe_entry = get_pml4_table_entry(target_virt_ADDR);
								uint64_t* target_pde_entry = get_pml4_table_entry(target_virt_ADDR);
								uint64_t* target_pte_entry = get_pml4_table_entry(target_virt_ADDR);
								if(check_if_physical_presence_bit_set(*target_pml4_entry))
								{
																if(check_if_physical_presence_bit_set(*target_pdpe_entry))
																{
																								if(check_if_physical_presence_bit_set(*target_pde_entry))
																								{
																																if(check_if_physical_presence_bit_set(*target_pte_entry))
																																{
																																								free_physical_block_of_memory(target_phys_ADDR,0);	
																																}
																																else
																																								*target_pte_entry = target_phys_ADDR | permissions;
																								}
																								else
																								{
																																// pde entry needs to be created along with pte entry addition
																																*target_pde_entry = allocate_physical_block_address() |  0x3UL;
																																*target_pte_entry = target_phys_ADDR | permissions;
																								}


																}
																else
																{
																								//pdpe entry onwards need to be created and added
																								*target_pdpe_entry = allocate_physical_block_address() | 0x3UL;
																								*target_pde_entry = allocate_physical_block_address() | 0x3UL;
																								*target_pte_entry = target_phys_ADDR | permissions;
																}


								}
								else//Last else
								{
																// all 4 needs to be created 
																*target_pml4_entry = allocate_physical_block_address() | 0x3UL; 
																*target_pdpe_entry = allocate_physical_block_address() | 0x3UL;
																*target_pde_entry = allocate_physical_block_address() | 0x3UL;
																*target_pte_entry = target_phys_ADDR | permissions;
								}
}
