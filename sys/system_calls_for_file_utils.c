
 
int seek_s (int fileOrDir, int distance, int base)
{

// what we need to do is : we need to find the start and end address of the vma list assigned to the file associated to the process and then we need to move
//distance from base accordingly
if(run_task == NULL)
return -1;

struct file_D* temp =((struct file_D *)(run_task->file_descriptors[fileOrDir]));


if(temp->associated_file->fileOrDir == 0)
return -1;
uint64_t start_ADDR;
uint64_t end_ADDR;

if(temp->inode_info == 0)
{
start_ADDR = temp->associated_file->startingChild;
end_ADDR = temp->associated_file->number_of_childs_in_subtree;

}
else
{
//now iterate the vms for getting the required
struct vma_info_format* iter = run_task->process_mm->process_vma_list;

while(iter!=NULL)
{

if(iter->vm_FD == fileOrDir)
{
break;
}

iter=iter->next;

}
start_ADDR = iter->start_vm_ADDR;
end_ADDR = iter->end_vm_ADDR;

}
//now we just have to move wrt to base distance if the address lies in between start and end.
if(base == START_FILE)
{
if(distance <=0)
distance =0;

temp->currentPtr = start_ADDR + distance;

return distance;
}
else if(base == CURRENT_LOCATION)
{
if((distance +temp->currentPtr) >end_ADDR)//can't go beyond end address
{
//distance =0;
temp->currentPtr = end_ADDR;
}
else
temp->currentPtr = temp->currentPtr + distance;

return (end_ADDR - temp->currentPtr + 1);
}
else if(base == EOFILE)
{
//if the distance is negative .. it is allowed ..keeps us within bound
if(distance >=0)
distance =0;

temp->currentPtr = start_ADDR + distance;
return distance;

}
else
return -1;

return -1;
}
 
int write_s(int targetLocation, uint64_t addr, int size)
{
size = 0;
if(targetLocation == STDOUT || targetLocation == STDERR)
{
size = puts((char*)addr);
}
return size;
} 

int read_s(int targetFD, uint64_t output, uint64_t size)
{
if(targetFD == 0)
{
size = gets((char*)output);
return size;
}
else if(targetFD == STDOUT || targetFD == STDERR)
{
kprintf("INVALID FD .. NOT SUPPORTED \n");
return 0;
}
else
{
// now if the file is write only or if the file is not present in open state then not possible to do a read so return failure indicated by -1
struct file_D* temp =((struct file_D *)(run_task->file_descriptors[fileOrDir]));
uint64_t local_length = 0, ending_ADDR;
if(temp->permissions ==WRITE)
{
return -1;
}
else if(temp->associated_file->inodeNo == 0)
{
local_length = temp->currentPtr;
ending_ADDR = temp->associated_file->number_of_childs_in_subtree;
}
else
{
//update local_length 
local_length = temp->currentPtr;
struct vma_info_format * temp_vma_list = run_task->process_mm->process_vma_list;
while(temp_vma_list !=NULL)
{

if(temp_vma_list->vm_FD == targetFD)
break;

temp_vma_list=temp_vma_list->next;
}
if(temp_vma_list!=NULL)
ending_ADDR = temp_vma_list->end_vm_ADDR;


}

if(ending_ADDR - local_length < size)
size = ending_ADDR - local_length;
//now you know the start location which is the current ptr of current vma, then size if known
memcpy((void*)output,(void*)temp->currentPtr,size);

//now update current 
temp->currentPtr = temp->currentPtr + size;

}
return size;
}

void close_s(int fileDescriptor)
{
struct file_D* temp =((struct file_D *)(run_task->file_descriptors[fileDescriptor]));

if(temp->inode_info!=NULL)
{
struct vma_info_format* vma_iter = run_task->process_mm->process_vma_list, *prev=NULL;

while(vma_iter!=NULL)
{
if(vma_iter->vm_FD == fileDescriptor)
break;

prev = vma_iter;
vma_iter=vma_iter->next;
}

prev->next = vma_iter->next;
push_into_vma_LL(vma_iter);


}
temp = NULL;


  
}

int open_s(char* path, uint64_t permissions)
{
char * cwd = NULL;
struct file_info_node* temp_file_node,*fileIter=get_tarfs_root_node();
struct file_D * local_FD;
if(path[0] != '/')
{
//means we need to append the current working directory into path
kernelStringCopy(cwd,run_task->working_dir);
kernelConcat(cwd,"/");
kernelConcat(cwd,path);
}
else
kernelStringCopy(cwd,path);

char * local_name =NULL;
local_name = kernelStrtok(cwd,"/");

if(temp==NULL)
return -1;//not exists

int check1 = (kernelStrcmp(temp,"Rootfs"));

if(check1==0)
{

while(temp!=NULL)
{
temp_file_node = get_tarfs_root_node();

//we have stored the node and the parent as the first 2 entries of the associated nodes at each level
//therefore we iterate from the 3rd element
int i = 2;
while(i<fileIter->number_of_childs_in_subtree)
{
int check2 = (kernelStrcmp(temp,fileIter->child[i]->name));

if(check2==0)
{
fileIter = fileIter->child[i];
break;
}
i++;
}
if(i==fileIter->number_of_childs_in_subtree)//means could not find the entry so negative case return -1
return -1;

temp=kernelStrtok(NULL,"/");


}

if(fileIter->fileOrDir==0)return -1;//means directory 

local_FD->associated_file=fileIter;
local_FD->currentPtr = fileIter->startingChild;
local_FD->permissions = permissions;
//now save this file descriptor entry into the PCB

int i =0;
for(i=3;i<15;i++)
{
//find the empty slot by linear search
if(run_task->file_descriptors[i]==NULL)
{
run_task->file_descriptors[i]=(uint64_t*)local_FD;
//break;
return i;
}
}
return -1;
}
return -1;

}

int closedir_s(uint64_t* directory_entry)
{
struct mapping_file_info_dirent* file_d_map =directory_entry;
// if the given directory entry is valid then we can close it basically making the entry NULL

if(file_d_map->fil->fileOrDir==0 && file_d_map->currentIter>0)
{
file_d_map->fil=NULL;
file_d_map->currentIter = 0;
return 0;
}
return -1;

}


struct dirent * readdir_s(uint64_t* directory_entry)
{
struct mapping_file_info_dirent * file_d_map= directory_entry;
//to check if current node is valid ... just check if it has anything in its hierarchy and if it has any postive contents ... both of them essentially indicate the same thing.
if(file_d_map->fil->number_of_childs_in_subtree<3||file_d_map->currentIter==0||file_d_map->currentIter==file_d_map->number_of_childs_in_subtree)
return NULL;

kernelStringCopy(file_d_map->dirnt.name,file_d_map->fil->child[file_d_map->currentIter]->name);
file_d_map->currentIter++;
return &file_d_map->dirnt;
}

struct mapping_file_info_dirent* opendir_s(uint64_t* targetEntry, uint64_t* directory_entry)
{

struct mapping_file_info_dirent* local_dir=directory_entry;
char* targetPath = targetEntry;

struct file_info_node* local_temp_node = NULL, * fileIter=get_tarfs_root_node();

//copy targetPath into local variable else it will be modifying the original entry as well... be careful

char* pathIter;
kernelStringCopy(pathIter,targetPath);
char * temp = kernelStrtok(pathIter,"/");
while(temp!=NULL)
{
	local_temp_node=fileIter;
	
//check if the  path  contains dot or double dot indicating self or parent

int check1 = kernelStrcmp(temp,".");
int check2 = kernelStrcmp(temp,"..");

if(check1==0)
fileIter=(struct file_info_node*)fileIter->child[0];
else if(check2==0)
fileIter = (struct file_info_node*)fileIter->child[1];
else
{
int i = 2;

while(i<fileIter->number_of_childs_in_subtree)
{
int check3 = (kernelStrcmp(temp,fileIter->file_info_node[i]->name));
if(check3==0)//means found the required file
{
fileIter = fileIter->file_info_node[i];
break;
}
i++;
}

if(i == fileIter->number_of_childs_in_subtree)
{
//not found so need to return ... need to return a  dirent mapping structure

local_dir->currentItr=NULL;
local_dir->fil=NULL;
return local_dir;

}
}
temp=kernelStrtok(NULL,"/");
}

if(fileIter->fileOrDir==0)
{
//if it is a  directory then its currentIter should be 2 as first 2 are occupied at 0 and 1 as self and parent
local_dir->currentIter=2;
local_dir->fil = fileIter;
}
else
{
local_dir->currentItr=NULL;
local_dir->fil=NULL;
}
return local_dir;
}
