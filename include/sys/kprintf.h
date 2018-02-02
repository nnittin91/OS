#ifndef __KPRINTF_H
#define __KPRINTF_H
#include<sys/defs.h>
#define VGA_SIZE (160*25)
#define vga_start_virt_address (char*)0xffffffff800b8000 
#define PRINT_CHAR(C) \
do { \
																																*current_vga_cursor = C;\
																																current_vga_cursor+=2;\
																																linePosition+=2;\
																																check_if_buffer_is_full(C);\
																																modify_cursor();\
}while(0);

#define va_start(v,l)     __builtin_va_start ((v), l)
#define va_arg(v,l)       __builtin_va_arg ((v), l)
#define va_end(v)         __builtin_va_end ((v))
#define va_copy(d,s)      __builtin_va_copy ((d), (s))
typedef __builtin_va_list va_list;


void kprintf(const char *fmt, ...);
void check_if_buffer_is_full(char c);
void initialize_cursor(uint8_t cursor_start, uint8_t cursor_end);
void destroy_cursor();
void modify_cursor();
void printCharacter(char c);
void printString(char *s);
int formPrinteableNumberInArray(int n, int base,int * a);
int formPrinteableLongNumberInArray(uint64_t n, int base,uint64_t * a);
void printInteger(int n);
void printHexadecimal(int n);
void printPointer(uint64_t n);
void handleNextLine();
void handle_carriage_return();
void handle_fmtstring_print(const char *fmt);
void print_common_handler(const char *fmt, va_list args);
void clScreen();
int32_t puts(const char* str);


















#endif
