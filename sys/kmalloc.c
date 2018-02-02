#include<sys/kmalloc.h>

static void correct_alignment(int * size)
{
								if((*size % BLOCK_ALIGNMENT_SIZE) !=0)
								{
																*size = *size >> BLOCK_ALIGNMENT_SIZE_PWR_OF_2;
																*size = *size << BLOCK_ALIGNMENT_SIZE_PWR_OF_2;
																*size += BLOCK_ALIGNMENT_SIZE;
								}
}
void initialize_local_kernel_malloc()
{
								total_curr_free_memory_available = 0;
								ptr_to_current_free_memory = 0;
}

void *kmalloc(int size)
{
								void * answer = NULL;
								correct_alignment(&size);

								if(size > total_curr_free_memory_available)
								{
																//need to allocate virtual pages

																int no_of_reqd_pages = size /(PAGE_SIZE + 1) + 1;

													//			answer = allocate_virtual_pages_memory(no_of_reqd_pages,PRESENT|WRITEABLE);

																if(answer !=NULL) // allocation success
																{
																								total_curr_free_memory_available = ((no_of_reqd_pages*PAGE_SIZE) - size);
																								ptr_to_current_free_memory = (uint64_t)answer + (uint64_t)size; 
																}

								}
								else
								{
																// we have required memory;
																answer = (void*)ptr_to_current_free_memory;
																ptr_to_current_free_memory+=(uint64_t)size;
																total_curr_free_memory_available -=(uint64_t)size;
								}
								return answer; // allocation fail means answer is NULL
}              
