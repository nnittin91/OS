#ifndef __TIMER_H__
#define __TIMER_H__
#include <sys/kprintf.h>
#include <sys/help.h>
#define VGA_TIMER_STR_SIZE 14
char * vga_timer_display_offset = vga_start_virt_address + ((VGA_SIZE) -2*VGA_TIMER_STR_SIZE);
int time_elapsed_in_seconds_since_boot;
#define CHECK_VAL_DISP(V) \
do { \
if(V) \
printTimerSpecificIntegers(V); \
else \
printTimerSpecificStrings("00"); \
*vga_timer_display_offset = 0x3A; \
vga_timer_display_offset += 2; \
}while(0);

void printTimerSpecificIntegers(int timerIntegerValue);
void printTimerSpecificStrings(char *timerString);
void initialize_timer_device();
void reset_timer_offset();
void incTimerValueAndDisplay();







#endif
