#ifndef __HELP_H__
#define __HELP_H__
#include <sys/defs.h>
void outByte (unsigned short devicePort, unsigned char data);
void outWord (uint16_t devicePort, uint16_t data);
unsigned char inByte (unsigned short devicePort);
int32_t atoi(char* str);
int32_t pow(int base, int exp);
int32_t oct_to_dec(int n) ;
int open (const char* filePath,  int flags);
int read(int fd, void *buf, int readBytes);
int strfirstOccur(const char* s1, const char* s2);
char *strchr(const char *s, int c); 
void memset(char *str, char val, int size);
 void *memcpy(void *destination, void *source, uint64_t num) ;

//string helping functions

char * kernelStringCopyN(char *destination, const char *source, uint64_t n);
char * kernelStringCopy(char *dest, const char *src);
char* kernelConcat(char *str1,  char *str2);
int kernelStrcmp(const char *string1, const char *string2);
int kernelStrlen(char *str);
char * kernelStrtok(char *s, const char* delim);


#endif
