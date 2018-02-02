#include <sys/kprintf.h>
#include<sys/help.h>
int linePosition;
char *current_vga_cursor = vga_start_virt_address;
void check_if_buffer_is_full(char c)
{
if(current_vga_cursor > (vga_start_virt_address +(VGA_SIZE)))
{
clScreen();
if(c)
{
*current_vga_cursor = c;
current_vga_cursor+=2;
linePosition+=2;
modify_cursor();
}

}

}

void kprintf(const char *fmt, ...)
{
  va_list args;
   va_start(args, fmt);
   print_common_handler(fmt, args);
   va_end(args);
}


//Cursor related functions
void initialize_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
 outByte(0x3D4, 0x0A);
 outByte(0x3D5, (inByte(0x3D5) & 0xC0) | cursor_start);
 
 outByte(0x3D4, 0x0B);
 outByte(0x3D5, (inByte(0x3E0) & 0xE0) | cursor_end);
}

void destroy_cursor()
{
 outByte(0x3D4, 0x0A);
 outByte(0x3D5, 0x20);
}

void modify_cursor()
{
  uint16_t current_position_off = (uint16_t)((char*)current_vga_cursor-vga_start_virt_address)/2;
  outWord(0x3d4, 0x0e | (current_position_off & 0xff00));
  outWord(0x3d4, 0x0f | (current_position_off << 8)); 
}

// print cases

void printCharacter(char c){
PRINT_CHAR(c);

}

void printString(char *s){
while(*s!='\0')
{
PRINT_CHAR(*s);
s++;
}
}
int formPrinteableNumberInArray(int n, int base,int * a)
{
//int a[base];
int len  = 0;
while(n>0)
{
a[len++] = n%base;
n/=base;
}
return len-1;
}

int formPrinteableLongNumberInArray(uint64_t n, int base,uint64_t * a)
{
//int a[base];
int len  = 0;
while(n>0)
{
a[len++] = n%base;
n/=base;
}
return len-1;
}
void printInteger(int n){
int a[10];
int len = formPrinteableNumberInArray(n,10,a);
while(len>=0)
{
PRINT_CHAR(a[len]+0x30);
len --;
}
}

void printHexadecimal(int n){
int a[16];
int len = formPrinteableNumberInArray(n,16,a);
while(len>=0)
{
if(a[len]<10)
{
PRINT_CHAR(a[len]+0x30);
}
else
{ 
PRINT_CHAR(a[len]+87);
}
len --;
}

}

void printPointer(uint64_t n){
uint64_t a[16];
printString("0x");
int len =formPrinteableLongNumberInArray(n,16,a);
while(len>=0)
{
if(a[len]<10)
{
PRINT_CHAR(a[len]+0x30);
}
else
{ 
PRINT_CHAR(a[len]+87);
}
len --;
}
}

void handleNextLine()
{
								current_vga_cursor +=(160-linePosition);
								linePosition = 0;
								check_if_buffer_is_full(0);
								modify_cursor();
}

void handle_carriage_return()
{
								current_vga_cursor-=(linePosition);
								linePosition = 0;
								check_if_buffer_is_full(0);
								modify_cursor();
}

void handle_fmtstring_print(const char *fmt)
{
PRINT_CHAR(*fmt);
}

void print_common_handler(const char *fmt, va_list args)
{

								//iterate over the format to detect format specifiers

								while(*fmt!='\0')
								{
																switch(*fmt)
																{
																								case '%':
																																fmt++;
																																break;
																								case 'd':
																																printInteger(va_arg(args,int));
																																break;
																								case 's':
																																printString(va_arg(args, char*));
																																break;  
																								case 'c':
																																printCharacter(va_arg(args, int));
																																break;
																								case 'x':
																																printHexadecimal(va_arg(args, int));
																																break;
																								case 'p':
																																printPointer(va_arg(args, uint64_t));
																																break;
																								case '\n':
																																handleNextLine();
																																break;
																								case'\r':
																																handle_carriage_return();
																																break;
																								default:
																															handle_fmtstring_print(fmt);
																															break;
																}

																
																fmt++;
								}
}

void clScreen()
{
current_vga_cursor = vga_start_virt_address;

int i =0;

for(;i<VGA_SIZE;i++)
{
*((char*)current_vga_cursor+i) = 0;
}
linePosition = 0;
modify_cursor();
}



// puts definition
int32_t puts(const char* str)
{
kprintf("%s",str);
return 1;
}
