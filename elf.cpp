﻿#include <cstdio>
#include <cstdint>
#include <cstring>
#include <functional>
#include <algorithm>
#include <vector>
#include <map>

#ifdef _WIN32
#   include <windows.h>
#else  // _WIN32
#   include <sys/mman.h>
#   include <dlfcn.h>
#   include <link.h>
#endif // _WIN32

#if defined(_M_X64) || defined(_LP64)
#   define dpX64
#endif
#if defined(_WIN32)
#   define dpWindows
#else
#   define dpPOSIX
#endif

#ifdef dpWindows
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Word;
typedef int32_t  Elf32_Sword;
typedef uint64_t Elf32_Xword;
typedef int64_t  Elf32_Sxword;
typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Section;
typedef uint32_t Elf32_Symndx;
typedef unsigned char Elf32_Byte;

typedef uint16_t Elf64_Half;
typedef uint32_t Elf64_Word;
typedef int32_t  Elf64_Sword;
typedef uint64_t Elf64_Xword;
typedef int64_t  Elf64_Sxword;
typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Section;
typedef uint64_t Elf64_Symndx;
typedef unsigned char Elf64_Byte;

#define EI_NIDENT (16)

#define EI_CLASS        4               /* File class byte index */
#define ELFCLASSNONE    0               /* Invalid class */
#define ELFCLASS32      1               /* 32-bit objects */
#define ELFCLASS64      2               /* 64-bit objects */

/* Special section indices.  */
#define SHN_UNDEF       0               /* Undefined section */
#define SHN_LORESERVE   0xff00          /* Start of reserved indices */
#define SHN_LOPROC      0xff00          /* Start of processor-specific */
#define SHN_HIPROC      0xff1f          /* End of processor-specific */
#define SHN_ABS         0xfff1          /* Associated symbol is absolute */
#define SHN_COMMON      0xfff2          /* Associated symbol is common */
#define SHN_HIRESERVE   0xffff          /* End of reserved indices */

/* Legal values for sh_type (section type).  */
#define SHT_NULL        0               /* Section header table entry unused */
#define SHT_PROGBITS    1               /* Program data */
#define SHT_SYMTAB      2               /* Symbol table */
#define SHT_STRTAB      3               /* String table */
#define SHT_RELA        4               /* Relocation entries with addends */
#define SHT_HASH        5               /* Symbol hash table */
#define SHT_DYNAMIC     6               /* Dynamic linking information */
#define SHT_NOTE        7               /* Notes */
#define SHT_NOBITS      8               /* Program space with no data (bss) */
#define SHT_REL         9               /* Relocation entries, no addends */
#define SHT_SHLIB       10              /* Reserved */
#define SHT_DYNSYM      11              /* Dynamic linker symbol table */
#define SHT_NUM         12              /* Number of defined types.  */
#define SHT_LOSUNW      0x6ffffffd      /* Sun-specific low bound.  */
#define SHT_GNU_verdef  0x6ffffffd      /* Version definition section.  */
#define SHT_GNU_verneed 0x6ffffffe      /* Version needs section.  */
#define SHT_GNU_versym  0x6fffffff      /* Version symbol table.  */
#define SHT_HISUNW      0x6fffffff      /* Sun-specific high bound.  */
#define SHT_LOPROC      0x70000000      /* Start of processor-specific */
#define SHT_HIPROC      0x7fffffff      /* End of processor-specific */
#define SHT_LOUSER      0x80000000      /* Start of application-specific */
#define SHT_HIUSER      0x8fffffff      /* End of application-specific */

/* Legal values for sh_flags (section flags).  */
#define SHF_WRITE       (1 << 0)        /* Writable */
#define SHF_ALLOC       (1 << 1)        /* Occupies memory during execution */
#define SHF_EXECINSTR   (1 << 2)        /* Executable */
#define SHF_MASKPROC    0xf0000000      /* Processor-specific */



// Segment types.
enum {
    PT_NULL    = 0, // Unused segment.
    PT_LOAD    = 1, // Loadable segment.
    PT_DYNAMIC = 2, // Dynamic linking information.
    PT_INTERP  = 3, // Interpreter pathname.
    PT_NOTE    = 4, // Auxiliary information.
    PT_SHLIB   = 5, // Reserved.
    PT_PHDR    = 6, // The program header table itself.
    PT_TLS     = 7, // The thread-local storage template.
    PT_LOOS    = 0x60000000, // Lowest operating system-specific pt entry type.
    PT_HIOS    = 0x6fffffff, // Highest operating system-specific pt entry type.
    PT_LOPROC  = 0x70000000, // Lowest processor-specific program hdr entry type.
    PT_HIPROC  = 0x7fffffff, // Highest processor-specific program hdr entry type.

    // x86-64 program header types.
    // These all contain stack unwind tables.
    PT_GNU_EH_FRAME  = 0x6474e550,
    PT_SUNW_EH_FRAME = 0x6474e550,
    PT_SUNW_UNWIND   = 0x6464e550,

    PT_GNU_STACK  = 0x6474e551, // Indicates stack executability.
    PT_GNU_RELRO  = 0x6474e552, // Read-only after relocation.

    // ARM program header types.
    PT_ARM_ARCHEXT = 0x70000000, // Platform architecture compatibility info
    // These all contain stack unwind tables.
    PT_ARM_EXIDX   = 0x70000001,
    PT_ARM_UNWIND  = 0x70000001
};

// Segment flag bits.
enum {
    PF_X        = 1,         // Execute
    PF_W        = 2,         // Write
    PF_R        = 4,         // Read
    PF_MASKOS   = 0x0ff00000,// Bits for operating system-specific semantics.
    PF_MASKPROC = 0xf0000000 // Bits for processor-specific semantics.
};

// X86_64 relocations.
enum {
    R_X86_64_NONE       = 0,
    R_X86_64_64         = 1,
    R_X86_64_PC32       = 2,
    R_X86_64_GOT32      = 3,
    R_X86_64_PLT32      = 4,
    R_X86_64_COPY       = 5,
    R_X86_64_GLOB_DAT   = 6,
    R_X86_64_JUMP_SLOT  = 7,
    R_X86_64_RELATIVE   = 8,
    R_X86_64_GOTPCREL   = 9,
    R_X86_64_32         = 10,
    R_X86_64_32S        = 11,
    R_X86_64_16         = 12,
    R_X86_64_PC16       = 13,
    R_X86_64_8          = 14,
    R_X86_64_PC8        = 15,
    R_X86_64_DTPMOD64   = 16,
    R_X86_64_DTPOFF64   = 17,
    R_X86_64_TPOFF64    = 18,
    R_X86_64_TLSGD      = 19,
    R_X86_64_TLSLD      = 20,
    R_X86_64_DTPOFF32   = 21,
    R_X86_64_GOTTPOFF   = 22,
    R_X86_64_TPOFF32    = 23,
    R_X86_64_PC64       = 24,
    R_X86_64_GOTOFF64   = 25,
    R_X86_64_GOTPC32    = 26,
    R_X86_64_GOT64      = 27,
    R_X86_64_GOTPCREL64 = 28,
    R_X86_64_GOTPC64    = 29,
    R_X86_64_GOTPLT64   = 30,
    R_X86_64_PLTOFF64   = 31,
    R_X86_64_SIZE32     = 32,
    R_X86_64_SIZE64     = 33,
    R_X86_64_GOTPC32_TLSDESC = 34,
    R_X86_64_TLSDESC_CALL    = 35,
    R_X86_64_TLSDESC    = 36,
    R_X86_64_IRELATIVE  = 37
};

// i386 relocations.
enum {
    R_386_NONE          = 0,
    R_386_32            = 1,
    R_386_PC32          = 2,
    R_386_GOT32         = 3,
    R_386_PLT32         = 4,
    R_386_COPY          = 5,
    R_386_GLOB_DAT      = 6,
    R_386_JUMP_SLOT     = 7,
    R_386_RELATIVE      = 8,
    R_386_GOTOFF        = 9,
    R_386_GOTPC         = 10,
    R_386_32PLT         = 11,
    R_386_TLS_TPOFF     = 14,
    R_386_TLS_IE        = 15,
    R_386_TLS_GOTIE     = 16,
    R_386_TLS_LE        = 17,
    R_386_TLS_GD        = 18,
    R_386_TLS_LDM       = 19,
    R_386_16            = 20,
    R_386_PC16          = 21,
    R_386_8             = 22,
    R_386_PC8           = 23,
    R_386_TLS_GD_32     = 24,
    R_386_TLS_GD_PUSH   = 25,
    R_386_TLS_GD_CALL   = 26,
    R_386_TLS_GD_POP    = 27,
    R_386_TLS_LDM_32    = 28,
    R_386_TLS_LDM_PUSH  = 29,
    R_386_TLS_LDM_CALL  = 30,
    R_386_TLS_LDM_POP   = 31,
    R_386_TLS_LDO_32    = 32,
    R_386_TLS_IE_32     = 33,
    R_386_TLS_LE_32     = 34,
    R_386_TLS_DTPMOD32  = 35,
    R_386_TLS_DTPOFF32  = 36,
    R_386_TLS_TPOFF32   = 37,
    R_386_TLS_GOTDESC   = 39,
    R_386_TLS_DESC_CALL = 40,
    R_386_TLS_DESC      = 41,
    R_386_IRELATIVE     = 42,
    R_386_NUM           = 43
};


typedef enum {
    CK_NULL       = 0,        /* Invalid, same as EK_NULL */
    CK_DEFAULT    = 0x30,     /* Default type of data for section */
    CK_ALIGN      = 0x31,     /* Alignment for described range */
    CK_INSTR      = 0x32,     /* Instructions */
    CK_DATA       = 0x33,     /* Non-address data */
    CK_SADDR_32   = 0x34,     /* Simple 32-bit addresses */
    CK_GADDR_32   = 0x35,     /* GP-relative 32-bit addresses */
    CK_CADDR_32   = 0x36,     /* Complex 32-bit addresses */
    CK_SADDR_64   = 0x37,     /* Simple 64-bit addresses */
    CK_GADDR_64   = 0x38,     /* GP-relative 64-bit addresses */
    CK_CADDR_64   = 0x39,     /* Complex 64-bit addresses */
    CK_NO_XFORM   = 0x3a,     /* No transformations allowed in this range */
    CK_NO_REORDER = 0x3b,     /* No reordering allowed in this range */
    CK_GP_GROUP   = 0x3c,     /* Text/data in range with length given by
                                 second argument references GP group given
                                 by first. */
    CK_STUBS      = 0x3d      /* Text in range is stub code. ULEB */
} Elf_MIPS_Content_Kind;


// elf header
typedef struct
{
    unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
    Elf32_Half    e_type;                 /* Object file type */
    Elf32_Half    e_machine;              /* Architecture */
    Elf32_Word    e_version;              /* Object file version */
    Elf32_Addr    e_entry;                /* Entry point virtual address */
    Elf32_Off     e_phoff;                /* Program header table file offset */
    Elf32_Off     e_shoff;                /* Section header table file offset */
    Elf32_Word    e_flags;                /* Processor-specific flags */
    Elf32_Half    e_ehsize;               /* ELF header size in bytes */
    Elf32_Half    e_phentsize;            /* Program header table entry size */
    Elf32_Half    e_phnum;                /* Program header table entry count */
    Elf32_Half    e_shentsize;            /* Section header table entry size */
    Elf32_Half    e_shnum;                /* Section header table entry count */
    Elf32_Half    e_shstrndx;             /* Section header string table index */
} Elf32_Ehdr;
typedef struct
{
    unsigned char e_ident[EI_NIDENT];     /* Magic number and other info */
    Elf64_Half    e_type;                 /* Object file type */
    Elf64_Half    e_machine;              /* Architecture */
    Elf64_Word    e_version;              /* Object file version */
    Elf64_Addr    e_entry;                /* Entry point virtual address */
    Elf64_Off     e_phoff;                /* Program header table file offset */
    Elf64_Off     e_shoff;                /* Section header table file offset */
    Elf64_Word    e_flags;                /* Processor-specific flags */
    Elf64_Half    e_ehsize;               /* ELF header size in bytes */
    Elf64_Half    e_phentsize;            /* Program header table entry size */
    Elf64_Half    e_phnum;                /* Program header table entry count */
    Elf64_Half    e_shentsize;            /* Section header table entry size */
    Elf64_Half    e_shnum;                /* Section header table entry count */
    Elf64_Half    e_shstrndx;             /* Section header string table index */
} Elf64_Ehdr;


// program header
typedef struct
{
    Elf32_Word    p_type;                 /* Segment type */
    Elf32_Off     p_offset;               /* Segment file offset */
    Elf32_Addr    p_vaddr;                /* Segment virtual address */
    Elf32_Addr    p_paddr;                /* Segment physical address */
    Elf32_Word    p_filesz;               /* Segment size in file */
    Elf32_Word    p_memsz;                /* Segment size in memory */
    Elf32_Word    p_flags;                /* Segment flags */
    Elf32_Word    p_align;                /* Segment alignment */
} Elf32_Phdr;
typedef struct
{
    Elf64_Word    p_type;                 /* Segment type */
    Elf64_Off     p_offset;               /* Segment file offset */
    Elf64_Addr    p_vaddr;                /* Segment virtual address */
    Elf64_Addr    p_paddr;                /* Segment physical address */
    Elf64_Word    p_filesz;               /* Segment size in file */
    Elf64_Word    p_memsz;                /* Segment size in memory */
    Elf64_Word    p_flags;                /* Segment flags */
    Elf64_Word    p_align;                /* Segment alignment */
} Elf64_Phdr;


// section header
typedef struct
{
    Elf32_Word    sh_name;                /* Section name (string tbl index) */
    Elf32_Word    sh_type;                /* Section type */
    Elf32_Word    sh_flags;               /* Section flags */
    Elf32_Addr    sh_addr;                /* Section virtual addr at execution */
    Elf32_Off     sh_offset;              /* Section file offset */
    Elf32_Word    sh_size;                /* Section size in bytes */
    Elf32_Word    sh_link;                /* Link to another section */
    Elf32_Word    sh_info;                /* Additional section information */
    Elf32_Word    sh_addralign;           /* Section alignment */
    Elf32_Word    sh_entsize;             /* Entry size if section holds table */
} Elf32_Shdr;
typedef struct
{
    Elf64_Word    sh_name;                /* Section name (string tbl index) */
    Elf64_Word    sh_type;                /* Section type */
    Elf64_Xword   sh_flags;               /* Section flags */
    Elf64_Addr    sh_addr;                /* Section virtual addr at execution */
    Elf64_Off     sh_offset;              /* Section file offset */
    Elf64_Xword   sh_size;                /* Section size in bytes */
    Elf64_Word    sh_link;                /* Link to another section */
    Elf64_Word    sh_info;                /* Additional section information */
    Elf64_Xword   sh_addralign;           /* Section alignment */
    Elf64_Xword   sh_entsize;             /* Entry size if section holds table */
} Elf64_Shdr;


// symbol entry
typedef struct
{
    Elf32_Word    st_name;                /* Symbol name (string tbl index) */
    Elf32_Addr    st_value;               /* Symbol value */
    Elf32_Word    st_size;                /* Symbol size */
    unsigned char st_info;                /* Symbol type and binding */
    unsigned char st_other;               /* No defined meaning, 0 */
    Elf32_Section st_shndx;               /* Section index */
} Elf32_Sym;
typedef struct
{
    Elf64_Word    st_name;                /* Symbol name (string tbl index) */
    unsigned char st_info;                /* Symbol type and binding */
    unsigned char st_other;               /* No defined meaning, 0 */
    Elf64_Section st_shndx;               /* Section index */
    Elf64_Addr    st_value;               /* Symbol value */
    Elf64_Xword   st_size;                /* Symbol size */
} Elf64_Sym;


typedef struct
{
    Elf32_Addr    r_offset;               /* Address */
    Elf32_Word    r_info;                 /* Relocation type and symbol index */
} Elf32_Rel;

typedef struct
{
    Elf32_Addr    r_offset;               /* Address */
    Elf32_Word    r_info;                 /* Relocation type and symbol index */
    Elf32_Sword   r_addend;               /* Addend */
} Elf32_Rela;


typedef struct
{
    Elf64_Addr    r_offset;
    Elf64_Byte    r_type;                 /* 1st relocation op type */
    Elf64_Byte    r_type2;                /* 2nd relocation op type */
    Elf64_Byte    r_type3;                /* 3rd relocation op type */
    Elf64_Byte    r_ssym;                 /* Special symbol */
    Elf64_Word    r_sym;                  /* Symbol index */
} Elf64_Rel;

typedef struct {
    Elf64_Addr    r_offset; // Location (file byte offset, or program virtual addr).
    Elf64_Xword   r_info;   // Symbol table index and type of relocation to apply.
    Elf64_Sxword  r_addend; // Compute value for relocatable field by adding this.
} Elf64_Rela;
#endif // dpWindows

inline Elf32_Word ElfRela_GetSymbol(const Elf32_Rela &v) { return (v.r_info >> 8); }
inline unsigned char ElfRela_GetType(const Elf32_Rela &v) { return (unsigned char) (v.r_info & 0x0ff); }
inline Elf64_Word ElfRela_GetSymbol(const Elf64_Rela &v) { return (v.r_info >> 32); }
inline Elf64_Word ElfRela_GetType(const Elf64_Rela &v) { return (Elf64_Word) (v.r_info & 0xffffffffL); }

#ifdef dpX64
    typedef Elf64_Ehdr Elf_Ehdr;
    typedef Elf64_Phdr Elf_Phdr;
    typedef Elf64_Shdr Elf_Shdr;
    typedef Elf64_Sym  Elf_Sym;
    typedef Elf64_Rel  Elf_Rel;
    typedef Elf64_Rela Elf_Rela;
#else  // dpX64
    typedef Elf32_Ehdr Elf_Ehdr;
    typedef Elf32_Phdr Elf_Phdr;
    typedef Elf32_Shdr Elf_Shdr;
    typedef Elf32_Sym  Elf_Sym;
    typedef Elf32_Rel  Elf_Rel;
    typedef Elf32_Rela Elf_Rela;
#endif // dpX64

struct dpModuleInfo
{
    void *addr;
    const char *name;
};

enum dpSymbolAttr
{
    SAttr_Local,
    SAttr_Weak,
    SAttr_Function,
    SAttr_Data,
    SAttr_Undef,
};

struct dpSymbol
{
    const char  *name;
    void        *addr;
    uint32_t    attributes; // combination of dpSymbolAttr
};
inline bool operator <(const dpSymbol &a, const dpSymbol &b) { return std::strcmp(a.name, b.name)<0; }
inline bool operator >(const dpSymbol &a, const dpSymbol &b) { return std::strcmp(a.name, b.name)>0; }
inline bool operator==(const dpSymbol &a, const dpSymbol &b) { return std::strcmp(a.name, b.name)==0; }


bool  dpMapFile(const char *path, void *&o_data, size_t &o_size, const std::function<void* (size_t)> &alloc);
void* dpAllocateRWX(size_t size);
void  dpDeallocate(void *ptr);

void dpEachModules(const std::function<void (const dpModuleInfo&)> &f);
bool dpElfEachSymbols(void *_elf_data, const std::function<void (const dpSymbol &)> &f);



class dpElfFile
{
public:
    dpElfFile();
    ~dpElfFile();
    bool loadFromFile(const char *path_to_elf);
    bool loadFromMemory(void *data);
    void unload();
    bool link();
    const dpSymbol* findSymbol(const char *name);

private:
    std::map<const char*, dpSymbol> m_symbols;
    void *m_elf_file;
    bool m_needs_deallocate;
};

class dpSymbolManager
{
public:
    static dpSymbolManager& getInstance();
    const dpSymbol* findSymbol(const char *name);

private:
    typedef std::map<const char*, dpSymbol> SymbolCont;
    dpSymbolManager();
    ~dpSymbolManager();
    void gatherSymbols();

    SymbolCont m_syms;
};


dpElfFile::dpElfFile()
    : m_elf_file(nullptr)
    , m_needs_deallocate(false)
{
}

dpElfFile::~dpElfFile()
{
    unload();
}

bool dpElfFile::loadFromFile( const char *path_to_elf )
{
    unload();

    size_t size = 0;
    void *data = nullptr;
    if(!dpMapFile(path_to_elf, data, size, dpAllocateRWX)) {
        return false;
    }

    printf("ElfFile::loadFromFile(): loading %s...\n", path_to_elf);
    if(loadFromMemory(data)) {
        m_needs_deallocate = true;
        printf("ElfFile::loadFromFile(): loading %s succeeded.\n", path_to_elf);
        return true;
    }
    else {
        dpDeallocate(data);
        printf("ElfFile::loadFromFile(): loading %s failed.\n", path_to_elf);
        return false;
    }
}

bool dpElfFile::loadFromMemory(void *data)
{
    char *elf_data = (char*)data;
    Elf_Ehdr *elf_header = (Elf_Ehdr*)elf_data;
    if(memcmp(elf_header->e_ident, "\x7F" "ELF", 4)!=0) {
        printf("ElfFile::loadFromMemory(): invalid elf file.\n");
        unload();
        return false;
    }
#ifdef dpX64
    if(elf_header->e_ident[EI_CLASS]!=ELFCLASS64) {
        printf("ElfFile::load(): not 64 bit elf file.\n");
        unload();
        return false;
    }
#else  // dpX64
    if(elf_header->e_ident[EI_CLASS]!=ELFCLASS32) {
        printf("ElfFile::load(): %s not 32 bit elf file.\n");
        unload();
        return false;
    }
#endif // dpX64

    dpElfEachSymbols(elf_data, [&](const dpSymbol &sym){ m_symbols[sym.name]=sym; });
    m_elf_file = data;
    return true;
}


void dpElfFile::unload()
{
    if(m_needs_deallocate && m_elf_file) {
        dpDeallocate(m_elf_file);
    }
    m_elf_file = nullptr;
    m_needs_deallocate = false;
    m_symbols.clear();
}

bool dpElfFile::link()
{
    if(!m_elf_file) { return false; }

    char *elf_data = (char*)m_elf_file;
    Elf_Ehdr *elf_header = (Elf_Ehdr*)elf_data;

    Elf_Shdr *sections = (Elf_Shdr*)(elf_data + elf_header->e_shoff);
    const char *section_str = (const char*)(elf_data + sections[elf_header->e_shstrndx].sh_offset);

    for(int isec=0; isec<elf_header->e_shnum; ++isec) {
        Elf_Shdr *section = sections + isec;
        const char *section_name = section_str + section->sh_name;

        if(section->sh_type==SHT_REL) {
            for(int irel=0; irel<section->sh_size; irel+=section->sh_entsize) {
                Elf_Rel *rela = (Elf_Rel*)(elf_data + section->sh_offset + irel);
            }
        }
        else if(section->sh_type==SHT_RELA) {
            Elf_Shdr *sym_section = sections + section->sh_link;
            const char *sym_str = (const char*)(elf_data + sections[sym_section->sh_link].sh_offset);
            for(int irel=0; irel<section->sh_size; irel+=section->sh_entsize) {
                Elf_Rela *rela = (Elf_Rela*)(elf_data + section->sh_offset + irel);
                uint32_t *target = (uint32_t*)(elf_data+sym_section->sh_addr + rela->r_offset);
                Elf_Sym *rel_symbol = (Elf_Sym*)(elf_data + sym_section->sh_offset + sym_section->sh_entsize*ElfRela_GetSymbol(*rela));
                const char *rel_name = sym_str + rel_symbol->st_name;
                void *rel_addr = nullptr;
                if(rel_symbol->st_shndx!=SHN_UNDEF) {
                    rel_addr = (void*)(elf_data + sections[rel_symbol->st_shndx].sh_offset + rel_symbol->st_value);
                }
                else {
                    // todo: resolve symbol
                }

                switch(ElfRela_GetType(*rela)) {
#ifdef dpX64
                case R_X86_64_PC32:
                    break;

                case R_X86_64_PLT32:
                    break;
#else  // dpX64
#endif // dpX64
                }
            }
        }
    }

    return true;
}

const dpSymbol* dpElfFile::findSymbol(const char *name)
{
    auto it = m_symbols.find(name);
    if(it!=m_symbols.end()) {
        return &it->second;
    }
    return nullptr;
}


dpSymbolManager& dpSymbolManager::getInstance()
{
    static dpSymbolManager s_inst;
    return s_inst;
}

dpSymbolManager::dpSymbolManager()
{
    gatherSymbols();
}

dpSymbolManager::~dpSymbolManager()
{
}

void dpSymbolManager::gatherSymbols()
{
#ifdef dpWindows
#else  // dpWindows

    dpEachModules([&](const dpModuleInfo &mod){
    });

#endif // dpWindows
}

const dpSymbol* dpSymbolManager::findSymbol(const char *name)
{
    auto it = m_syms.find(name);
    if(it!=m_syms.end()) {
        return &it->second;
    }
    return nullptr;
}


bool dpMapFile(const char *path, void *&o_data, size_t &o_size, const std::function<void* (size_t)> &alloc)
{
    o_data = nullptr;
    o_size = 0;
    if(FILE *f=fopen(path, "rb")) {
        fseek(f, 0, SEEK_END);
        o_size = ftell(f);
        if(o_size > 0) {
            o_data = alloc(o_size);
            fseek(f, 0, SEEK_SET);
            fread(o_data, 1, o_size, f);
        }
        fclose(f);
        return true;
    }
    return false;
}

void* dpAllocateRWX(size_t size)
{
#ifdef dpWindows
    return ::VirtualAlloc(nullptr, size, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
#else  // dpWindows
    return ::mmap(nullptr, size, PROT_EXEC|PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#endif // dpWindows
}

void dpDeallocate(void *ptr)
{
#ifdef dpWindows
    ::VirtualFree(ptr, 0, MEM_RELEASE);
#else  // dpWindows
    ::munmap(ptr, 0);
#endif // dpWindows
}

void dpEachModules(const std::function<void (const dpModuleInfo&)> &f)
{
#ifdef dpWindows
#else  // dpWindows
    link_map *modules = (link_map*)dlopen(nullptr, RTLD_NOW);
    while(modules) {
        if(modules->l_name && *modules->l_name) {
            dpModuleInfo mod = {(void*)modules->l_addr, (const char*)modules->l_name};
            f(mod);
        }
        modules = (link_map*)modules->l_next;
    }
#endif // dpWindows
}

bool dpElfEachSymbols(void *_elf_data, const std::function<void (const dpSymbol &)> &f)
{
    if(!_elf_data) { return false; }
    char *elf_data = (char*)_elf_data;
    Elf_Ehdr *elf_header = (Elf_Ehdr*)elf_data;

    size_t size = 1024*64;
    while(size < elf_header->e_shoff) { size*=2; }
    if(mprotect((void*)elf_data, size, PROT_READ|PROT_WRITE|PROT_EXEC)==-1) {
        printf("dpElfEachSymbols() failed\n");
        return false;
    }

    Elf_Phdr *elf_pheader = nullptr;
    for(int i=0; i<elf_header->e_phnum; ++i) {
        Elf_Phdr *ph = (Elf_Phdr*)(elf_data + elf_header->e_phoff) + i;
        if(ph->p_type==PT_LOAD && (ph->p_flags&PF_X)!=0) {
            elf_pheader = ph;
            break;
        }
    }

    Elf_Shdr *sections = (Elf_Shdr*)(elf_data + elf_header->e_shoff);
    for(int isec=0; isec<elf_header->e_shnum; ++isec) {
        Elf_Shdr *section = sections + isec;

        if(section->sh_type==SHT_SYMTAB || section->sh_type==SHT_DYNSYM) {
            const char *sym_str = (const char*)(elf_data + sections[section->sh_link].sh_offset);
            for(int isym=0; isym<section->sh_size; isym+=section->sh_entsize) {
                Elf_Sym *symbol = (Elf_Sym*)(elf_data + section->sh_offset + isym);
                const char *sym_name = sym_str + symbol->st_name;
                if(symbol->st_name==0 || symbol->st_shndx>=elf_header->e_shnum) { continue; }

                Elf_Shdr *sym_section = sections + symbol->st_shndx;
                char *sym_addr = nullptr;
                if(symbol->st_shndx!=SHN_UNDEF) {
                    sym_addr = elf_data + sym_section->sh_offset + symbol->st_value;
                    if(elf_pheader) {
                        sym_addr -= elf_pheader->p_vaddr;
                    }
                    dpSymbol sym = {sym_name, sym_addr};
                    f(sym);
                }
            }
        }
    }
    return true;
}



int main(int argc, char *argv[])
{
    dpEachModules([](const dpModuleInfo &mod){
        printf("module [%p] %s\n", mod.addr, mod.name);
        dpElfEachSymbols(mod.addr, [](const dpSymbol &sym){
            //printf("  [%p] %s\n", sym.addr, sym.name);
        });
    });

    if(argc<2) {
        printf("usage: %s [path to .o]\n", argv[0]);
        return 0;
    }

    dpElfFile elf;
    if(elf.loadFromFile(argv[1])) {
        elf.link();
        typedef int (*add_t)(int, int);
        if(const dpSymbol *sym = elf.findSymbol("add")) {
            add_t add = (add_t)sym->addr;
            printf("add(1,2): %d\n", add(1,2));
        }
    }
}
