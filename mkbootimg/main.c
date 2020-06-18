/*
 * mkbootimg/main.c
 *
 * Copyright (C) 2017 - 2020 bzt (bztsrc@gitlab)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the BOOTBOOT Protocol package.
 * @brief Bootable image creator main file
 *
 */
#include "main.h"
#include "fs.h"

#ifdef __WIN32__
#define ISHH(x) ((((x)>>30)&0xFFFFFFFF)==0xFFFFFFFF)
#else
#define ISHH(x) (((x)>>30)==0x3FFFFFFFF)
#endif
#if defined(MACOSX) || __WORDSIZE == 32
#define LL "ll"
#else
#define LL "l"
#endif

/**
 * Parse the mkbootimg json configuration file
 */
void parsejson(char *json)
{
    char *tmp, key[64];
    int i;
    tmp = json_get(json, "diskguid"); getguid(tmp, &diskguid); free(tmp);
    tmp = json_get(json, "disksize"); if(tmp) { disk_size = atoi(tmp); } free(tmp);
    tmp = json_get(json, "align"); if(tmp) { disk_align = atoi(tmp); } free(tmp);
    tmp = json_get(json, "initrd.type");
    if(!tmp || !*tmp) { fprintf(stderr,"mkbootimg: initrd type not specified in json\r\n"); exit(1); }
    for(i = 0; fsdrv[i].name && fsdrv[i].add; i++)
        if(!strcmp(tmp, fsdrv[i].name)) { rd_open = fsdrv[i].open; rd_add = fsdrv[i].add; rd_close = fsdrv[i].close; break; }
    if(!rd_add) {
        fprintf(stderr,"mkbootimg: invalid initrd type %s, valid values:", tmp);
        for(i = 0; fsdrv[i].name && fsdrv[i].add; i++) fprintf(stderr,"%s %s",i ? "," : "",fsdrv[i].name);
        fprintf(stderr,"\r\n");
        exit(1);
    }
    free(tmp);
    tmp = json_get(json, "initrd.gzip");
    if(tmp && tmp[0] != '1' && tmp[0] != 't') initrd_gzip = 0;
    free(tmp);
    memset(initrd_dir, 0, NUMARCH*sizeof(void*));
    memset(initrd_buf, 0, NUMARCH*sizeof(void*));
    for(i = 0; i < NUMARCH; i++) {
        sprintf(key, "initrd.file.%d", i);
        tmp = json_get(json, key);
        if(!i && (!tmp || !*tmp)) tmp = json_get(json, "initrd.file");
        if(tmp && *tmp) {
            initrd_buf[i] = readfileall(tmp);
            initrd_size[i] = read_size;
            if(!initrd_buf[i] || !read_size) { fprintf(stderr,"mkbootimg: unable to read initrd image from %s\r\n",tmp); exit(1); }
            free(tmp);
        } else {
            sprintf(key, "initrd.directory.%d", i);
            initrd_dir[i] = json_get(json, key);
            if(!i && !initrd_dir[i]) initrd_dir[i] = json_get(json, "initrd.directory");
        }
        if(!initrd_dir[i] && !initrd_buf[i]) break;
    }
    if((!initrd_dir[0] || !initrd_dir[0][0]) && !initrd_buf[0]) { fprintf(stderr,"mkbootimg: initrd not specified in json\r\n"); exit(1); }
    tmp = json_get(json, "config");
    if(tmp && *tmp) {
        config = (char*)readfileall(tmp);
        if(!config || !*config) { fprintf(stderr,"mkbootimg: unable to read BOOTBOOT configuration from %s\r\n",tmp); exit(1); }
        if(read_size > 4095) { fprintf(stderr,"mkbootimg: BOOTBOOT configuration file %s is bigger than 4095\r\n",tmp); exit(1); }
    }
    free(tmp);
    tmp = json_get(json, "iso9660"); if(tmp && (*tmp=='1' || *tmp=='t' || *tmp=='y')) { iso9660 = 1; } free(tmp);
    tmp = json_get(json, "partitions.0.type");
    if(!tmp || !*tmp) { fprintf(stderr,"mkbootimg: partitions array or boot partition's type not specified in json\r\n"); exit(1); }
    if(tmp && !memcmp(tmp, "fat32", 5)) boot_fat = 32;
    free(tmp);
    tmp = json_get(json, "partitions.0.size");
    boot_size = atoi(tmp); free(tmp);
    if(!diskguid.Data1) diskguid.Data1 = crc32(0,(uint8_t*)&t, sizeof(time_t)) ^ 0x08040201;
    if(!diskguid.Data2 && !diskguid.Data3) {
        ((uint32_t*)&diskguid)[1] = crc32(0,(uint8_t*)&diskguid.Data1, 4);
        ((uint32_t*)&diskguid)[2] = crc32(0,(uint8_t*)&diskguid.Data2, 4) ^ (unsigned long int)t;
        ((uint32_t*)&diskguid)[3] = crc32(0,(uint8_t*)&diskguid.Data3, 4);
    }
}

/**
 * Parse the BOOTBOOT configuration file
 */
void parseconfig()
{
    char *ptr = config, *e;
    while(ptr && *ptr) {
        if(ptr[0]==' '||ptr[0]=='\t'||ptr[0]=='\r'||ptr[0]=='\n') { ptr++; continue; }
        if((ptr[0]=='/'&&ptr[1]=='/')||ptr[0]=='#') { while(ptr[0]!=0 && ptr[0]!='\r' && ptr[0]!='\n') ptr++; }
        if(ptr[0]=='/'&&ptr[1]=='*') { ptr+=2; while(ptr[0]!=0 && ptr[-1]!='*' && ptr[0]!='/') ptr++; }
        if(!memcmp(ptr, "kernel=", 7)) {
            ptr += 7; for(e = ptr; *e && *e != '\r' && *e != '\n'; e++);
            kernelname = malloc(e - ptr + 1);
            if(!kernelname) { fprintf(stderr,"mkbootimg: unable to allocate memory\r\n"); exit(1); }
            memcpy(kernelname, ptr, e - ptr); kernelname[e - ptr] = 0;
            break;
        }
        ptr++;
    }
    if(!kernelname || !*kernelname) {
        kernelname = malloc(10);
        if(!kernelname) { fprintf(stderr,"mkbootimg: unable to allocate memory\r\n"); exit(1); }
        strcpy(kernelname, "sys/core");
    }
}

/**
 * Parse the ELF or PE kernel executable
 */
void parsekernel(int idx, unsigned char *data, int v)
{
    Elf64_Ehdr *ehdr;
    Elf64_Phdr *phdr;
    Elf64_Shdr *shdr, *strt, *sym_sh = NULL, *str_sh = NULL;
    Elf64_Sym *sym = NULL, *s;
    pe_hdr *pehdr;
    pe_sym *ps;
    uint32_t i, n = 0, bss = 0, strsz = 0, syment = 0, ma, fa;
    uint64_t core_ptr = 0, core_size = 0, core_addr = 0, entrypoint = 0, mm_addr = 0, fb_addr = 0, bb_addr = 0, env_addr = 0;
    char *strtable, *name;
    ehdr=(Elf64_Ehdr *)(data);
    pehdr=(pe_hdr*)(data + ((mz_hdr*)(data))->peaddr);
    if(v) printf("File format:  ");
    if((!memcmp(ehdr->e_ident,ELFMAG,SELFMAG)||!memcmp(ehdr->e_ident,"OS/Z",4)) &&
        ehdr->e_ident[EI_CLASS]==ELFCLASS64 && ehdr->e_ident[EI_DATA]==ELFDATA2LSB) {
        if(v) printf("ELF64\r\nArchitecture: %s\r\n", ehdr->e_machine==EM_AARCH64 ? "AArch64" : (ehdr->e_machine==EM_X86_64 ?
            "x86_64" : "invalid"));
        if(ehdr->e_machine == EM_AARCH64) { ma = 2*1024*1024-1; fa = 4095; initrd_arch[idx] = 1; } else
        if(ehdr->e_machine == EM_X86_64)  { ma = 4095; fa = 2*1024*1024-1; initrd_arch[idx] = 2; } else
        { fprintf(stderr,"mkbootimg: invalid architecture in kernel\r\n"); exit(1); }
        phdr=(Elf64_Phdr *)((uint8_t *)ehdr+ehdr->e_phoff);
        for(i=0;i<ehdr->e_phnum;i++){
            if(phdr->p_type==PT_LOAD) {
                n++;
                core_size = phdr->p_filesz + (ehdr->e_type==3?0x4000:0);
                bss = phdr->p_memsz - core_size;
                core_addr = phdr->p_vaddr;
                entrypoint = ehdr->e_entry;
                break;
            }
            phdr=(Elf64_Phdr *)((uint8_t *)phdr+ehdr->e_phentsize);
        }
        if(n != 1) { fprintf(stderr,"mkbootimg: more than one loadable segment in kernel\r\n"); exit(1); }
        if(v) printf("Entry point:  %08" LL "x ", entrypoint);
        if(entrypoint < core_addr || entrypoint > core_addr+core_size)
            { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: entry point is not in text segment\r\n"); exit(1); }
        if(ehdr->e_shoff > 0) {
            shdr = (Elf64_Shdr *)((uint8_t *)ehdr + ehdr->e_shoff);
            strt = (Elf64_Shdr *)((uint8_t *)shdr+(uint64_t)ehdr->e_shstrndx*(uint64_t)ehdr->e_shentsize);
            strtable = (char *)ehdr + strt->sh_offset;
            for(i = 0; i < ehdr->e_shnum; i++){
                /* checking shdr->sh_type is not enough, there can be multiple SHT_STRTAB records... */
                if(!memcmp(strtable + shdr->sh_name, ".symtab", 8)) sym_sh = shdr;
                if(!memcmp(strtable + shdr->sh_name, ".strtab", 8)) str_sh = shdr;
                shdr = (Elf64_Shdr *)((uint8_t *)shdr + ehdr->e_shentsize);
            }
            if(str_sh && sym_sh) {
                strtable = (char *)ehdr + str_sh->sh_offset; strsz = str_sh->sh_size;
                sym = (Elf64_Sym *)((uint8_t*)ehdr + sym_sh->sh_offset); syment = sym_sh->sh_entsize;
                if(str_sh->sh_offset && strsz > 0 && sym_sh->sh_offset && syment > 0)
                    for(s = sym, i = 0; i<(strtable-(char*)sym)/syment && s->st_name < strsz; i++, s++) {
                        if(!memcmp(strtable + s->st_name, "bootboot", 9)) bb_addr = s->st_value;
                        if(!memcmp(strtable + s->st_name, "environment", 12)) env_addr = s->st_value;
                        if(!memcmp(strtable + s->st_name, "mmio", 4)) mm_addr = s->st_value;
                        if(!memcmp(strtable + s->st_name, "fb", 3)) fb_addr = s->st_value;
                    }
            }
        }
    } else
    if(((mz_hdr*)(data))->magic==MZ_MAGIC && ((mz_hdr*)(data))->peaddr<65536 && pehdr->magic == PE_MAGIC &&
        pehdr->file_type == PE_OPT_MAGIC_PE32PLUS) {
        if(v) printf("PE32+\r\nArchitecture: %s\r\n", pehdr->machine == IMAGE_FILE_MACHINE_ARM64 ? "AArch64" : (
            pehdr->machine == IMAGE_FILE_MACHINE_AMD64 ? "x86_64" : "invalid"));
        if(pehdr->machine == IMAGE_FILE_MACHINE_ARM64) { ma = 2*1024*1024-1; fa = 4095; initrd_arch[idx] = 1; } else
        if(pehdr->machine == IMAGE_FILE_MACHINE_AMD64) { ma = 4095; fa = 2*1024*1024-1; initrd_arch[idx] = 2; } else
        { fprintf(stderr,"mkbootimg: invalid architecture in kernel\r\n"); exit(1); }
        core_size = (pehdr->entry_point-pehdr->code_base) + pehdr->text_size + pehdr->data_size;
        bss = pehdr->bss_size;
        core_addr = (int64_t)pehdr->code_base;
        entrypoint = (int64_t)pehdr->entry_point;
        if(v) printf("Entry point:  %08" LL "x ", entrypoint);
        if(entrypoint < core_addr || entrypoint > core_addr+pehdr->text_size)
            { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: entry point is not in text segment\r\n"); exit(1); }
        if(pehdr->sym_table > 0 && pehdr->numsym > 0) {
            strtable = (char *)pehdr + pehdr->sym_table + pehdr->numsym * 18 + 4;
            for(i = 0; i < pehdr->numsym; i++) {
                ps = (pe_sym*)((uint8_t *)pehdr + pehdr->sym_table + i * 18);
                name = !ps->iszero ? (char*)&ps->iszero : strtable + ps->nameoffs;
                if(!memcmp(name, "bootboot", 9)) bb_addr = (int64_t)ps->value;
                if(!memcmp(name, "environment", 12)) env_addr = (int64_t)ps->value;
                if(!memcmp(name, "mmio", 4)) mm_addr = (int64_t)ps->value;
                if(!memcmp(name, "fb", 3)) fb_addr = (int64_t)ps->value;
                i += ps->auxsyms;
            }
        }
    } else {
        if(v) printf("unknown\r\n");
        fprintf(stderr,"mkbootimg: invalid kernel executable format. ELF64 or PE32+ only.\r\n");
        exit(1);
    }
    if(v) printf("OK\r\n");
    if(!mm_addr && !fb_addr && !bb_addr && !env_addr) {
        if(v) printf("Complies with BOOTBOOT Protocol Level 1, must use valid static addresses\r\n");
        free(data);
        return;
    }
    if(mm_addr) {
        if(v) printf("mmio:         %08" LL "x ", mm_addr);
        if(!ISHH(mm_addr)) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: symbol mmio not in the higher half top -1G\r\n"); exit(1); }
        if(mm_addr & ma) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: symbol mmio not %d bytes aligned\r\n", ma+1); exit(1); }
        if(v) printf("OK\r\n");
    }
    if(fb_addr) {
        if(v) printf("fb:           %08" LL "x ", fb_addr);
        if(!ISHH(fb_addr)) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: symbol fb not in the higher half top -1G\r\n"); exit(1); }
        if(fb_addr & fa) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: symbol fb not %d bytes aligned\r\n", fa+1); exit(1); }
        if(v) printf("OK\r\n");
    }
    if(bb_addr) {
        if(v) printf("bootboot:     %08" LL "x ", bb_addr);
        if(!ISHH(bb_addr)) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: symbol bootboot not in the higher half top -1G\r\n"); exit(1); }
        if(bb_addr & 4095) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: symbol bootboot not page aligned\r\n"); exit(1); }
        if(v) printf("OK\r\n");
    }
    if(env_addr) {
        if(v) printf("environment:  %08" LL "x ", env_addr);
        if(!ISHH(env_addr)) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: symbol environment not in the higher half top -1G\r\n"); exit(1); }
        if(env_addr & 4095) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: symbol environment not page aligned\r\n"); exit(1); }
        if(v) printf("OK\r\n");
    }
    if(v) printf("Load segment: %08" LL "x size %" LL "dK offs %" LL "x ", core_addr, (core_size + bss + 1024)/1024, core_ptr);
    if(!ISHH(core_addr)) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: loadable segment not in the higher half top -1G\r\n"); exit(1); }
    if(core_addr & 4095) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: loadable segment is not page aligned\r\n"); exit(1); }
    if(core_size + bss > 16 * 1024 * 1024) { if(v) { printf("invalid\r\n"); } fprintf(stderr,"mkbootimg: loadable segment is bigger than 16M\r\n"); exit(1); }
    if(v) printf("OK\r\n");
}

/**
 * Create a ROM image of the initrd
 */
void makerom()
{
    int i, size;
    unsigned char *buf, c=0;
    FILE *f;

    size=((initrd_size[0]+32+511)/512)*512;
    if(!initrd_buf[0] || size < 1) { fprintf(stderr,"mkbootimg: unable to read initrd\r\n"); exit(1); }
    buf=(unsigned char*)malloc(size+1);
    if(!buf) { fprintf(stderr,"mkbootimg: unable to allocate memory\r\n"); exit(1); }
    memset(buf, 0, size+1);
    /* Option ROM header */
    buf[0]=0x55; buf[1]=0xAA; buf[2]=(initrd_size[0]+32+511)/512;
    /* asm "xor ax,ax; retf" */
    buf[3]=0x31; buf[4]=0xC0; buf[5]=0xCB;
    /* identifier, size and data */
    memcpy(buf+8,"INITRD",6);
    memcpy(buf+16,&initrd_size[0],4);
    memcpy(buf+32,initrd_buf[0],initrd_size[0]);
    /* checksum */
    for(i=0;i<size;i++) c+=buf[i];
    buf[6]=(unsigned char)((int)(256-c));
    /* write out */
    f=fopen("initrd.rom","wb");
    if(!f) { fprintf(stderr,"mkbootimg: unable to write %s\r\n", "initrd.rom"); exit(3); }
    fwrite(buf,size,1,f);
    fclose(f);
    printf("mkbootimg: %s saved.\r\n", "initrd.rom");
}

/**
 * Main function
 */
int main(int argc, char **argv)
{
    Elf64_Ehdr *ehdr;
    pe_hdr *pehdr;
    int i, j;
    unsigned char *data;
    char kfn[32768];
    FILE *f;
    if(argc < 3 || argv[1]==NULL || argv[2] == NULL || !strcmp(argv[1],"help")) {
        printf( "BOOTBOOT mkbootimg utility - bztsrc@gitlab\r\n  https://gitlab.com/bztsrc/bootboot\r\n\r\n"
                "You can validate an ELF or PE executable for being BOOTBOOT compatible, otherwise\r\n"
                "creates a bootable hybrid disk / cdrom image for your hobby OS or Option ROM image.\r\n\r\n");
        printf( "Usage:\r\n"
                "  ./mkbootimg check <kernel elf or pe>\r\n"
                "  ./mkbootimg <configuration json> initrd.rom\r\n"
                "  ./mkbootimg <configuration json> bootpart.bin\r\n"
                "  ./mkbootimg <configuration json> <output disk image name>\r\n\r\n"
                "Examples:\n"
                "  ./mkbootimg check mykernel/mykernel.x86_64.elf\r\n"
                "  ./mkbootimg myos.json initrd.rom\r\n"
                "  ./mkbootimg myos.json bootpart.bin\r\n"
                "  ./mkbootimg myos.json myos.img\r\n"
                );
        return 0;
    }
    if(!strcmp(argv[1], "check")) {
        data = readfileall(argv[2]);
        if(!data || read_size < 16) { fprintf(stderr,"mkbootimg: unable to read kernel from %s\r\n",argv[2]); exit(1); }
        parsekernel(0, data, 1);
    } else {
        t = time(NULL);
        ts = gmtime(&t);
        memset(kfn, 0, sizeof(kfn)); /* <- make valgrind happy with sprintf */
        json = (char*)readfileall(argv[1]);
        if(!json || !*json) { fprintf(stderr,"mkbootimg: unable to read configuration json from %s\r\n",argv[1]); exit(1); }
        parsejson(json);
        parseconfig();
        for(i = 0; i < NUMARCH; i++)
            if(initrd_dir[i]) {
                sprintf(kfn, "%s/%s", initrd_dir[i], kernelname);
                data = readfileall(kfn);
                if(!data || read_size < 16) { fprintf(stderr,"mkbootimg: unable to read kernel from %s\r\n",kfn); exit(1); }
                parsekernel(i, data, 0);
                free(data);
                skipbytes = strlen(initrd_dir[i]) + 1;
                fs_base = NULL; fs_len = 0;
                if(rd_open) (*rd_open)();
                parsedir(initrd_dir[i], 0);
                if(rd_close) (*rd_close)();
                initrdcompress();
                initrd_buf[i] = fs_base;
                initrd_size[i] = fs_len;
            } else
            if(initrd_buf[i]) {
                for(j = 0, kfn[0] = 0; j < initrd_size[i] - 512; j++) {
                    ehdr=(Elf64_Ehdr *)(initrd_buf[i] + j);
                    pehdr=(pe_hdr*)(initrd_buf[i] + j + ((mz_hdr*)(initrd_buf[i] + j))->peaddr);
                    if(((!memcmp(ehdr->e_ident,ELFMAG,SELFMAG)||!memcmp(ehdr->e_ident,"OS/Z",4)) &&
                        ehdr->e_ident[EI_CLASS]==ELFCLASS64 && ehdr->e_ident[EI_DATA]==ELFDATA2LSB) ||
                        (((mz_hdr*)(initrd_buf[i] + j))->magic==MZ_MAGIC && ((mz_hdr*)(initrd_buf[i] + j))->peaddr<65536 &&
                        pehdr->magic == PE_MAGIC && pehdr->file_type == PE_OPT_MAGIC_PE32PLUS)) {
                            parsekernel(i, initrd_buf[i] + j, 0);
                            kfn[0] = 1;
                            break;
                        }
                }
                if(!kfn[0]) { fprintf(stderr,"mkbootimg: unable to locate kernel in initrd #%d\r\n",i+1); exit(1); }
            } else
                break;
        if(initrd_arch[1] && initrd_arch[1] == initrd_arch[0]) { initrd_size[1] = 0; initrd_arch[1] = 0; }
        if(!strcmp(argv[2], "initrd.rom")) makerom(); else
        if(!strcmp(argv[2], "initrd.bin")) {
            /* write out */
            f=fopen("initrd.bin","wb");
            if(!f) { fprintf(stderr,"mkbootimg: unable to write %s\r\n", "initrd.bin"); exit(3); }
            fwrite(initrd_buf[0],initrd_size[0],1,f);
            fclose(f);
            printf("mkbootimg: %s saved.\r\n", "initrd.bin");
        } else {
            esp_makepart();
            if(!strcmp(argv[2], "bootpart.bin")) {
                /* write out */
                f=fopen("bootpart.bin","wb");
                if(!f) { fprintf(stderr,"mkbootimg: unable to write %s\r\n", "bootpart.bin"); exit(3); }
                fwrite(esp,esp_size,1,f);
                fclose(f);
                printf("mkbootimg: %s saved.\r\n", "bootpart.bin");
            } else {
                gpt_maketable();
                img_write(argv[2]);
                free(gpt);
            }
            free(esp);
        }
        free(kernelname);
        free(initrd_buf[0]);
        if(initrd_buf[1]) free(initrd_buf[1]);
        free(json);
    }
    return 0;
}
