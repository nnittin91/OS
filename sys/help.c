#include <sys/kprintf.h>
#include<sys/defs.h>
#include <sys/help.h>
#define OPEN 2
#define READ 0
void outWord (uint16_t devicePort, uint16_t data)
{
        __asm__ volatile ("outw %w0, %w1" 
																										: 
																										: "a" (data), "Nd" (devicePort));
}


void outByte (unsigned short devicePort, unsigned char data){
        __asm__ volatile ("outb %1, %0" 
																											: 
																											: "dN" (devicePort), "a" (data));
}

unsigned char inByte (unsigned short devicePort){
        unsigned char retVal;
        __asm__ volatile ("inb %1, %0" 
																											:"=a"(retVal) 
																											:"dN" (devicePort));
        return retVal;
}

int kernelStrlen(char *str){
								int i = 0;
								while(str[i]!='\0')
								{
																i++;
								}
								return i;
}

int32_t atoi(char *str){
								int result = 0;  
								int posNeg = 1;  
								int i = 0;  

								if (str[0] == '-'){
																posNeg = -1;  
																i=1; 
								}
								if(str[0] == '+'){
																i=1;
								}

								for (; str[i] != '\0'; ++i)
																result = result*10 + str[i] - '0';

								return posNeg*result;
}

int32_t pow(int base, int exp)
{
int res = base;
int i =1;

if(exp == 0)
return 1;

if(exp<0)
return -1;

for(;i<exp;i++)
{
res*=base;
}
return res;
}

int32_t oct_to_dec(int n) {
int decimalNo = 0, i = 0;

    while(n != 0)
    {
        decimalNo += (n%10) * pow(8,i);
        ++i;
        n/=10;
    }

    i = 1;

    return decimalNo;
}

//local function
int strfirstOccur(const char* s1, const char* s2){
								const char *dest = s1, *pattern=s2;
								char p, d;

								while(*dest != '\0')
								{
																d = *dest++;
																do
																{
																								if((p=*pattern++) == d)
																																return dest-s1-1;
																}while(p!=0);
								}
return 0;
}

char *strchr(const char *s, int c){

								char ch = c;
								while(*s!=ch && *s!='\0')
								{
																s++;
								} 
								if(*s=='\0')
																return 0;
								else
																return (char*)s;


}


char * kernelStrtok(char *s, const char* delim){
								static char* lastStringState;
								int chOccur;
								if(s==NULL)
																s=lastStringState;
								do
								{
																if((chOccur = *s++) == '\0')
																								return NULL;
								}while(strchr(delim,chOccur));
																								s--;
																								lastStringState =s+ strfirstOccur(s,delim);
																								if(*lastStringState !=0)
																								*lastStringState++ = 0;

																								return s;
}

void memset(char *str, char val, int size)
{
int i =0;
while(i<size)
{
str[i++]=val;
}
}

void *memcpy(void *destination, void *source, uint64_t num)
{
int i =0;
unsigned char * d = (unsigned char*)destination;
unsigned char* s = (unsigned char*)source;
for(;i<num;i++)
{
d[i]=(char)s[i];
}
return d;
}

//ASM calls

int open(const char *filePath, int flags)
{
int retVal;
__asm__ volatile
("syscall"
:"=a" (retVal)
:"0"(OPEN), "D"(filePath),"S"(flags)
:"cc","rcx","memory","r11"
);

return retVal;
}

int read(int fd, void *buf, int readBytes){
int retVal;
__asm__ volatile
("syscall"
:"=a" (retVal)
:"0"(READ),"D"(fd),"S"(buf),"d"(readBytes)
:"cc","rcx","memory","r11"
);

return retVal;
}

//remaining String Utilities

int kernelStrcmp(const char *string1, const char *string2)
{
								int i = 0;
								for(;string1[i]!='\0' && string2[i]!='\0';i++)
								{   
																if(string1[i]!=string2[i])
																								return (string2[i]>string1[i])?-1:1;
								}   
								if(string1[i]==string2[i])
																return 0;
								else if(string1[i] == '\0')
																return -1; 
								else 
																return 1;

}

char* kernelConcat(char *str1,  char *str2)
{
    uint64_t len1 = kernelStrlen(str1);
    uint64_t len2 = kernelStrlen(str2);
    uint64_t i = 0;

    for(i = 0; i < len2 ; i++)
        str1[len1 + i] = str2[i];
    str1[len1 + i] = '\0';

    return str1;    
}

char * kernelStringCopy(char *dest, const char *src)
{
    char *str = dest;
    while (*src) {
        *dest++ = *src++;
    }   
    *dest = '\0';
    return str;
}

char * kernelStringCopyN(char *destination, const char *source, uint64_t n)
{
    uint64_t i = 0;
    char *str = destination;

    for (i = 0; i < n; i++) {
        *destination++ = *source++;
        if ( *source == '\0') {
            *destination++ = '\0';
            return(str);
        }
    }
    *destination++ = '\0';

    return str;
}


