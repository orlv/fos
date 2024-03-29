#ifndef _SYS_ELF32_H_
#define _SYS_ELF32_H_

/*
 * ELF definitions common to all 32-bit architectures.
 */

typedef unsigned long Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned long Elf32_Off;
typedef signed long Elf32_Sword;
typedef unsigned long Elf32_Word;
typedef unsigned long Elf32_Size;
typedef Elf32_Off Elf32_Hashelt;

#define EI_NIDENT	16

/*
 * ELF header.
 */

typedef struct {
  unsigned char e_ident[EI_NIDENT];	/* File identification. */
  Elf32_Half e_type;		/* File type. */
  Elf32_Half e_machine;		/* Machine architecture. */
  Elf32_Word e_version;		/* ELF format version. */
  Elf32_Addr e_entry;		/* Entry point. */
  Elf32_Off e_phoff;		/* Program header file offset. */
  Elf32_Off e_shoff;		/* Section header file offset. */
  Elf32_Word e_flags;		/* Architecture-specific flags. */
  Elf32_Half e_ehsize;		/* Size of ELF header in bytes. */
  Elf32_Half e_phentsize;	/* Size of program header entry. */
  Elf32_Half e_phnum;		/* Number of program header entries. */
  Elf32_Half e_shentsize;	/* Size of section header entry. */
  Elf32_Half e_shnum;		/* Number of section header entries. */
  Elf32_Half e_shstrndx;	/* Section name strings section. */
} Elf32_Ehdr;

/*
 * Section header.
 */

typedef struct {
  Elf32_Word sh_name;		/* Section name (index into the
				   section header string table). */
  Elf32_Word sh_type;		/* Section type. */
  Elf32_Word sh_flags;		/* Section flags. */
  Elf32_Addr sh_addr;		/* Address in memory image. */
  Elf32_Off sh_offset;		/* Offset in file. */
  Elf32_Size sh_size;		/* Size in bytes. */
  Elf32_Word sh_link;		/* Index of a related section. */
  Elf32_Word sh_info;		/* Depends on section type. */
  Elf32_Size sh_addralign;	/* Alignment in bytes. */
  Elf32_Size sh_entsize;	/* Size of each entry in section. */
} Elf32_Shdr;

/*
 * Program header.
 */

typedef struct {
  Elf32_Word p_type;		/* Entry type. */
  Elf32_Off p_offset;		/* File offset of contents. */
  Elf32_Addr p_vaddr;		/* Virtual address in memory image. */
  Elf32_Addr p_paddr;		/* Physical address (not used). */
  Elf32_Size p_filesz;		/* Size of contents in file. */
  Elf32_Size p_memsz;		/* Size of contents in memory. */
  Elf32_Word p_flags;		/* Access permission flags. */
  Elf32_Size p_align;		/* Alignment in memory and file. */
} Elf32_Phdr;

/*
 * Dynamic structure.  The ".dynamic" section contains an array of them.
 */

typedef struct {
  Elf32_Sword d_tag;		/* Entry type. */
  union {
    Elf32_Size d_val;		/* Integer value. */
    Elf32_Addr d_ptr;		/* Address value. */
  } d_un;
} Elf32_Dyn;

/*
 * Relocation entries.
 */

/* Relocations that don't need an addend field. */
typedef struct {
  Elf32_Addr r_offset;		/* Location to be relocated. */
  Elf32_Word r_info;		/* Relocation type and symbol index. */
} Elf32_Rel;

/* Relocations that need an addend field. */
typedef struct {
  Elf32_Addr r_offset;		/* Location to be relocated. */
  Elf32_Word r_info;		/* Relocation type and symbol index. */
  Elf32_Sword r_addend;		/* Addend. */
} Elf32_Rela;

/* Macros for accessing the fields of r_info. */
#define ELF32_R_SYM(info)	((info) >> 8)
#define ELF32_R_TYPE(info)	((unsigned char)(info))

/* Macro for constructing r_info from field values. */
#define ELF32_R_INFO(sym, type)	(((sym) << 8) + (unsigned char)(type))

/*
 * Symbol table entries.
 */

typedef struct {
  Elf32_Word st_name;		/* String table index of name. */
  Elf32_Addr st_value;		/* Symbol value. */
  Elf32_Size st_size;		/* Size of associated object. */
  unsigned char st_info;	/* Type and binding information. */
  unsigned char st_other;	/* Reserved (not used). */
  Elf32_Half st_shndx;		/* Section index of symbol. */
} Elf32_Sym;

/* Macros for accessing the fields of st_info. */
#define ELF32_ST_BIND(info)		((info) >> 4)
#define ELF32_ST_TYPE(info)		((info) & 0xf)

/* Macro for constructing st_info from field values. */
#define ELF32_ST_INFO(bind, type)	(((bind) << 4) + ((type) & 0xf))

#define ELF32_FLAG_EXECUTABLE 1
#define ELF32_FLAG_WRITABLE   2
#define ELF32_FLAG_READABLE   4

#define ELF32_TYPE_NULL    0
#define ELF32_TYPE_LOAD    1
#define ELF32_TYPE_DYNAMIC 2
#define ELF32_TYPE_INTERPR 3
#define ELF32_TYPE_NOTE    4
#define ELF32_TYPE_SHLIB   5
#define ELF32_TYPE_PHDR    6

/* Flags for sh_flags. */
#define SHF_WRITE       0x1             /* Section contains writable data. */
#define SHF_ALLOC       0x2             /* Section occupies memory. */
#define SHF_EXECINSTR   0x4             /* Section contains instructions. */
#define SHF_TLS         0x400           /* Section contains TLS data. */
#define SHF_MASKPROC    0xf0000000      /* Reserved for processor-specific. */

#endif				/* !_SYS_ELF32_H_ */
