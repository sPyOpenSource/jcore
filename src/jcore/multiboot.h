#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#include "types.h"

#define MULTIBOOT_MEMORY	(1L<<0)
#define MULTIBOOT_BOOT_DEVICE	(1L<<1)
#define MULTIBOOT_CMDLINE	(1L<<2)
#define MULTIBOOT_MODS		(1L<<3)
#define MULTIBOOT_AOUT_SYMS	(1L<<4)
#define MULTIBOOT_ELF_SHDR	(1L<<5)
#define MULTIBOOT_MEM_MAP	(1L<<6)

struct multiboot_module {
	char *mod_start;
	char *mod_end;
	char *string;
	unsigned reserved;
};

struct multiboot_info {
	unsigned flags;
	size_t mem_lower;
	size_t mem_upper;
	unsigned char boot_device[4];
	addr_t cmdline;
	unsigned mods_count;
	addr_t mods_addr;
	union {
		struct {
			size_t tabsize;
			size_t strsize;
			addr_t addr;
			unsigned reserved;
		} a;
		struct {
			unsigned num;
			size_t size;
			addr_t addr;
			unsigned shndx;
		} e;
	} syms;
	size_t mmap_count;
	addr_t mmap_addr;
};

extern struct multiboot_info boot_info;

#endif
