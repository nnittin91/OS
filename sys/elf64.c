#include<sys/elf64.h>
#define MAX_ARGS 10
#define ARG_MAX_SZ 100

static char gbl_args_array[MAX_ARGS][ARG_MAX_SZ];

struct process_control_block* initiate_elf_process(char* binaryName,char* args[])
{

Elf64_Ehdr *  hdr =(Elf64_Ehdr*)(tarfs_Header*)tar_file_system_file_search(binaryName);
int isELFFile = is_elfFile(hdr);

if(is_elfFile> 0)
{
//allocate new PCB block
struct process_control_block* temp = initialize_UPCB();
return load_elf_headers(args,binaryName,temp,hdr);
}
return 0;
}

void copy_arg_to_global_arg_array(int *no_of_args, char* target[], char * binaryName)
{

								// first argument would be binary itself

								kernelStringCopy(gbl_args_array[0],binaryName);
								*no_of_args = *no_of_args+1;
								if(target == NULL)
																return;
								//until we end up with no argument in target array we need to increment no_of_args and push it to global array
								int i = 0;
								for(;target[i]!='\0';i++)
								{
																kernelStringCopy(gbl_args_array[i+1],target[i]);
																*no_of_args = *no_of_args+1;

								}
								return;
}

void push_args_to_user_stack(struct process_control_block * proc,int no_of_args)
{
//first push the arguments in the global array to stack as it is in decreasing order of index & then push their addresses and finally the number!
//Remember the stack grows in reverse direction
uint64_t cur_cr3;

if(!proc)
return;
//first get the user stack

__asm__ volatile ("movq  %%cr3 , %0;"     
																		:"=r"(cur_cr3)
																	);


//load current process address space

__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(proc->process_mm->pml4_phys_ADDR)
																);
//so now get user stack

uint64_t user_stack_ADDR = (proc->process_mm->start_address_stack);

int argNo = no_of_args;
uint64_t *temp[MAX_ARGS];
for(;argNo>0;argNo--)
{
user_stack_ADDR=(uint64_t*)((void*)user_stack_ADDR -(kernelStrlen(gbl_args_array[argNo-1]) +1));
memcpy((void*)user_stack_ADDR,(void*)gbl_args_array[argNo-1],(kernelStrlen(gbl_args_array[argNo-1]) +1));
temp[argNo-1] = user_stack_ADDR;
}

argNo = no_of_args;
for(;argNo>0;argNo--)
{
user_stack_ADDR =  user_stack_ADDR -1;
*user_stack_ADDR = (uint64_t)gbl_args_array[argNo-1];
}
//now store noofargs finally as top of stack

user_stack_ADDR --;

*user_stack_ADDR = (uint64_t)no_of_args;

// update stack and reload back old page tables

proc->process_mm->start_address_stack= user_stack_ADDR;
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(cur_cr3)
																);

}

struct process_control_block * load_elf_headers(char*args[], char*binaryName,process_control_block* current_process, Elf64_Ehdr* hdr)
{

if(current_process==NULL)
return NULL;

uint64_t cr3_val;

__asm__ volatile ("movq  %%cr3 , %0;"     
																		:"=r"(cr3_val)
																	);

kernelStringCopy(current_process->name,binaryName);

//need to iterate over the elf program headers that we will get from the header
struct vma_info_format temp;

struct vma_info_format * local_temp_node;
Elf64_Phdr * p_hdr = (Elf64_Phdr*)((void*)hdr+hdr->e_phoff);
uint64_t max;
int i =0;
for(;i<hdr->e_phnum;i++)
{
//we have to check the type of section in the loop and load the sections accordingly
if((int)p_hdr->p_type==0x1)//means TRUE, we can load... now check what kind of segment has to be set based on  type
{
temp.start_vm_ADDR = p_hdr->p_vaddr;
temp.end_vm_ADDR = temp.start_vm_ADDR + p_hdr->p_memsz;
if(p_hdr->p_flags == 0x5)
temp.vma_mem_segmentType =MAP_CODE;
else if(p_hdr->p_flags ==0x6) 
temp.vma_mem_segmentType =MAP_DATA;
else
temp.vma_mem_segmentType =-1;

temp.permissions = EXEC;
temp.vm_FD =0;
 
//use this to create a new  vma now
local_temp_node = create_new_vma_entry(temp);

//update current process vma related info
current_process->process_mm->size_of_process_vma_list++;
current_process->process_mm->total_virtual_mem_for_process+=p_hdr->p_memsz;

if(max<temp.end_vm_ADDR)
max=temp.end_vm_ADDR;

//now allocate v_memory for it using kernel mmap 
uint64_t permissions;
if(p_hdr->p_type == MAP_CODE)
permissions=0x5;
else
permissions = 0x7;
//mapping has to be current process' address space
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(current_process->process_mm->pml4_phys_ADDR)
																);

kernelMapping(temp.start_vm_ADDR, permissions, p_hdr->p_memsz);

//add new vma to the list maintained by current process
if(current_process->process_mm->process_vma_list ==   NULL)
current_process->process_mm->process_vma_list=local_temp_node;
else
{
struct vma_info_format* indexVma = current_process->process_mm->process_vma_list;

while(indexVma->next!=NULL)
{
indexVma=indexVma->next;
}
indexVma->next = local_temp_node;//inserted at the end of list
}
//now copy the contents to the allocated memory area

memcpy((void*)temp.start_vm_ADDR,(void*)hdr+p_hdr->p_offset,p_hdr->p_filesz);

//initialize to allow for lazy loading

memset((void*)temp.start_vm_ADDR,0,p_hdr->p_memsz-p_hdr->p_filesz);
//load back current page tables
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(cr3_val)
																);

}
p_hdr=p_hdr+1;
}
struct vma_info_format* tempVma=current_process->process_mm->process_vma_list;

while(tempVma->next!=NULL)
{
tempVma=tempVma->next;
}

temp.start_vm_ADDR = (((max-1)>>12)+1)<<12;
temp.end_vm_ADDR =temp.start_vm_ADDR;
temp.vma_mem_segmentType =MAP_HEAP;


temp.permissions = READ_WRITE;
temp.vm_FD =0;

tempVma->next=create_new_vma_entry(temp);

//update current process vma info
current_process->process_mm->size_of_process_vma_list ++;
current_process->process_mm->start_brk_heap_address = temp.start_vm_ADDR;
current_process->process_mm->end_brk_heap_address = temp.end_vm_ADDR;

//now we need to add stack segment
tempVma=current_process->process_mm->process_vma_list;

while(tempVma->next!=NULL)
{
tempVma=tempVma->next;
}

temp.start_vm_ADDR = 0xF000000000UL;
temp.end_vm_ADDR =0xEFFFFF0000UL;
temp.vma_mem_segmentType =MAP_STACK;
temp.permissions = READ_WRITE;
temp.vm_FD =0;

tempVma->next = create_new_vma_entry(temp);

//get local page tables again
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(current_process->process_mm->pml4_phys_ADDR)
																);

kernelMapping(temp.end_vm_ADDR - 4096, 0x7,4096);

__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(cr3_val)
																);


//update current process structures
current_process->process_mm->size_of_process_vma_list ++;

//we get the total stack virtual memory available
current_process->process_mm->total_stack_virtual_mm_for_process = 0x10000;

current_process->process_mm->total_virtual_mem_for_process+=0x10000;

//update the  top of stack
//since one page is alloacted so it is 1 PAGE_SIZE less than allocated

current_process->process_mm->start_address_stack=temp.end_vm_ADDR -0x8;

int no_of_arguments = 0 ;

copy_arg_to_global_arg_array(&no_of_arguments,args,binaryName);
push_args_to_user_stack(current_process,no_of_arguments);

//initiate_run_process(current_process,hdr->e_entry, current_process->process_mm->start_address_stack);
//setup kernel stack   -- first data segment then stack address, flags, code segment,then address of task function, then leave 15 locations for popped registers of new process, then address of
//POPA instruction of timer interrupt    
current_process->stack[0xFFFF] =0x23;
current_process->stack[0xFFFE] =current_process->process_mm->start_address_stack;
current_process->stack[0xFFFD] = 0x200202UL;
current_process->stack[0xFFFC]= 0x1B;
current_process->stack[0xFFFB]= (uint64_t)hdr->e_entry;
current_process->stack[0xFFDF] = (uint64_t)int0 + 0x20;

current_process->rip = (uint64_t)hdr->e_entry;;
current_process->rsp = (uint64_t)current_process->stack[0xFFDE];
push_into_process_ready_LL(current_process);


//done loading elf process
 return current_process;


}


