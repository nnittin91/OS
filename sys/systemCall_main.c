#include<sys/systemCall_main.h>



void system_call_hdlr()
{
uint64_t funcNo;

//implementation of system call is putting the system call functionno in rax so fetching from it
__asm__ volatile("movq %%rax,%0;"
																:"=r"(funcNo)
																);

//just do a switch on funcNo and call specific functions
void * funcPtr;

switch(funcNo)
{
case 0:
funcPtr = read_s;
break;
case 1:
funcPtr = write_s;
break;
case 2:
funcPtr = close_s;
break;
case 3:
funcPtr = open_s;
break;
case 4:
funcPtr = exit_s;
break;
case 5:
funcPtr= fork_s;
break;
case 7:
funcPtr = execvpe_s;
break;
case 8:
funcPtr= wait_s;
break;
case 9:
functPtr = waitpid_s;
break;
case 10:
funcPtr = getprocessId_s;
break;
case 11:
funcPtr= getparentProcessId_s;
break;
case 12:
funcPtr = suspend_s;
break;
case 13:
funcPtr = clear_console_s;
break;
case 14:
funcPtr = kill_PROC;
break;
case 15:
funcPtr = lsPROC;
break;
case 16:
func_Ptr = get_current_working_dir_s;
break;
case 17:
funcPtr= change_directory_s;
break;
case 18:
funcPtr = brk_s;
break;
case 19:
funcPtr = mmap_s;
break;
default:
__asm__ volatile("iretq;");
break;
}

uint64_t system_return_val;
//now we need to just call in using the funcPtr and  return the ret value obtained in rdx
__asm__ volatile("pushq %%rdx");

__asm__ volatile("movq %%rax,%0;"
																	"popq %%rdx;"
																	"callq *%%rax;"
																	:"=a" (system_return_val)
																	:"r" (funcPtr)
																);

__asm__ volatile("iretq;");

}

