#pragma once
#include <stdint.h>

#define PT_NOTE 4              /* Auxiliary information */
#define PT_LOAD 1              /* Loadable program segment */
#define SHT_PROGBITS 1         /* Program data */
#define SHF_EXECINSTR (1 << 2) /* Executable */

/* Legal values for p_flags (segment flags).  */
#define PF_X		(1 << 0)	/* Segment is executable */

typedef struct {
  unsigned char e_ident[16]; /* Magic number and other info */
  uint16_t e_type;           /* Object file type */
  uint16_t e_machine;        /* Architecture */
  uint32_t e_version;        /* Object file version */
  uint64_t e_entry;          /* Entry point virtual address */
  uint64_t e_phoff;          /* Program header table file offset */
  uint64_t e_shoff;          /* Section header table file offset */
  uint32_t e_flags;          /* Processor-specific flags */
  uint16_t e_ehsize;         /* ELF header size in bytes */
  uint16_t e_phentsize;      /* Program header table entry size */
  uint16_t e_phnum;          /* Program header table entry count */
  uint16_t e_shentsize;      /* Section header table entry size */
  uint16_t e_shnum;          /* Section header table entry count */
  uint16_t e_shstrndx;       /* Section header string table index */
} Elf64_Ehdr;

typedef struct {
  uint32_t p_type;   /* Segment type */
  uint32_t p_flags;  /* Segment flags */
  uint64_t p_offset; /* Segment file offset */
  uint64_t p_vaddr;  /* Segment virtual address */
  uint64_t p_paddr;  /* Segment physical address */
  uint64_t p_filesz; /* Segment size in file */
  uint64_t p_memsz;  /* Segment size in memory */
  uint64_t p_align;  /* Segment alignment */
} Elf64_Phdr;

typedef struct {
  uint32_t sh_name;      /* section name (string tbl index)*/
  uint32_t sh_type;      /* section type*/
  uint64_t sh_flags;     /* section flags*/
  uint64_t sh_addr;      /* section virtual addr at execution*/
  uint64_t sh_offset;    /* section file offset*/
  uint64_t sh_size;      /* section size in bytes*/
  uint32_t sh_link;      /* link to another section*/
  uint32_t sh_info;      /* additional section information */
  uint64_t sh_addralign; /* section alignement*/
  uint64_t sh_entsize;   /* entry size if section holds table*/
} Elf64_Shdr;