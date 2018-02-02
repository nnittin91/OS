#include <dirent.h>

void * isValidFileInPath(char * path)
{
	if(!path)
	return NULL;
	
	char localPath[512];
	localPath=kernelStringCopyN(localPath,path,kernelStrlen(path)); //  may be buggy

	char *temp =   kernelStrtok(localPath,"/");
	
	if(!temp) // means root  directory so no file
	return NULL; 

	//breaking path by backslashes "/" 
// iterating from 2nd node   
// have to search the entire tree starting from root
struct file_info * root = tarfs_root_node;
struct file_info* iter =tarfs_root_node;
while(temp!=NULL)
{
int i = 2;
struct file_info* temp_file = iter;
while(i< iter->number_of_childs_in_subtree)
{
int check = kernelStrcmp(temp,iter->child[i]->name);

if(!check) //found.... so break
{
iter = (struct file_info*)iter->child[i];
break;
}
i++;
}
if(i==temp_file->number_of_childs_in_subtree)
return NULL;

temp = kernelStrtok(NULL,"/");
}
if(iter->fileOrDir == 0)//not file
return NULL;
else
return (void*)iter->firstChild;


}

void locateAndConstructFileNode(char * path, uint64_t startNode,uint64_t endNode, int isFile)
{
	if(!path)
	return;
	
	struct file_info * local_new_fileNode;
	char localPath[512];
	localPath=kernelStringCopyN(localPath,path,kernelStrlen(path)); //  may be buggy

	char *temp =   kernelStrtok(localPath,"/");
	
	if(!temp) // means root  directory so no file
	return; 

	//breaking path by backslashes "/" 
// iterating from 2nd node   
// have to search the entire tree starting from root
struct file_info * root = tarfs_root_node;
struct file_info* iter =tarfs_root_node;
while(temp!=NULL)
{
int i = 2;
struct file_info* temp_file = iter;
while(i< iter->number_of_childs_in_subtree)
{
int check = kernelStrcmp(temp,iter->child[i]->name);

if(!check) //found.... so break
{
iter = (struct file_info*)iter->child[i];
break;
}
i++;
}
if(i==temp_file->number_of_childs_in_subtree)
{
//create node
//first allocate memoryand then assign values
local_new_fileNode = (struct file_info* )kmalloc(sizeof(struct file_info));
struct file_info_node local;
local.startingChild = startNode;
local.lastChild = endNode;
local.fileOrDir = isFile;
construct_file_info_node(&local,local_new_fileNode,iter);
iter->children[iter->number_of_childs_in_subtree]= local_new_fileNode;
iter->number_of_childs_in_subtree++; 
}

temp = kernelStrtok(NULL,"/");
}

//logic is same as lookup but here in case at any particular point you don't find the node create it.


}

void * setupInitialTarFileSystem()
{
// need to return the root of the setup fileSystem

tarfs_Header * fileSysHeader  = (tarfs_Header *)&(_binary_tarfs_start);
tarfs_root_node = (struct file_info_node*)kmalloc(sizeof(struct file_info_node));
struct file_info_node * rootfsDir = (struct file_info_node*)kmalloc(sizeof(struct file_info_node));
struct file_info_node * diskDir = (struct file_info_node*)kmalloc(sizeof(struct file_info_node));

struct file_info_node local_temp_file_node;
local_temp_file_node.startingChild = 0;
local_temp_file_node.number_of_childs_in_subtree = 2;
kernelStringCopyN(local_temp_file_node.name,"/",sizeof("/"));
local_temp_file_node.fileOrDir = 0;//DIR
local_temp_file_node.inodeNo = 0;

construct_file_info_node(&local_temp_file_node,tarfs_root_node,tarfs_root_node);

kernelStringCopyN(local_temp_file_node.name,"Rootfs",sizeof("Rootfs"));

construct_file_info_node(&local_temp_file_node,rootfsDir,tarfs_root_node);
kernelStringCopyN(local_temp_file_node.name,"Disk",sizeof("Disk"));

construct_file_info_node(&local_temp_file_node,diskDir,tarfs_root_node);

while(fileSysHeader <(tarfs_Header *)&(_binary_tarfs_end))
{
int fileSize = oct_to_dec(stoi(fileSysHeader->size));

int checkifFile  = (kernelStrcmp(fileSysHeader->typeflag,"5"));
if(!checkifFile)
{
locateAndConstructFileNode(fileSysHeader->name,0,2,0);
}
else
{
uint64_t startingNode = (uint64_t)(fileSysHeader + 1);
uint64_t endNode = (uint64_t)(fileSysHeader + fileSize  + /*metadata */ 512);
locateAndConstructFileNode(fileSysHeader->name,startingNode,endNode,1);
}
if(fileSize)
fileSysHeader = fileSysHeader + fileSize/(sizeof(tarfs_Header) +1) +2;
else
fileSysHeader+=1;
}

return (void*)&tarfs_root_node;




}

void construct_file_info_node(struct file_info_node * targetInfo, struct file_info_node* self, struct file_info_node* parent)
{
if(!targetInfo || !self)
return;
//save self and parent both in per node hierarchy
self->children[0] = self;
self->children[1] = parent;

kernelStringCopyN(self->name,targetInfo->name,kernelStrlen(targetInfo->name));
self->startingChild = targetInfo->startingChild;
self->lastChild=targetInfo->lastChild;
self->presentChild = targetInfo->presentChild;
self->fileOrDir = targetInfo->fileOrDir;
self->inodeNo = targetInfo->inodeNo;

}

struct file_info_node * get_tarfs_root_node()
{
return tarfs_root_node;
}

