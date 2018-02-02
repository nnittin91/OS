#include<sys/idt.h>


void initialize_PIC_device()
{
		outByte(PIC1_COMMAND_REG, 0x11);
  outByte(PIC2_COMMAND_REG, 0x11);
  
		outByte(PIC1_DATA_REG, 0x20); // starting interrupts from 0x20 for PIC1
  outByte(PIC2_DATA_REG, 0x28); // starting interrupts from 0x40 for PIC2
  
		outByte(PIC1_DATA_REG, 0x04); // attaching slave to irq2 for PIC1
  outByte(PIC2_DATA_REG, 0x02);// setting SLAVE identity as 2
  
		outByte(PIC1_DATA_REG, 0x01);// using 8086 mode for both ICs
  outByte(PIC2_DATA_REG, 0x01);
  
		outByte(PIC1_DATA_REG, 0x0); //setting masks for both ICs zero ... first time
  outByte(PIC2_DATA_REG, 0x0);
}

void register_Interupt_Handler(int intNo, uint64_t hdlr){
interrupt_Arr[intNo].offset_low = hdlr & 0xFFFF;
interrupt_Arr[intNo].offset_middle = hdlr >> 16 & 0xFFFF;
interrupt_Arr[intNo].offset_high = hdlr >> 32 & 0xFFFFFFFF;

interrupt_Arr[intNo].gpl_selector = 8;

//careful

if(intNo == 128) // syscall ... so kernel execution only
{
interrupt_Arr[intNo].dpl_attr = 0x3;

}
else
{
interrupt_Arr[intNo].dpl_attr = 0x0;
}

interrupt_Arr[intNo].ist = 0x0;

interrupt_Arr[intNo].type = 0xe;

interrupt_Arr[intNo].reserved0_unused = 0x0;
interrupt_Arr[intNo].reserved1_unused = 0x0;

interrupt_Arr[intNo].zero_attr = 0x0;
interrupt_Arr[intNo].p_attr = 0x1;  // is present is true
}

void register_IDT() {

initialize_PIC_device();

idt_Ptr.size_of_entry = sizeof(struct interrupt_desc_table)*256 - 1;

idt_Ptr.address_of_entry =(uint64_t) &interrupt_Arr;

register_Interupt_Handler(0,(uint64_t)&iservice0);
register_Interupt_Handler(10,(uint64_t)&iservice10);
register_Interupt_Handler(13,(uint64_t)&iservice13);
register_Interupt_Handler(14,(uint64_t)&iservice14);
register_Interupt_Handler(32,(uint64_t)&int0);
register_Interupt_Handler(33,(uint64_t)&int1);
//register_Interupt_Handler(128,(uint64_t)&system_call_hdlr);

//load the idt now

lidt(&idt_Ptr);
}

void interrupt_handler(int intNo)
{
if(intNo == 33)
	keyBoardInputHandler();	

//Signal end of Interrupt

outByte(0x20,0x20);

}

void division_by_0_hdlr(int intNo)
{
kprintf("DIVISION BY 0 ENCOUNTERED!! \n");
//system_exit();
}

void tss_flt_hdlr(int intNo)
{
kprintf("INVALID TSS EXCEPTION TRIGGERED!");
}

void gen_protection_fault_handler(int intNo)
{
kprintf(" GENERAL PROTECTION FAULT HANDLER! \n");
}


//remove when written

void page_flt_hdlr(int intNo)
{
}

#if 0
void page_flt_hdlr(int intNo)
{
uint64_t addr_at_fault;
bool isFaultPresent = FALSE;

__asm__ volatile ("movq %%cr2, %0;" 
																						: "=r"(addr_at_fault));

if(addr_at_fault >= KERNEL_START_VIRT_ADDRESS)
{
kprintf("FATAL ERROR!");
return;
}

if(intNo & 0x1)
{
//handle case when page table entry is present

uint64_t *page_table_entry = get_page_table_entry(addr_at_fault);
uint64_t physical_address = *page_table_entry & 0x000FFFFFFFFFF000UL;

uint64_t is_page_w = *page_table_entry & 0x2UL;
uint64_t is_page_copy_on_write = *page_table_entry & 0x4000000000000000UL;

if(!is_page_w && is_page_copy_on_write)
{
int physical_block_ref_cnt = get_phys_block_ref_cnt(physical_addr);

if(physical_block_ref_cnt > 1)
{
uint64_t physical_address_new = allocate_physical_block_address ();
uint64_t temporary_virt_addr = get_virtual_address_for_phy_addr (physical_address_new);
void * page_aligned_addr_at_fault = fault_addr >> 12 << 12;

//copying the memory from fault to temp virtual addr 

memcpy((void*) temporary_virt_addr,page_aligned_addr_at_fault,4096);
*page_table_entry = physical_address_new;

//adding flags

*page_table_entry |= 7UL;;

//release_temporary_virtual_address(temporary_virt_addr);
uint64_t * page_tbl_entry = get_page_table_entry(temporary_virt_addr);
*page_tbl_entry = 0UL;
decrease_phys_block_reference_count(physical_address);
}

else
{
reset_cow(page_table_entry);
set_write(page_table_entry);
}
}
else
isFaultPresent = TRUE;
}
else
{

vma_format_structure * vma_pointer = CURRENT_RUNNING_TASK->mem->vma_lst;

uint64_t low,high;

while(vma_pointer != NULL)
{
//map the fault address with the flags separately
low = vma_pointer->start;
high = vma_pointer->end;


if(addr_at_fault >=low && addr_at_fault < high)
{//map it separately
kernel_mmap(low, high - low, 7UL);
break;
}

vma_pointer = vma_pointer->next;

}
if(vma_pointer == NULL)
isFaultPresent = TRUE;

}
if(isFaultPresent)
{
kprintf("SIGSEGV\n");
system_exit();

}
}
#endif

void iservice_handler(int intOrErrNo)
{
if(intOrErrNo == 0)
division_by_0_hdlr(intOrErrNo);
else if(intOrErrNo == 10)
tss_flt_hdlr(intOrErrNo);
else if(intOrErrNo == 13)
gen_protection_fault_handler(intOrErrNo);
else if(intOrErrNo == 14)
page_flt_hdlr(intOrErrNo);

return;
}

