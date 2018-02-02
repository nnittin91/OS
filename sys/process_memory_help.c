#include <sys/process_memory_help.h>

//if switch to is written as a function then the compiler is allowed to do anything with the stack and it is difficult to control the offsets so writing it in pure assembly
#define SWITCH_TO_MACRO(prev,next)   \
/* push all gerneral purpose registers for the caller*/\
__asm__ volatile (\
								"pushq %%rax\n\t"\
								"pushq %%rcx\n\t"\
								"pushq %%rdx\n\t"\
								"pushq %%rbx\n\t"\
/* push the flags using command pushfq*/\
								"pushfq\n\t"\
/* need to push stack pointer stored in process structure as well as instruction pointer*/\
								"pushq %%rbp\n\t"\
								"pushq %[prev_proc]\n\t"\
								"movq %%rsp, %[prev_stack]\n\t"\
								"movq %[next_stack], %%rsp\n\t"\
								"movq $current_proc_exec, %[prev_instr]\n\t"\
								"pushq %[next_instr]\n\t"\
/* if a function without any body and parameters is called then calling return on such a function would fill for us the current process' instruction address*/\
								"jmp next_I\n\t"\
					"current_proc_exec:\t"\
					/*current process will just pop back all its contents if it gets scheduled again*/\
								"popq %[previous_saved_proc]\n\t"\
								"popq %%rbp\n\t"\
								"popfq\n\t"\
								"popq %%rbx\n\t"\
								"popq %%rdx\n\t"\
								"popq %%rcx\n\t"\
								"popq %%rax\n\t"\
							:[prev_stack] "=m" (prev->rsp),\
								[prev_instr] "=m" (prev->rip),\
								[previous_saved_proc] "=m" (prev)\
							:[next_stack] "m" (next->stack),\
							[next_instr] "m" (next->ip),\
							[prev_proc]  "m" (prev) \
							); 

void next_I()
{
} 

uint32_t get_next_available_pid()
{
								return next_available_pid;
}

void set_next_available_pid(uint32_t targetPID)
{
								next_available_pid = targetPID;
}

//maintaining A FIFO struct for lists 
static void move_process_LL_forward(struct process_control_block* ptr)
{
								ptr=ptr->next;
}
static void move_vma_LL_forward(struct vma_info_format* ptr)
{
								ptr=ptr->next;
}

static struct process_control_block* get_front_process()
{
								if(process_free_LL)
								{
																return process_free_LL;
																//process_free_LL = process_free_LL->next
								}
								else
																return NULL;
}

static struct vma_info_format * get_front_vma()
{
								if(available_global_free_vma_LL)
																return available_global_free_vma_LL;
								else
																return NULL;

}

void push_into_process_LL(struct process_control_block* p)
{
								if(p!=NULL)
								{
																p->next = process_free_LL;
																//update front 
																process_free_LL = p;
								}
								return;
}

void push_into_vma_LL(struct vma_info_format* p)
{
								if(p!=NULL)
								{
																p->next = available_global_free_vma_LL;
																available_global_free_vma_LL = p;
								}
								else
																return;
}

struct process_control_block* initialize_KPCB()
{
								struct process_control_block * newPCB = get_front_process();

								if(!newPCB)
								{
																//need to add a new one.
																newPCB = (struct process_control_block*)kmalloc(sizeof(struct process_control_block));

																newPCB->parent_task = NULL; newPCB->next = NULL; newPCB->prev = NULL; newPCB->sameParentTasks = NULL; newPCB->childRoot = NULL;
																newPCB->number_of_child_tasks = 0;
																newPCB->PID_of_died_Chld =0;

																// initialize memory to main memory space for the process

																struct main_process_memory_format* new_MP_mem_fmt = (struct main_process_memory_format*)kmalloc(sizeof(struct main_process_memory_format));
																new_MP_mem_fmt -> pml4_phys_ADDR = create_new_pml4_entry();
																new_MP_mem_fmt->process_vma_list = NULL;
																new_MP_mem_fmt->size_of_process_vma_list = 0;
																new_MP_mem_fmt->total_virtual_mem_for_process = 0;
																new_MP_mem_fmt->total_stack_virtual_mm_for_process = 0;
																new_MP_mem_fmt->start_address_stack = 0;
																new_MP_mem_fmt->start_brk_heap_address = 0;
																new_MP_mem_fmt->end_brk_heap_address = 0;

																newPCB->process_mm = new_MP_mem_fmt;
																memset((void*)newPCB->name,0,50);
																memset((void*)newPCB->working_dir,0,300);
																memset((void*)newPCB->file_descriptors,0,15);
																memset((void*)newPCB->stack,0,512);
								}
								else
																move_process_LL_forward(process_free_LL);

								newPCB->PID = get_next_available_pid();
								next_available_pid++;
								newPCB->ParentPID = 0;
								newPCB->processType =KERNEL_PROC;
								newPCB->current_status = READY;

								return newPCB;  
}


struct process_control_block* initialize_UPCB()
{
								struct process_control_block * newPCB = get_front_process();

								if(!newPCB)
								{
																//need to add a new one.
																newPCB = (struct process_control_block*)kmalloc(sizeof(struct process_control_block));

																newPCB->parent_task = NULL; newPCB->next = NULL; newPCB->prev = NULL; newPCB->sameParentTasks = NULL; newPCB->childRoot = NULL;
																newPCB->number_of_child_tasks = 0;
																newPCB->PID_of_died_Chld =0;

																// initialize memory to main memory space for the process

																struct main_process_memory_format* new_MP_mem_fmt = (struct main_process_memory_format*)kmalloc(sizeof(struct main_process_memory_format));
																new_MP_mem_fmt -> pml4_phys_ADDR = create_new_pml4_entry();
																new_MP_mem_fmt->process_vma_list = NULL;
																new_MP_mem_fmt->size_of_process_vma_list = 0;
																new_MP_mem_fmt->total_virtual_mem_for_process = 0;
																new_MP_mem_fmt->total_stack_virtual_mm_for_process = 0;
																new_MP_mem_fmt->start_address_stack = 0;
																new_MP_mem_fmt->start_brk_heap_address = 0;
																new_MP_mem_fmt->end_brk_heap_address = 0;

																newPCB->process_mm = new_MP_mem_fmt;
																memset((void*)newPCB->name,0,50);
																memset((void*)newPCB->working_dir,0,300);
																memset((void*)newPCB->file_descriptors,0,15);
																memset((void*)newPCB->stack,0,512);
								}
								else
																move_process_LL_forward(process_free_LL);

								newPCB->PID = get_next_available_pid();
								next_available_pid++;
								newPCB->ParentPID = 0;
								newPCB->processType =USER_PROC;
								newPCB->current_status = READY;

								return newPCB;  
}

void destroy_PCB(struct process_control_block* p)
{
								if(!p)
																return;

								p->next=NULL;p->prev=NULL;p->parent_task=NULL;p->sameParentTasks=NULL;p->childRoot=NULL;p->number_of_child_tasks=0;
								p->PID_of_died_Chld=0;
								memset((void*)p->name,0,50);
								memset((void*)p->working_dir,0,300);
								memset((void*)p->file_descriptors,0,15);
								memset((void*)p->stack,0,512);

								struct main_process_memory_format * temp = p->process_mm;

								del_all_VMA_LL(temp->process_vma_list);
								temp->size_of_process_vma_list = 0;
								temp->total_virtual_mem_for_process = 0;
								temp->total_stack_virtual_mm_for_process=0;
								temp->start_address_stack=0;temp->start_brk_heap_address=0;temp->end_brk_heap_address=0;
								delete_pml4_entries(temp->pml4_phys_ADDR);


}

void del_all_VMA_LL(struct vma_info_format* p)
{
								struct vma_info_format *temp=p,*prev=0;
								if(p==NULL)
																return;
								while(temp!=NULL)
								{
																temp->associated_mm = NULL;
																temp->permissions = 0;
																temp->start_vm_ADDR = 0;
																temp->end_vm_ADDR = 0;
																temp->vma_mem_segmentType = -1;
																temp->vm_FD = 0;
																prev = temp;
																temp = temp->next; 
								}
								if(prev != NULL)
								{
																//add it to free list of vmas
																prev->next = available_global_free_vma_LL;
																//all free vms added
																available_global_free_vma_LL=p; 

								}
								return;
}

void copy_PCB_into_child(struct process_control_block* p1, struct process_control_block* p2)
{

								if(p1 == NULL || p2 == NULL)
																return;

								struct process_control_block* pp1 = p1->parent_task;
								struct process_control_block * chldRoot = p1->childRoot;
								struct process_control_block * prev_chld =NULL;

								while(chldRoot)
								{

																{
																								if(chldRoot == p1)
																																break;

																}
																prev_chld = chldRoot;
																chldRoot= chldRoot->sameParentTasks;
								}
								if(!chldRoot) return ;

								if(prev_chld)
								{
																prev_chld->sameParentTasks = chldRoot->sameParentTasks;;
																pp1->childRoot =p2;
								}
								else
								{
																p2->sameParentTasks = chldRoot->sameParentTasks;
																pp1->childRoot = p2;
								}

}

void * kernelMapping(uint64_t startingADDR, uint64_t permissions, uint64_t sz)
{
								uint64_t saved_virt_ADDR = get_highestVirtual_ADDR();_
																set_highestVirtual_ADDR((startingADDR >> 12 << 12));
								uint64_t endingADDR = startingADDR + sz;
								uint64_t number_of_pages_till_end = (endingADDR - 1)/4096;
								uint64_t number_of_pages_till_start = (startingADDR)/4096;
								uint64_t number_of_pages_reqd = number_of_pages_till_end - number_of_pages_till_start + 1;
								allocate_n_virtual_pages_memory(number_of_pages_reqd,permissions);
								//restore the highest virtual address
								set_highestVirtual_ADDR(saved_virt_ADDR);

								return (void*)startingADDR;

}

struct vma_info_format * map_given_address_into_vma(uint64_t targetADDR, uint64_t permissions,uint64_t fd, uint64_t size, uint64_t vma_type)
{
								struct vma_info_format temp;

								temp.start_vm_ADDR = targetADDR;
								temp.end_vm_ADDR = targetADDR + size;
								temp.permissions = permissions;
								temp.vm_FD = fd;
								temp.vma_mem_segmentType = vma_type;
								struct vma_info_format * retVMA =  create_new_vma_entry(temp);

								run_task->process_mm->size_of_process_vma_list +=1;
								run_task->process_mm->total_virtual_mem_for_process +=size;
								struct vma_info_format* ptr_vmas = running_active_task->process_mm->process_vma_list;
								struct vma_info_format * prev=NULL;
								uint64_t end_ADDR = targetADDR + size;
								int ind =  0;
								while(ptr_vmas->next!=NULL)
								{
																prev = ptr_vmas;
																ptr_vmas=ptr_vmas->next;
																if(prev->end_vm_ADDR < targetADDR && ((end_ADDR < ptr_vmas->start_vm_ADDR)))
																{
																								ind = 1;
																								break;
																}
								}
								if(ind)
								{
																prev->next = retVMA;
																retVMA->next =ptr_vmas;
								}
								else
																ptr_vmas->next = retVMA;

								return retVMA;
}

int is_given_address_mapped_into_vma(struct process_control_block * p1, uint64_t target_ADDR, uint64_t size)
{
								struct main_process_memory_format * local_mm = p1->process_mm;
								struct vma_info_format * temp = local_mm->process_vma_list;

								while(temp!=NULL)
								{
																int case1 = (target_ADDR <temp->start_vm_ADDR && temp->start_vm_ADDR<(size+target_ADDR));
																int case2 =  (target_ADDR <temp->end_vm_ADDR && temp->end_vm_ADDR<(size+target_ADDR));
																if(case1 ||case2)
																								return 0;

																temp=temp->next;

								}
								int case3 = (target_ADDR>0xFFFFFFFF80000000UL);
								if(case3)
																return 0;
								else
																return 1;
} 


void increment_heap_via_brk(struct process_control_block* p, uint64_t size)
{
								if(p==NULL)
																return;
								struct main_process_memory_format * local_mm = p->process_mm;
								struct vma_info_format * temp = local_mm->process_vma_list;
								int flag =0;
								while(temp!=NULL)
								{
																if(temp->vma_mem_segmentType == MAP_HEAP)
																{
																								flag =1;
																								break;
																}
																temp=temp->next;
								}
								if(flag)
								{
																temp->end_vm_ADDR = temp->end_vm_ADDR + size;
																local_mm->total_virtual_mem_for_process = temp->total_virtual_mem_for_process + size;
																local_mm->end_brk_heap_address = temp->end_brk_heap_address + size;
								}
								return;
}

struct vma_info_format * create_new_vma_entry(struct vma_info_format target)
{
								struct vma_info_format * temp= get_front_vma();
								if(temp==NULL)
								{
																temp = (struct vma_info_format*)kmalloc(sizeof(struct vma_info_format));
								}
								else
																move_vma_LL_forward(available_global_free_vma_LL);

								temp->start_vm_ADDR = target.start_vm_ADDR;
								temp->end_vm_ADDR = target.end_vm_ADDR;
								temp->vm_FD = target.vm_FD;
								temp->vma_mem_segmentType = target.vma_mem_segmentType;
								temp->permissions = target.permissions;
								temp->associated_mm = NULL;
								temp->next = NULL;

}

void first_dummy_kernel_process()
{
while(1);

}

void create_first_kernel_process()
{
//allocate memory for first kernel process and set its  state and name

first_k_proc  = initialize_KPCB();

if(first_k_proc==NULL)
return;

kernelStringCopy(first_k_proc->name,"FIRST_PROC");
kernelStringCopy(first_k_proc->working_dir,"/");//root directory
first_k_proc->current_status = FIRST_KERNAL_PROC;
//setup kernel stack   -- first data segment then stack address, flags, code segment,then address of task function, then leave 15 locations for popped registers of new process, then address of
//POPA instruction of timer interrupt    
first_k_proc->stack[0xFFFF] =0x10;
first_k_proc->stack[0xFFFE] =first_k_proc->stack[0xFFFF];
first_k_proc->stack[0xFFFD] = 0x200202UL;
first_k_proc->stack[0xFFFC]= 0x08;
first_k_proc->stack[0xFFFB]= (uint64_t)create_first_kernel_process;
first_k_proc->stack[0xFFDF] = (uint64_t)int0 + 0x20;

first_k_proc->rip = (uint64_t)create_first_kernel_process;
first_k_proc->rsp = (uint64_t)first_k_proc->stack[0xFFDE];
push_into_process_ready_LL(first_k_proc);
}

void create_kernel_process_with_FUNC(uint64_t func_ADDR)
{
struct process_control_block* k_proc = initialize_KPCB();
if(k_proc==NULL)
return;

kernelStringCopy(k_proc->name,"K_PROC");
kernelStringCopy(k_proc->working_dir,"/");//root directory
k_proc->current_status = RUN;
k_proc->rip = (uint64_t)func_ADDR;
k_proc->rsp = (uint64_t)k_proc->stack[0xFFFF];
k_proc->next =NULL;
k_proc->prev = NULL;
push_into_process_ready_LL(k_proc);

}

void push_into_process_ready_LL(struct process_control_block* p)
{

//need to check process current status
//1)EXIT  -- means  add tofree list and return
//2)FIRST PROC --- return ---need not insert it 
//3)RUNNING -- means active --now will become ready and will be added to back of list
if(!p)
return;

if(p->status == FIRST_KERNAL_PROC)
return;
if(p->status == EXIT)
{
push_into_process_LL(p);
return;
} 
if(p->status == RUN)
{
p->status = READY;
}

struct process_control_block* local_LL = process_ready_LL;
//push it to end of list always

if(local_LL == NULL)
local_LL = p;
else if(local_LL->next !=NULL)
{
while(local_LL->next!=NULL)
{
local_LL=local_LL->next;
}
local_LL->next =p;
p->next = NULL;
}
return;
}

struct process_control_block* get_next_ready_process_from_LL()
{
//basically maintain prev and next pointers in LL while traversing and search for a  task in the list which has status as READY

struct process_control_block * prev = NULL, *next=NULL, *local_temp = process_ready_LL;
if(local_temp==NULL)
{
local_temp=first_k_proc;
next=local_temp;
return next;
}

while(local_temp!=NULL)
{
if(local_temp->status == READY)
{
next = local_temp;
next->status = RUN;
break;
}
prev = local_temp;
local_temp=local_temp->next'
}

//now need to check if last = NULL or not

if(last!=NULL)
{
last->next=local_temp->next;
//put local temp to front of list
local_temp->next = process_ready_LL;
}
else
{
process_ready_LL = process_ready_LL->next;
}

return next;
}

//need a bunch of arguments to copy stack address space from parent to  child tasks

void copyStackFromParent2Child(uint64_t * parent_cr3,uint64_t* page_table_entry,uint64_t* virtual_ADDR,uint64_t* physical_ADDR,
uint64_t* child_cr3,uint64_t* startingADDR,uint64_t* endingADDR)
{

uint64_t temp_ADDR = get_highestVirtual_ADDR();

*virtual_ADDR = (*virtual_ADDR >>12 <<12) - 4096;

//copy all the pages from start to end
while(*virtual_ADDR >= *startingADDR)
{
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(parent_cr3)
																);
//validate its presence ... in case not present ...oops a problem

*page_table_entry = get_page_table_entry(*virtual_ADDR);
int check1 = check_if_physical_presence_bit_set(*page_table_entry);

if(check1==0)
break;

*physical_ADDR = allocate_physical_block_address();  
do_virtual_2_physical_mapping(temp_ADDR,*physical_ADDR,0x7UL);

// copy this page in kernel space
memcpy((void*)temp_ADDR,(void*)virtual_ADDR,4096);

//now change the address space to that of child
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(child_cr3)
																);
do_virtual_2_physical_mapping(*virtual_ADDR,*physical_ADDR,0x7UL);

//release the temp  virtual entry
*get_page_table_entry(temp_ADDR) = 0x0;

//iterate further
*virtual_ADDR = *virtual_ADDR - 4096;

}
}

void copyParentInfo2ChildForFork(struct process_control_block* c, struct process_control_block* p, uint64_t *child_cr3)
{
//copy memory structures and  other  resources
memcpy((void*)c->process_mm,(void*)p->process_mm, sizeof(main_process_memory_format));

c->mm->process_vma_list = NULL;
c->mm->pml4_phys_ADDR = *child_cr3;
c->ParentPID = p->PID;
kernelStringCopy(c->name,p->name);
kernelStringCopy(c->working_Dir,p->working_dir);

//now we only have  filedescriptors

int i =0;
for(i=0;i<15;i++)
{
if(p->file_descriptors[i] !=NULL)
{
//need to copy
struct file_D * f  = (struct file_D*)kmalloc(sizeof(struct file_D));
f->currentPtr = p->file_descriptors[i]->currentPtr;
f->associated_file = p->file_descriptors[i]->associated_file;
f->permissions = p->file_descriptors[i]->permissions;
c->file_descriptors[i]= (uint64_t*)f;
}
else
continue;
}
}

void mainCopyFunctionForFork(struct process_control_block* p)
{
if(!p)
return;

//this is a user process ... we are not forking in kernel space...
//so first allocate a child  process then we will proceed with   copy basic contents from parent before copying the VMAs

struct process_control_block* child_pp =initialize_UPCB();

//copy basic contents
uint64_t child_cr3 = child_pp->process_mm->pml4_phys_ADDR;
copyParentInfo2ChildForFork(child_pp,p,&child_cr3);

if(p->childRoot !=NULL)
{
child_pp->sameParentTasks = p->childRoot;
}

p->childRoot= child_pp;
p->number_of_child_tasks+=1;

//now need to take care of VMAs
//STack only needs to be copied and a given a new space as it is having locals whereas other vmas can be shared and can be allocated on demand on write(COW).

//iterate over parent vma list ... search if type is stack ...call for copy

struct vma_info_format * local_vma_LL=p->process_mm->process_vma_list;
struct vma_info_format * local_c_vma_LL=child_pp->process_mm->process_vma_list;
while(local_vma_LL!=NULL)
{
struct vma_info_format temp;
uint64_t local_temp_phy_ADDR, local_temp_vir_ADDR;

temp.start_vm_ADDR = local_vma_LL->start_vm_ADDR;
temp.end_vm_ADDR = local_vma_LL->end_vm_ADDR;
temp.permissions = local_vma_LL->permissions;
temp.vma_FD = local_vma_LL->vm_FD;
temp.vma_mem_segmentType = local_vma_LL->vma_mem_segmentType;

if(child_pp->process_mm->process_vm_list == NULL)//means no vma
{
child_pp->process_mm->process_vma_list=create_new_vma_entry(temp);
local_c_vma_LL = child_pp->process_mm->process_vma_list;

}
else
{
local_c_vma_LL->next = create_new_vma_entry(temp);
local_c_vma_LL = local_c_vma_LL->next;
}
if(local_vma_LL->vma_mem_segmentType==MAP_STACK)
{
copyStackFromParent2Child(p->process_mm->pml4_phys_ADDR,NULL,&local_temp_vir_ADDR,&local_temp_phy_ADDR,&child_cr3,local_vma_LL->start_vm_ADDR,local_vma_LL->end_vm_ADDR);
}
else 
{
//just need to share physical pages
//every segment just grows in increasing order  
local_temp_vir_ADDR = (temp.start_vm_ADDR>>12 << 12);
while(local_temp_vir_ADDR < temp.end_vm_ADDR)
{
//load parent's address space
//check ifpage is present and then  map it in child's space after setting the COW and readonly flags to true
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(parent_cr3)
																);

	//get page table entry for the given virtual address
page_table_entry = get_page_table_entry(local_temp_vir_ADDR);

if(check_if_physical_presence_bit_set(*page_table_entry))
{
//make cow bit set, write bit  0
*page_table_entry = *page_table_entry &  0xFFFFFFFFFFFFFFFDUL;
*page_table_entry = *page_table_entry &  0x4000000000000000UL;
uint64_t permissions = *page_table_entry & 0xFFF0000000000FFFUL ;
local_temp_phy_ADDR = *page_table_entry & 0x000FFFFFFFFFF000UL;

//load child's space and do mmaping
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(child_cr3)
																);

do_virtual_2_physical_mapping(local_temp_vir_ADDR,local_temp_phy_ADDR,permissions);
I_OR_D_REF_CNT_OF_ADDR(local_temp_phy_ADDR,1);

}
local_temp_vir_ADDR+= 4096;


}
}
//load back parent address
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(parent_cr3)
																);

local_vma_LL = local_vma_LL->next;





}
return child_pp;
}


void switchProcess()
{
//check if any process is available to run
if(process_ready_LL == NULL)// no ready process
{
kprintf("Nothing to switch to\n");
return;
}

//need to check if schedule is called the  first time as  in that case there is no  running task and running task pointer is NULL

if(firstSwitchDone)
{
run_task =  process_ready_LL;
prev  = run_task;

process_ready_LL=process_ready_LL->next;
}
else
{
//running task is pushed to back of list and status changes to ready from running. update runtask
prev =  run_task;
run_task =  process_ready_LL;
next = run_task;

push_into_process_ready_LL(prev);

process_ready_LL = process_ready_LL->next;

SWITCH_TO_MACRO(prev,next);
}
return;
}

void start_Scheduler()
{
firstSwitchDone = 0;

switchProcess();

//need to save rsp as well as rbp ... using rbp helps to refer every address as offset to base pointer

__asm__ volatile ("movq %[stack_p] %%rsp;"
																			:
																			:[stack_p] "m" (prev->rsp)
																	);

__asm__ volatile ("movq %[stack_bp] %%rbp;"
																			:
																			:[stack_bp] "m" (prev->rbp)
																	);
 firstSwitchDone = 1;

}

void timer_irq_handler()
{
//need to update time since boot every 100 jiffies
jiffieCount +=1;

if(jiffieCount%100 == 0)
incTimerValueAndDisplay();//increment timer value every 100 jiffies

if(firstSwitchDone)
{
struct process_control_block* local_timer_LL = process_ready_LL;
//based on timer increment need to update timer(quantum) values for each process  
if(local_timer_LL!=NULL)
{

while(local_timer_LL!=NULL)
{
if(local_timer_LL->status ==SUSPENDED)
{
if(local_timer_LL->quantum_left == 0)//wake up
local_timer_LL->status = READY;
else
{
local_timer_LL ->quantum_left-=1;
}
}
local_timer_LL=local_timer_LL->next;
}

}

// check which task to run .... 

if(run_task == NULL)
{
//means first task is to be scheduled

next= get_next_ready_process_from_LL();

if(next==NULL)
return;
//setup next process' addressspace
__asm__ volatile("movq %0, %%cr3;"
																	:
																	:"r"(next->process_mm->pml4_phys_ADDR)
																);
//change the stack pointer to next process' and if it is user process... need to change to kernel stack rsp stored in TSS' rsp register
__asm__ volatile ("movq %[stack_p] %%rsp;"
																			:
																			:[stack_p] "m" (next->rsp)
																	);
if(next->processType == USER_PROC)
{
set_tss_rsp((uint64_t)&next->stack[0xFFFF]);
//need to change ring level from 0 to 3

//taken from os dev ... mentioned that need to push flags for rpl 3 for all segements, using rax for same 
__asm__ volatile(
																"movq $0x23 , %rax;"
																	"movq %rax, %ds;"
																	"movq %rax, %es;"
																	"movq %rax, %fs;"
																	"movq %rax, %gs;"
																);
}
}
else
{
//save current running process to ready list end and get next ready task ... if it is any different from existing process then just change the stacks which will esseintially switch
//the tasks
uint64_t current_process_stack_ADDR;

__asm__ volatile("movq %%rsp %0;"
																:"=r"(current_process_stack_ADDR)
																);
prev = run_task;
prev->rsp=current_process_stack_ADDR;

next = get_next_ready_process_from_LL();

if(prev!=next)
{
//first load next's address space
//then change the stack pointer
__asm__ volatile("movq %0, %%cr3 ;"
																:
																:"r"(next->process_mm->pml4_phys_ADDR));

__asm__ volatile("movq %0, %%rsp ;"
																:
																:"r"(next->rsp));
//maybe if the process is the user process so need to switch to user space ... setting rsp found that in TSS rsp
if(next->processType == USER_PROC)
{
set_tss_rsp((uint64_t)&next->stack[0xFFFF]);
//need to change ring level from 0 to 3

//taken from os dev ... mentioned that need to push flags for rpl 3 for all segements, using rax for same 
__asm__ volatile(
																"movq $0x23 , %rax;"
																	"movq %rax, %ds;"
																	"movq %rax, %es;"
																	"movq %rax, %fs;"
																	"movq %rax, %gs;"
																);
}
}
}
}

//send end of interrupt to PIC
outByte(0x20,0x20);




}
 
