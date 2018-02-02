
void lsPROC()
{

								//iterate over the list of processes  and display their info such as parent pid, pid,status

								process_control_block* iter = run_task;

								kprintf("\n PID || PPID || STATUS || NAME \n");
								while(iter!=NULL)
								{
																char st[20];
																switch(iter->status)
																{
																								case 0 :
																																kernelStringCopy(st,"EXIT");
																																break;
																								case 1:
																																kernelStringCopy(st,"ZOMBIE");
																																break;
																								case 2:
																																kernelStringCopy(st,"WAIT");
																																break;
																								case 3:
																																kernelStringCopy(st,"FIRST_KERNEL_PROC");
																																break;
																								case 4:
																																kernelStringCopy(st,"READY");
																																break;
																								case 5:
																																kernelStringCopy(st,"RUN");
																																break;
																								case 6:
																																kernelStringCopy(st,"SUSPENDED");
																																break;

																}

																kprintf(" %d || %d || %s || %s \n",iter->PID,iter->ParentPID, st,iter->name);      
																iter=iter->next;
								}
								return;
}

int clear_console_s()
{
								clScreen();
								return 0;
} 

int getprocessId_s()
{
								//need to return running process' PID
								if(run_task)
																return run_task->PID;
								else
																return -1;
}

int getparentProcessId_s()
{
								//need to return running process' parent's PID
								if(run_task)
																return run_task->ParentPID;
								else
																return -1;
}

uint64_t mmap_s(uint64_t target_ADDR,uint64_t size, uint64_t permissions)
{

								//if you dont have an address then get one from the end... else check if it is valid address .. is it mapped to a vma already... then if everything is cool
								// map it to new vma if required
								// with the given permissions and mind the size
								process_control_block* iter = run_task;
								if(run_task == NULL)
																return NULL;

								vma_info_format* iterVMA = iter->process_mm->process_vma_list;

								if(target_ADDR == 0)
								{
																//go till end of vma list;
																while(iterVMA->next!=NULL)
																{
																								iterVMA=iterVMA->next;
																}
																targetADDR = ((iterVMA->end_vm_ADDR - 1)>>12) << 12;
								}
								else
								{
																if(!is_given_address_mapped_into_vma(run_task,target_ADDR,size))// if unsucessful then we get 0
																{
																								return NULL;
																}
								}

								map_given_address_into_vma(target_ADDR,permissions,0,size,MAP_ANON);
								return target_ADDR;
}

uint64_t brk_s(uint64_t size)
{
								increment_heap_via_brk(run_task,4096*size);
								return run_task->process_mm->end_brk_heap_address;
}

int suspend_s(int timeInMilliSecs)
{
								if(run_task == NULL)
								{
																kprintf("TASK IS NULL\n");
																return -1;
								}

								run_task->status = SUSPENDED;
								run_task->quantum_left = timeInMilliSecs/10;
								//trigger timer interrupt which is 32 
								__asm__ volatile("int $32;");
								return run_task->qunatum_left;
}

void exit_s()
{

								//first remove the current process from the parent process structures
								// iterate over the  parents' children and remove the current process' entry first

								struct process_control_block* iter = run_task;

								struct process_control_block* prnt = iter->parent_task;
								struct process_control_block* siblingIter, *prevSibling = NULL;

								if(prnt)
								{
																siblingIter = prnt->childRoot;
																while(siblingIter!=NULL)
																{

																								if(siblingIter == run_task)
																																break;

																								prevSibling = siblingIter;
																								siblingIter = siblingIter->sameParentTasks;
																}

																//if the task is at root then update childroot else just remove the entry from between ofthe list
																//corner case ... negative testing ...incase siblingIter is NULL then return

																if(siblingIter==NULL)
																								return;

																if(prevSibling == NULL)
																								prnt->childRoot = siblingIter->sameParentTasks;
																else
																								prevSibling->sameParentTasks = siblingIter->sameParentTasks;//entry removal

																//reducing the number of children of parent task
																prnt->number_of_child_tasks -=1;
																//if parent is in wait state for this particular process need to wake it 

																if(prnt->status == WAIT)
																{
																								if(prnt->PID_of_died_Chld == 0  || prnt->PID_of_died_Chld == run_task->PID)
																								{
																																prnt->PID_of_died_Chld = iter ->PID;
																																prnt->status = READY;
																								}
																}
								}

								//Now check if the current running task does have child tasks pending... if yes then we need to make their status to zombie

								if(iter->childRoot)
								{
																struct process_control_block* local_temp= iter->childRoot, *prevChild = NULL;

																while(local_temp)
																{
																								local_temp->status = ZOMBIE;
																								prevChild = local_temp;
																								local_temp=local_temp->sameParentTasks;
																								prevchild->sameParentTasks = NULL;
																}
								}

								//now we can safely remove all the structures

								destroy_PCB(iter);
								iter->status = EXIT;

								//now we need to trigger timer interrupt(kindof invoking the scheduler forcefully so as to schedule a new process
								__asm__ volatile("int $32;");
}

uint64_t waitpid_s (uint64_t processID, uint64_t st, uint64_t flags)
{

								volatile struct process_control_block* iter = run_task;
								int * ptr = (int*) st;
								if(iter->number_of_child_tasks == 0)
								{
																if(ptr)
																								*ptr = -1;

																return -1;
								}

								if(processID >0)//means wait for a particular id
																iter->PID_of_died_Chld = processID;
								else
																iter->PID_of_died_Chld  = 0;//wait for all children


								iter->status = WAIT;

								__asm__ volatile("int $32;");

								if(ptr) 
																*ptr = 0;


								uint64_t result = iter->PID_of_died_Chld;
								return result;
}

uint64_t wait_s (uint64_t st)
{

								volatile struct process_control_block* iter = run_task;
								int * ptr = (int*) st;
								if(iter->number_of_child_tasks == 0)
								{
																if(ptr)
																								*ptr = -1;

																return -1;
								}

								//if(processID >0)//means wait for a particular id // this step not required for wait
								//iter->PID_of_died_Chld = processID;
								//else
								iter->PID_of_died_Chld  = 0;//wait for all children


								iter->status = WAIT;

								__asm__ volatile("int $32;");

								if(ptr) 
																*ptr = 0;


								uint64_t result = iter->PID_of_died_Chld;
								return result;
}

uint64_t execvpe_s(char* binary, char* args[], char * envir[])
{

								//what is required copy the contents of parent just like fork but unlike fork ... current process is gonna die and new   process executed 

								uint64_t retValue = 0;
								struct process_control_block * local_temp= initiate_elf_process(binary,args);
								if(local_temp != NULL)
								{
																local_temp->parent_task = run_task->parent_task;
																local_temp->PID = run_task->PID;
																local_temp->ParentPID = run_task->ParentPID;
																set_next_available_pid(run_task->PID);
																kernelStringCopy(local_temp->working_dir, run_task->working_dir);
																kernelStringCopy(local_temp->name, run_task->name);

																memcpy((void*)local_temp->file_descriptors,(void*) run_task->file_descriptors,120);

																//now after copying the contents we can call for replacing the current process with this one.
																copy_PCB_into_child(run_task,local_temp);
																destroy_PCB(run_task);
																run_task->status = EXIT;

																//now call the scheduler as usual

																__asm__ volatile ("int $32;");
								}

								else
																retValue = -1;
								return retValue;
}

int chdir_s(const char* directoryPath)
{
								if(run_task== NULL)
																return -1;

								kernelStringCopy(run_task->working_dir,directoryPath);
								return 0;
}
// data validation should be done in user space and avoid system call with faulty data
char* get_current_working_dir(char * buf)
{
								if(run_task==NULL)
																return NULL;

								kernelStringCopy(buf,run_task->working_dir);
								return buf;
}

int kill_PROC(int targetPID)
{
								//iterate over the list of processes and empty the structures safely
								struct process_control_block* proc_with_pid_1==NULL, *target_proc=NULL, *temp=run_task;
								
								while(temp!=NULL)
								{
									if(temp->PID == 1)
													{
														proc_with_pid_1 = temp;
												//		break;
													}
										else if(temp->PID == targetPID)
															{
																target_proc = temp;
															}
									temp=temp->next;
								}

							if(proc_with_pid_1 == NULL || target_proc == NULL)
									{
									kprintf("wrong PID\n");//need not store the valid pids in user space appliation although that may seem as an optimization
									return -1;
									}
						target_proc->status = EXIT;
						if(target_proc->number_of_child_tasks >0)
							{
										temp = target->childRoot;
										while(temp!=NULL)
									{
										if(temp->PID == 1)
											break;
										temp->parent_task =  proc_with_pid_1;
										temp->parentPID = 1;
										temp=temp->sameParentTasks;
										
									}
									return 0;
							}
return 0;
					

}

int fork_s()
{
//because this code runs always in the context of parent process so  need to return the pid value of the newly created process and need to push in value 0 onto stack so that when child process 
//added to the ready list is scheduled it retrieves it from rax register

if(run_task == NULL)
{
kprintf("CRTICAL_FAILURE\n");
return -1;
}

struct process_control_block* child_task = mainCopyFunctionForFork(run_task);
child_task->stack[0xFFFF] =0x23;
child_task->stack[0xFFFE] =run_task->stack[0xFFFD];
child_task->stack[0xFFFD] = 0x200202UL;
child_task->stack[0xFFFC]= 0x1B;
child_task->stack[0xFFFB]= (uint64_t)run_task->stack[0xFFFA];//starting from where we have the popped register values from parent
child_task->stack[0xFFF9]/*stack address for pushed in rax*/ = 0x0UL;
child_task->stack[0xFFDF] = (uint64_t)int0 + 0x20;

chid_task->rip = (uint64_t)run_task->stack[0xFFFA];
child_task->rsp = (uint64_t)child_task->stack[0xFFDE];
push_into_process_ready_LL(child_task);

return child_task->PID;
} 




