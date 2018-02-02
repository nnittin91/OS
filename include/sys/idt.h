#ifndef __IDT_H__
#define __IDT_H__

#include<sys/defs.h>
#include <sys/kprintf.h>
#include <sys/help.h>
#include<sys/keyboard.h>

// taken cue from OSDev.org for the structure format for x86 64 bit
struct interrupt_desc_table{
  uint16_t offset_low;
//gdt segment selector
  uint16_t gpl_selector;
  unsigned ist : 3 ; 
  unsigned reserved0_unused : 5;
  unsigned type : 4;
  unsigned zero_attr : 1;
  unsigned dpl_attr : 2; // important for setting privelege levels in case of user/kernel
  unsigned p_attr : 1;
  uint16_t offset_middle;
  uint32_t offset_high;
  uint32_t reserved1_unused;
}__attribute__((packed));

struct ptr_to_interrupt_desc_table {
  uint16_t size_of_entry;
  uint64_t address_of_entry;
}__attribute__((packed));

static struct interrupt_desc_table interrupt_Arr[256];

static struct ptr_to_interrupt_desc_table idt_Ptr;

void lidt(struct ptr_to_interrupt_desc_table* idtr);

#define PIC1_COMMAND_REG 0x20
#define PIC2_COMMAND_REG 0xA0
#define PIC1_DATA_REG 			0x21
#define PIC2_DATA_REG 			0xA1

extern void iservice0();
extern void iservice10();
extern void iservice13();
extern void iservice14();
extern void iservice_common();

extern void int0();
extern void int1();
extern void int_common();

extern void system_call_hdlr();


void initialize_PIC_device();
void register_Interupt_Handler(int intNo, uint64_t hdlr);
void register_IDT();
void interrupt_handler(int intNo);
void division_by_0_hdlr(int intNo);
void tss_flt_hdlr(int intNo);
void gen_protection_fault_handler(int intNo);
void page_flt_hdlr(int intNo);
void iservice_handler(int intOrErrNo);










#endif
