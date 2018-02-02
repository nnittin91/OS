#ifndef _DIRENT_H
#define _DIRENT_H

#define NAME_MAX 255

struct dirent {

 char d_name[NAME_MAX+1];
};
struct file_info_node
{
char name[512];
uint64_t startingChild,number_of_childs_in_subtree,presentChild;
struct file_info_node child[15];
int fileOrDir;
uint64_t inodeNo;
};

struct file_D
{
//uint64_t inodeNo;
struct file_info_node * associated_file;
uint64_t currentPtr;
uint64_t inode_info;
int permissions;

};

struct mapping_file_info_dirent
{
struct file_info_node* fil;
struct dirent dirnt;
uint64_t currentIter;
};

struct file_info_node* tarfs_root_node;

typedef struct DIR DIR;

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);
int closedir(DIR *dirp);

void * isValidFileInPath(char * path);
void locateAndConstructFileNode(char * path, uint64_t startNode,uint64_t endNode, int isFile);
void * setupInitialTarFileSystem();
void construct_file_info_node(struct file_info_node * targetInfo, struct file_info_node* self, struct file_info_node* parent);
struct file_info_node * get_tarfs_root_node();



#endif
