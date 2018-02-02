#include <sys/tar_file_system_help.h>

void* tar_file_system_file_search(char *tarfs_fileName) 
{

tarfs_Header * fileSysHeader = (tarfs_Header *)&(_binary_tarfs_start);

int fileSize = 0;


while(fileSysHeader <(tarfs_Header *)(&(_binary_tarfs_end)))
{
fileSize = oct_to_dec(atoi(fileSysHeader -> size));
//if filename is matched then return the pointer to the header right away

if(kernelStrcmp(tarfs_fileName,fileSysHeader -> name) == 0) // a  match
{
char * temp = (char*)(fileSysHeader + 1);
return (void*)temp;
}

//now if file size > 0, return pointer after skipping its contents as well as metadata length

if(fileSize > 0)
{
fileSysHeader += (fileSize/(sizeof(tarfs_Header) +   1)) + 2;
}
else
{
fileSysHeader+=1;
}

}
return NULL;
}
