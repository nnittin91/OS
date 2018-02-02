#ifndef __SYSTEMCALL_MAIN_H__

#define __SYSTEMCALL_MAIN_H__

#define STDOUT 1
#define STDERR 2

#define START_FILE 0
#define CURRENT_LOCATION 1
#define EOFILE 2

void system_call_hdlr();
void lsPROC();

int clear_console_s();
int getprocessId_s();

int getparentProcessId_s();
uint64_t mmap_s(uint64_t target_ADDR,uint64_t size, uint64_t permissions);
uint64_t brk_s(uint64_t size);
int suspend_s(int timeInMilliSecs);
void exit_s();
uint64_t waitpid_s (uint64_t processID, uint64_t st, uint64_t flags);
uint64_t wait_s (uint64_t st);
uint64_t execvpe_s(char* binary, char* args[], char * envir[]);
int chdir_s(const char* directoryPath);
char* get_current_working_dir(char * buf);
int kill_PROC(int targetPID);
int fork_s();
int seek_s (int fileOrDir, int distance, int base);
int write_s(int targetLocation, uint64_t addr, int size);
int read_s(int targetFD, uint64_t output, uint64_t size);
void close_s(int fileDescriptor);
int open_s(char* path, uint64_t permissions);
int closedir_s(uint64_t* directory_entry);
struct dirent * readdir_s(uint64_t* directory_entry);
struct mapping_file_info_dirent* opendir_s(uint64_t* targetEntry, uint64_t* directory_entry);























#endif
