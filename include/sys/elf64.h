#ifndef _ELF64_H
#define _ELF64_H

#include<sys/defs.h>
#include<sys/process_memory_help.h>

#define EI_NIDENT 16
#define MAX_ARGS 10
#define ARG_MAX_SZ 100
static char gbl_args_array[MAX_ARGS][ARG_MAX_SZ];

typedef uint64_t Elf64_Addr;
typedef uint16_t Elf64_Half;
typedef uint64_t Elf64_Lword;
typedef uint64_t Elf64_Off;
typedef uint32_t Elf64_Sword;
typedef uint64_t Elf64_Sxword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;

typedef struct {
  unsigned char e_ident[EI_NIDENT];
  Elf64_Half    e_type;
  Elf64_Half    e_machine;
  Elf64_Word    e_version;
  Elf64_Addr    e_entry;
  Elf64_Off     e_phoff;
  Elf64_Off     e_shoff;
  Elf64_Word    e_flags;
  Elf64_Half    e_ehsize;
  Elf64_Half    e_phentsize;
  Elf64_Half    e_phnum;
  Elf64_Half    e_shentsize;
  Elf64_Half    e_shnum;
  Elf64_Half    e_shstrndx;
} Elf64_Ehdr;

typedef struct {
  Elf64_Word    p_type;
  Elf64_Word    p_flags;
  Elf64_Off     p_offset;
  Elf64_Addr    p_vaddr;
  Elf64_Addr    p_paddr;
  Elf64_Xword   p_filesz;
  Elf64_Xword   p_memsz;
  Elf64_Xword   p_align;
} Elf64_Phdr;

struct process_control_block* initiate_elf_process(char* binaryName,char* args[]);
void copy_arg_to_global_arg_array(int *no_of_args, char* target[], char * binaryName);
void push_args_to_user_stack(struct process_control_block * proc,int no_of_args);
struct process_control_block * load_elf_headers(char*args[], char*binaryName,process_control_block* current_process, Elf64_Ehdr* hdr);


#endif
