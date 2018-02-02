#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__
#include<sys/kprintf.h>
#include<sys/help.h>
#include <sys/defs.h>

#define INVALID_SCAN_CODE 0xff

extern char* current_vga_cursor;
extern int linePosition;

struct scan_code_mapping_format
{
uint32_t scan_code;
char ascii_char_value;
char ascii_shift_combo_char_value;
}scan_code_mapping;


// counters used to manage key-board sequences

int scan_code_sequence_number;
int shift_key_pressed_seq_number;
int ctrl_key_pressed_seq_number;

volatile uint32_t gets_in_progress;

uint32_t gets_buffer_sz;
char buffer_used_for_gets[1024];



void signal_gets_trigger();
void release_gets_trigger();
int get_gets_trigger();
int gets(uint64_t destinationStr);
struct scan_code_mapping_format get_mapping(unsigned char inputScanCode);
void outputNewLine();
void backSpace();
void takeInputIntoBuffer(char c);
void keyBoardInputHandler();





#endif
