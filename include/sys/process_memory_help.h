#ifndef __PROCESS_MEMORY_HELP_H__
#define __PROCESS_MEMORY_HELP_H__
#include <sys/help.h>
#include<sys/paging_support_help.h>
#include<sys/virtual_memory_help.h>
#include<sys/kmalloc.h>
struct vma_info_format
{
struct main_process_memory_format * associated_mm;
uint64_t permissions;//even different permissions means a new vma
uint64_t start_vm_ADDR, end_vm_ADDR;
struct vma_info_format * next;

int vma_mem_segmentType;
//optional
uint64_t vm_FD;
};

//process Type
#define USER_PROC 1
#define KERNEL_PROC 0

// permissions
#define READ 1
#define WRITE 2
#define EXEC 4

#define READ_WRITE (READ + WRITE)
#define READ_EXEC  (READ + EXEC)
#define READ_WRITE_EXEC (READ + WRITE + EXEC)
#define WRITE_EXEC (WRITE + EXEC)

// task status types

#define EXIT 0
#define ZOMBIE 1
#define WAIT 2
#define FIRST_KERNAL_PROC 3
#define READY 4
#define RUN 5
#define SUSPENDED 6

//memory types to define PAGE origins

#define MAP_FILE 0
#define MAP_ANON 1
#define MAP_CODE 2
#define MAP_DATA 3
#define MAP_STACK 4
#define MAP_HEAP 5



struct main_process_memory_format
{
uint64_t pml4_phys_ADDR;
struct vma_info_format * process_vma_list;
uint64_t size_of_process_vma_list;
uint64_t total_virtual_mem_for_process;
uint64_t total_stack_virtual_mm_for_process;
uint64_t start_address_stack;
uint64_t start_brk_heap_address,end_brk_heap_address;
};

struct process_control_block
{
struct main_process_memory_format* process_mm;
//Linked list parameters
struct process_control_block* parent_task,*next,*prev,*childRoot, *sameParentTasks;

//
uint64_t number_of_child_tasks;
//process resources
uint64_t *file_descriptors[15];
int PID,ParentPID;
int PID_of_died_Chld;
int processType;
char working_dir[300];
char name[50];
uint32_t quantum_left;
uint32_t rip,rsp;

int current_status;

uint64_t stack[512];

};

static struct process_control_block * process_free_LL;
static struct vma_info_format * available_global_free_vma_LL;
static uint32_t next_available_pid;
static struct process_control_block* first_k_proc;
static struct process_control_block* process_ready_LL;
static int firstSwitchDone = 0;
static int jiffieCount = 0;
//maintain previous running task and next in list to be scheduled

static struct process_control_block* prev, *next, *run_task;

//FUNCTIONS


uint32_t get_next_available_pid();
void set_next_available_pid(uint32_t targetPID);
static void move_process_LL_forward(struct process_control_block* ptr);
static void move_vma_LL_forward(struct vma_info_format* ptr);
static struct process_control_block* get_front_process();
static struct vma_info_format * get_front_vma();
void push_into_process_LL(struct process_control_block* p);
void push_into_vma_LL(struct vma_info_format* p);
struct process_control_block* initialize_KPCB();
struct process_control_block* initialize_UPCB();
void destroy_PCB(struct process_control_block* p);
void del_all_VMA_LL(struct vma_info_format* p);
void copy_PCB_into_child(struct process_control_block* p1, struct process_control_block* p2);
void * kernelMapping(uint64_t startingADDR, uint64_t permissions, uint64_t sz);
struct vma_info_format * map_given_address_into_vma(uint64_t targetADDR, uint64_t permissions,uint64_t fd, uint64_t size, uint64_t vma_type);
int is_given_address_mapped_into_vma(struct process_control_block * p1, uint64_t target_ADDR, uint64_t size);
void increment_heap_via_brk(struct process_control_block* p, uint64_t size);
struct vma_info_format * create_new_vma_entry(struct vma_info_format target);
void first_dummy_kernel_process();
void create_first_kernel_process();
void create_kernel_process_with_FUNC(uint64_t func_ADDR);
void push_into_process_ready_LL(struct process_control_block* p);
struct process_control_block* get_next_ready_process_from_LL();
void copyStackFromParent2Child(uint64_t * parent_cr3,uint64_t* page_table_entry,uint64_t* virtual_ADDR,uint64_t* physical_ADDR,
uint64_t* child_cr3,uint64_t* startingADDR,uint64_t* endingADDR);
void copyParentInfo2ChildForFork(struct process_control_block* c, struct process_control_block* p, uint64_t *child_cr3);
void mainCopyFunctionForFork(struct process_control_block* p);
void switchProcess();
void start_Scheduler();
void timer_irq_handler();





#endif
