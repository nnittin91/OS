#include <sys/timer.h>

void printTimerSpecificIntegers(int timerIntegerValue)
{
								int a[10];
								int len = formPrinteableNumberInArray(timerIntegerValue,10,a);

								if(len < 0)
								{
																*vga_timer_display_offset = '0';
																vga_timer_display_offset +=2;
								}
								else
								{
																while(len >=0)
																{
																								char c = a[len] + 0x30;
																								len -- ;
																								*vga_timer_display_offset = c;
																								vga_timer_display_offset +=2;

																}
								}

} 

// need to write different function as location to be written is different
void printTimerSpecificStrings(char *timerString)
{
int index = 0;
for(;timerString[index]!='\0';index++)
{
*vga_timer_display_offset = timerString[index++];
vga_timer_display_offset += 2;
}
}

void initialize_timer_device()
{
// taking frequency for 1 tick to be 1 Hz 
int divisor = 1193182; //PIC oscillates at 1.193182 MHZ

outByte(0x43,0x36);

outByte(0x40, divisor & 0xff);
outByte(0x40,divisor >>8);
}

void reset_timer_offset()
{
vga_timer_display_offset = vga_start_virt_address + ((VGA_SIZE) -2*VGA_TIMER_STR_SIZE);
}
void incTimerValueAndDisplay()
{
time_elapsed_in_seconds_since_boot ++ ;
CHECK_VAL_DISP(time_elapsed_in_seconds_since_boot % 3600);
CHECK_VAL_DISP(time_elapsed_in_seconds_since_boot % 60);
printTimerSpecificIntegers(time_elapsed_in_seconds_since_boot);
reset_timer_offset();
}
