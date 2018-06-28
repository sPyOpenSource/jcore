/*
 * aarch64-rpi/bootboot.c
 *
 * Copyright (C) 2017 bzt (bztsrc@gitlab)
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
 * @brief Boot loader for the Raspberry Pi 3+ ARMv8
 *
 */
#define DEBUG 1
//#define SD_DEBUG DEBUG
//#define INITRD_DEBUG DEBUG
//#define EXEC_DEBUG DEBUG
//#define MEM_DEBUG DEBUG

#define CONSOLE UART0

#define NULL ((void*)0)
#define PAGESIZE 4096

#include "tinf.h"

/* get BOOTBOOT structure */
#include "../bootboot.h"

/* aligned buffers */
volatile uint32_t  __attribute__((aligned(16))) mbox[36];
/* we place these manually in linker script, gcc would otherwise waste lots of memory */
volatile uint8_t __attribute__((aligned(PAGESIZE))) __bootboot[PAGESIZE];
volatile uint8_t __attribute__((aligned(PAGESIZE))) __environment[PAGESIZE];
volatile uint8_t __attribute__((aligned(PAGESIZE))) __paging[23*PAGESIZE];
volatile uint8_t __attribute__((aligned(PAGESIZE))) __corestack[PAGESIZE];
#define __diskbuf __paging
extern volatile uint8_t _data;
extern volatile uint8_t _end;

/* forward definitions */
uint32_t color=0xC0C0C0;
void putc(char c);
void puts(char *s);

/*** ELF64 defines and structs ***/
#define ELFMAG      "\177ELF"
#define SELFMAG     4
#define EI_CLASS    4       /* File class byte index */
#define ELFCLASS64  2       /* 64-bit objects */
#define EI_DATA     5       /* Data encoding byte index */
#define ELFDATA2LSB 1       /* 2's complement, little endian */
#define PT_LOAD     1       /* Loadable program segment */
#define EM_AARCH64  183     /* ARM aarch64 architecture */

typedef struct
{
  unsigned char e_ident[16];/* Magic number and other info */
  uint16_t    e_type;         /* Object file type */
  uint16_t    e_machine;      /* Architecture */
  uint32_t    e_version;      /* Object file version */
  uint64_t    e_entry;        /* Entry point virtual address */
  uint64_t    e_phoff;        /* Program header table file offset */
  uint64_t    e_shoff;        /* Section header table file offset */
  uint32_t    e_flags;        /* Processor-specific flags */
  uint16_t    e_ehsize;       /* ELF header size in bytes */
  uint16_t    e_phentsize;    /* Program header table entry size */
  uint16_t    e_phnum;        /* Program header table entry count */
  uint16_t    e_shentsize;    /* Section header table entry size */
  uint16_t    e_shnum;        /* Section header table entry count */
  uint16_t    e_shstrndx;     /* Section header string table index */
} Elf64_Ehdr;

typedef struct
{
  uint32_t    p_type;         /* Segment type */
  uint32_t    p_flags;        /* Segment flags */
  uint64_t    p_offset;       /* Segment file offset */
  uint64_t    p_vaddr;        /* Segment virtual address */
  uint64_t    p_paddr;        /* Segment physical address */
  uint64_t    p_filesz;       /* Segment size in file */
  uint64_t    p_memsz;        /* Segment size in memory */
  uint64_t    p_align;        /* Segment alignment */
} Elf64_Phdr;

/*** PE32+ defines and structs ***/
#define MZ_MAGIC                    0x5a4d      /* "MZ" */
#define PE_MAGIC                    0x00004550  /* "PE\0\0" */
#define IMAGE_FILE_MACHINE_ARM64    0xaa64      /* ARM aarch64 architecture */
#define PE_OPT_MAGIC_PE32PLUS       0x020b      /* PE32+ format */
typedef struct
{
  uint16_t magic;         /* MZ magic */
  uint16_t reserved[29];  /* reserved */
  uint32_t peaddr;        /* address of pe header */
} mz_hdr;

typedef struct {
  uint32_t magic;         /* PE magic */
  uint16_t machine;       /* machine type */
  uint16_t sections;      /* number of sections */
  uint32_t timestamp;     /* time_t */
  uint32_t sym_table;     /* symbol table offset */
  uint32_t symbols;       /* number of symbols */
  uint16_t opt_hdr_size;  /* size of optional header */
  uint16_t flags;         /* flags */
  uint16_t file_type;     /* file type, PE32PLUS magic */
  uint8_t  ld_major;      /* linker major version */
  uint8_t  ld_minor;      /* linker minor version */
  uint32_t text_size;     /* size of text section(s) */
  uint32_t data_size;     /* size of data section(s) */
  uint32_t bss_size;      /* size of bss section(s) */
  int32_t entry_point;    /* file offset of entry point */
  int32_t code_base;      /* relative code addr in ram */
} pe_hdr;

/*** Raspberry Pi specific defines ***/
#define MMIO_BASE       0x3F000000

#define PM_RTSC         ((volatile uint32_t*)(MMIO_BASE+0x0010001c))
#define PM_WATCHDOG     ((volatile uint32_t*)(MMIO_BASE+0x00100024))
#define PM_WDOG_MAGIC   0x5a000000
#define PM_RTSC_FULLRST 0x00000020

#define GPFSEL0         ((volatile uint32_t*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile uint32_t*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile uint32_t*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile uint32_t*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile uint32_t*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile uint32_t*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile uint32_t*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile uint32_t*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile uint32_t*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile uint32_t*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile uint32_t*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile uint32_t*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile uint32_t*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile uint32_t*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile uint32_t*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile uint32_t*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile uint32_t*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile uint32_t*)(MMIO_BASE+0x0020009C))

#define UART0           0
#define UART0_DR        ((volatile uint32_t*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile uint32_t*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile uint32_t*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile uint32_t*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile uint32_t*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile uint32_t*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile uint32_t*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile uint32_t*)(MMIO_BASE+0x00201044))

#define UART1           1
#define AUX_ENABLE      ((volatile uint32_t*)(MMIO_BASE+0x00215004))
#define AUX_MU_IO       ((volatile uint32_t*)(MMIO_BASE+0x00215040))
#define AUX_MU_IER      ((volatile uint32_t*)(MMIO_BASE+0x00215044))
#define AUX_MU_IIR      ((volatile uint32_t*)(MMIO_BASE+0x00215048))
#define AUX_MU_LCR      ((volatile uint32_t*)(MMIO_BASE+0x0021504C))
#define AUX_MU_MCR      ((volatile uint32_t*)(MMIO_BASE+0x00215050))
#define AUX_MU_LSR      ((volatile uint32_t*)(MMIO_BASE+0x00215054))
#define AUX_MU_MSR      ((volatile uint32_t*)(MMIO_BASE+0x00215058))
#define AUX_MU_SCRATCH  ((volatile uint32_t*)(MMIO_BASE+0x0021505C))
#define AUX_MU_CNTL     ((volatile uint32_t*)(MMIO_BASE+0x00215060))
#define AUX_MU_STAT     ((volatile uint32_t*)(MMIO_BASE+0x00215064))
#define AUX_MU_BAUD     ((volatile uint32_t*)(MMIO_BASE+0x00215068))

/* timing stuff */
uint64_t cntfrq;
/* delay cnt clockcycles */
void delay(uint32_t cnt) { while(cnt--) { asm volatile("nop"); } }
/* delay cnt microsec */
void delaym(uint32_t cnt) {uint64_t t,r;asm volatile ("mrs %0, cntpct_el0" : "=r" (t));
    t+=((cntfrq/1000)*cnt)/1000;do{asm volatile ("mrs %0, cntpct_el0" : "=r" (r));}while(r<t);}

/* UART stuff */
void uart_send(uint32_t c) {
#if CONSOLE == UART1
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x20)); *AUX_MU_IO=c; *UART0_DR=c;
#else
    do{asm volatile("nop");}while(*UART0_FR&0x20); *UART0_DR=c;
#endif
}
char uart_getc() {char r;
#if CONSOLE == UART1
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));r=(char)(*AUX_MU_IO);
#else
    do{asm volatile("nop");}while(*UART0_FR&0x10);r=(char)(*UART0_DR);
#endif
    return r=='\r'?'\n':r;
}
void uart_hex(uint64_t d,int c) { uint32_t n;c<<=3;c-=4;for(;c>=0;c-=4){n=(d>>c)&0xF;n+=n>9?0x37:0x30;uart_send(n);} }
void uart_putc(char c) { if(c=='\n') uart_send((uint32_t)'\r'); uart_send((uint32_t)c); }
void uart_puts(char *s) { while(*s) uart_putc(*s++); }
void uart_dump(void *ptr,uint32_t l) {
    uint64_t a,b;
    unsigned char c;
    for(a=(uint64_t)ptr;a<(uint64_t)ptr+l*16;a+=16) {
        uart_hex(a,8); uart_puts(": ");
        for(b=0;b<16;b++) {
            uart_hex(*((unsigned char*)(a+b)),1);
            uart_putc(' ');
            if(b%4==3)
                uart_putc(' ');
        }
        for(b=0;b<16;b++) {
            c=*((unsigned char*)(a+b));
            uart_putc(c<32||c>=127?'.':c);
        }
        uart_putc('\n');
    }
}
void uart_exc(uint64_t idx, uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far, uint64_t sctlr, uint64_t tcr)
{
    register uint64_t r;
    asm volatile ("msr ttbr0_el1, %0;tlbi vmalle1" : : "r" ((uint64_t)&__paging+1));
    asm volatile ("dsb ish; isb; mrs %0, sctlr_el1" : "=r" (r));
    // set mandatory reserved bits
    r&=~((1<<12) |   // clear I, no instruction cache
         (1<<2));    // clear C, no cache at all
    asm volatile ("msr sctlr_el1, %0; isb" : : "r" (r));
    puts("\nBOOTBOOT-EXCEPTION");
    uart_puts(" #");
    uart_hex(idx,1);
    uart_puts(":\n  ESR_EL1 ");
    uart_hex(esr,8);
    uart_puts(" ELR_EL1 ");
    uart_hex(elr,8);
    uart_puts("\n SPSR_EL1 ");
    uart_hex(spsr,8);
    uart_puts(" FAR_EL1 ");
    uart_hex(far,8);
    uart_puts("\nSCTLR_EL1 ");
    uart_hex(sctlr,8);
    uart_puts(" TCR_EL1 ");
    uart_hex(tcr,8);
    uart_putc('\n');
    r=0; while(r!='\n' && r!=' ') r=uart_getc();
    asm volatile("dsb sy; isb");
    *PM_WATCHDOG = PM_WDOG_MAGIC | 1;
    *PM_RTSC = PM_WDOG_MAGIC | PM_RTSC_FULLRST;
    while(1);
}
#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile uint32_t*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile uint32_t*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile uint32_t*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile uint32_t*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile uint32_t*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile uint32_t*)(VIDEOCORE_MBOX+0x20))
#define MBOX_REQUEST    0
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000
#define MBOX_CH_POWER   0
#define MBOX_CH_FB      1
#define MBOX_CH_VUART   2
#define MBOX_CH_VCHIQ   3
#define MBOX_CH_LEDS    4
#define MBOX_CH_BTNS    5
#define MBOX_CH_TOUCH   6
#define MBOX_CH_COUNT   7
#define MBOX_CH_PROP    8

/* mailbox functions */
void mbox_write(uint8_t ch, volatile uint32_t *mbox)
{
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    *MBOX_WRITE = (((uint32_t)((uint64_t)mbox)&~0xF) | (ch&0xF));
}
uint32_t mbox_read(uint8_t ch)
{
    uint32_t r;
    while(1) {
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        r=*MBOX_READ;
        if((uint8_t)(r&0xF)==ch)
            return (r&~0xF);
    }
}
uint8_t mbox_call(uint8_t ch, volatile uint32_t *mbox)
{
    mbox_write(ch,mbox);
    return mbox_read(ch)==(uint32_t)((uint64_t)mbox) && mbox[1]==MBOX_RESPONSE;
}

/* string.h */
uint32_t strlen(unsigned char *s) { uint32_t n=0; while(*s++) n++; return n; }
void memcpy(void *dst, void *src, uint32_t n){uint8_t *a=dst,*b=src;while(n--) *a++=*b++; }
void memset(void *dst, uint8_t c, uint32_t n){uint8_t *a=dst;while(n--) *a++=c; }
int memcmp(void *s1, void *s2, uint32_t n){uint8_t *a=s1,*b=s2;while(n--){if(*a!=*b){return *a-*b;}a++;b++;} return 0; }
/* other string functions */
int atoi(unsigned char *c) { int r=0;while(*c>='0'&&*c<='9') {r*=10;r+=*c++-'0';} return r; }
int oct2bin(unsigned char *s, int n){ int r=0;while(n-->0){r<<=3;r+=*s++-'0';} return r; }
int hex2bin(unsigned char *s, int n){ int r=0;while(n-->0){r<<=4;
    if(*s>='0' && *s<='9')r+=*s-'0';else if(*s>='A'&&*s<='F')r+=*s-'A'+10;s++;} return r; }

#if DEBUG
#define DBG(s) puts(s)
#else
#define DBG(s)
#endif

/* sdcard */
#define EMMC_ARG2           ((volatile uint32_t*)(MMIO_BASE+0x00300000))
#define EMMC_BLKSIZECNT     ((volatile uint32_t*)(MMIO_BASE+0x00300004))
#define EMMC_ARG1           ((volatile uint32_t*)(MMIO_BASE+0x00300008))
#define EMMC_CMDTM          ((volatile uint32_t*)(MMIO_BASE+0x0030000C))
#define EMMC_RESP0          ((volatile uint32_t*)(MMIO_BASE+0x00300010))
#define EMMC_RESP1          ((volatile uint32_t*)(MMIO_BASE+0x00300014))
#define EMMC_RESP2          ((volatile uint32_t*)(MMIO_BASE+0x00300018))
#define EMMC_RESP3          ((volatile uint32_t*)(MMIO_BASE+0x0030001C))
#define EMMC_DATA           ((volatile uint32_t*)(MMIO_BASE+0x00300020))
#define EMMC_STATUS         ((volatile uint32_t*)(MMIO_BASE+0x00300024))
#define EMMC_CONTROL0       ((volatile uint32_t*)(MMIO_BASE+0x00300028))
#define EMMC_CONTROL1       ((volatile uint32_t*)(MMIO_BASE+0x0030002C))
#define EMMC_INTERRUPT      ((volatile uint32_t*)(MMIO_BASE+0x00300030))
#define EMMC_INT_MASK       ((volatile uint32_t*)(MMIO_BASE+0x00300034))
#define EMMC_INT_EN         ((volatile uint32_t*)(MMIO_BASE+0x00300038))
#define EMMC_CONTROL2       ((volatile uint32_t*)(MMIO_BASE+0x0030003C))
#define EMMC_SLOTISR_VER    ((volatile uint32_t*)(MMIO_BASE+0x003000FC))

// command flags
#define CMD_NEED_APP        0x80000000
#define CMD_RSPNS_48        0x00020000
#define CMD_ERRORS_MASK     0xfff9c004
#define CMD_RCA_MASK        0xffff0000

// COMMANDs
#define CMD_GO_IDLE         0x00000000
#define CMD_ALL_SEND_CID    0x02010000
#define CMD_SEND_REL_ADDR   0x03020000
#define CMD_CARD_SELECT     0x07030000
#define CMD_SEND_IF_COND    0x08020000
#define CMD_STOP_TRANS      0x0C030000
#define CMD_READ_SINGLE     0x11220010
#define CMD_READ_MULTI      0x12220032
#define CMD_SET_BLOCKCNT    0x17020000
#define CMD_APP_CMD         0x37000000
#define CMD_SET_BUS_WIDTH   (0x06020000|CMD_NEED_APP)
#define CMD_SEND_OP_COND    (0x29020000|CMD_NEED_APP)
#define CMD_SEND_SCR        (0x33220010|CMD_NEED_APP)

// STATUS register settings
#define SR_READ_AVAILABLE   0x00000800
#define SR_DAT_INHIBIT      0x00000002
#define SR_CMD_INHIBIT      0x00000001
#define SR_APP_CMD          0x00000020

// INTERRUPT register settings
#define INT_DATA_TIMEOUT    0x00100000
#define INT_CMD_TIMEOUT     0x00010000
#define INT_READ_RDY        0x00000020
#define INT_CMD_DONE        0x00000001

#define INT_ERROR_MASK      0x017E8000

// CONTROL register settings
#define C0_SPI_MODE_EN      0x00100000
#define C0_HCTL_HS_EN       0x00000004
#define C0_HCTL_DWITDH      0x00000002

#define C1_SRST_DATA        0x04000000
#define C1_SRST_CMD         0x02000000
#define C1_SRST_HC          0x01000000
#define C1_TOUNIT_DIS       0x000f0000
#define C1_TOUNIT_MAX       0x000e0000
#define C1_CLK_GENSEL       0x00000020
#define C1_CLK_EN           0x00000004
#define C1_CLK_STABLE       0x00000002
#define C1_CLK_INTLEN       0x00000001

// SLOTISR_VER values
#define HOST_SPEC_NUM       0x00ff0000
#define HOST_SPEC_NUM_SHIFT 16
#define HOST_SPEC_V3        2
#define HOST_SPEC_V2        1
#define HOST_SPEC_V1        0

// SCR flags
#define SCR_SD_BUS_WIDTH_4  0x00000400
#define SCR_SUPP_SET_BLKCNT 0x02000000
// added by my driver
#define SCR_SUPP_CCS        0x00000001

#define ACMD41_VOLTAGE      0x00ff8000
#define ACMD41_CMD_COMPLETE 0x80000000
#define ACMD41_CMD_CCS      0x40000000
#define ACMD41_ARG_HC       0x51ff8000

#define SD_OK                0
#define SD_TIMEOUT          -1
#define SD_ERROR            -2

uint32_t sd_scr[2], sd_ocr, sd_rca, sd_err, sd_hv;

/**
 * Wait for data or command ready
 */
int sd_status(uint32_t mask)
{
    int cnt = 500000; while((*EMMC_STATUS & mask) && !(*EMMC_INTERRUPT & INT_ERROR_MASK) && cnt--) delaym(1);
    return (cnt <= 0 || (*EMMC_INTERRUPT & INT_ERROR_MASK)) ? SD_ERROR : SD_OK;
}

/**
 * Wait for interrupt
 */
int sd_int(uint32_t mask)
{
    uint32_t r, m=mask | INT_ERROR_MASK;
    int cnt = 1000000; while(!(*EMMC_INTERRUPT & m) && cnt--) delaym(1);
    r=*EMMC_INTERRUPT;
    if(cnt<=0 || (r & INT_CMD_TIMEOUT) || (r & INT_DATA_TIMEOUT) ) { *EMMC_INTERRUPT=r; return SD_TIMEOUT; } else
    if(r & INT_ERROR_MASK) { *EMMC_INTERRUPT=r; return SD_ERROR; }
    *EMMC_INTERRUPT=mask;
    return 0;
}

/**
 * Send a command
 */
int sd_cmd(uint32_t code, uint32_t arg)
{
    int r=0;
    sd_err=SD_OK;
    if(code&CMD_NEED_APP) {
        r=sd_cmd(CMD_APP_CMD|(sd_rca?CMD_RSPNS_48:0),sd_rca);
        if(sd_rca && !r) { DBG("BOOTBOOT-ERROR: failed to send SD APP command\n"); sd_err=SD_ERROR;return 0;}
        code &= ~CMD_NEED_APP;
    }
    if(sd_status(SR_CMD_INHIBIT)) { DBG("BOOTBOOT-ERROR: EMMC busy\n"); sd_err= SD_TIMEOUT;return 0;}
#if SD_DEBUG
    uart_puts("EMMC: Sending command ");uart_hex(code,4);uart_puts(" arg ");uart_hex(arg,4);uart_putc('\n');
#endif
    *EMMC_INTERRUPT=*EMMC_INTERRUPT; *EMMC_ARG1=arg; *EMMC_CMDTM=code;
    if(code==CMD_SEND_OP_COND) delaym(1000); else
    if(code==CMD_SEND_IF_COND || code==CMD_APP_CMD) delaym(100);
    if((r=sd_int(INT_CMD_DONE))) {DBG("BOOTBOOT-ERROR: failed to send EMMC command\n");sd_err=r;return 0;}
    r=*EMMC_RESP0;
    if(code==CMD_GO_IDLE || code==CMD_APP_CMD) return 0; else
    if(code==(CMD_APP_CMD|CMD_RSPNS_48)) return r&SR_APP_CMD; else
    if(code==CMD_SEND_OP_COND) return r; else
    if(code==CMD_SEND_IF_COND) return r==arg? SD_OK : SD_ERROR; else
    if(code==CMD_ALL_SEND_CID) {r|=*EMMC_RESP3; r|=*EMMC_RESP2; r|=*EMMC_RESP1; return r; } else
    if(code==CMD_SEND_REL_ADDR) {
        sd_err=(((r&0x1fff))|((r&0x2000)<<6)|((r&0x4000)<<8)|((r&0x8000)<<8))&CMD_ERRORS_MASK;
        return r&CMD_RCA_MASK;
    }
    return r&CMD_ERRORS_MASK;
    // make gcc happy
    return 0;
}

/**
 * read a block from sd card and return the number of bytes read
 * returns 0 on error.
 */
int sd_readblock(uint64_t lba, uint8_t *buffer, uint32_t num)
{
    int r,c=0,d;
    if(num<1) num=1;
#if SD_DEBUG
    uart_puts("sd_readblock lba ");uart_hex(lba,4);uart_puts(" num ");uart_hex(num,4);uart_putc('\n');
#endif
    if(sd_status(SR_DAT_INHIBIT)) {sd_err=SD_TIMEOUT; return 0;}
    uint32_t *buf=(uint32_t *)buffer;
    if(sd_scr[0] & SCR_SUPP_CCS) {
        if(num > 1 && (sd_scr[0] & SCR_SUPP_SET_BLKCNT)) {
            sd_cmd(CMD_SET_BLOCKCNT,num);
            if(sd_err) return 0;
        }
        *EMMC_BLKSIZECNT = (num << 16) | 512;
        sd_cmd(num == 1 ? CMD_READ_SINGLE : CMD_READ_MULTI,lba);
        if(sd_err) return 0;
    } else {
        *EMMC_BLKSIZECNT = (1 << 16) | 512;
    }
    while( c < num ) {
        if(!(sd_scr[0] & SCR_SUPP_CCS)) {
            sd_cmd(CMD_READ_SINGLE,(lba+c)*512);
            if(sd_err) return 0;
        }
        if((r=sd_int(INT_READ_RDY))){DBG("\rBOOTBOOT-ERROR: Timeout waiting for ready to read\n");sd_err=r;return 0;}
        for(d=0;d<128;d++) buf[d] = *EMMC_DATA;
        c++; buf+=128;
    }
#if SD_DEBUG
    uart_dump(buffer,4);
#endif
    if( num > 1 && !(sd_scr[0] & SCR_SUPP_SET_BLKCNT) && (sd_scr[0] & SCR_SUPP_CCS)) sd_cmd(CMD_STOP_TRANS,0);
    return sd_err!=SD_OK || c!=num? 0 : num*512;
}

/**
 * set SD clock to frequency in Hz
 */
int sd_clk(uint32_t f)
{
    uint32_t d,c=41666666/f,x,s=32,h=0;
    int cnt = 100000;
    while((*EMMC_STATUS & (SR_CMD_INHIBIT|SR_DAT_INHIBIT)) && cnt--) delaym(1);
    if(cnt<=0) {
        DBG("BOOTBOOT-ERROR: timeout waiting for inhibit flag\n");
        return SD_ERROR;
    }

    *EMMC_CONTROL1 &= ~C1_CLK_EN; delaym(10);
    x=c-1; if(!x) s=0; else {
        if(!(x & 0xffff0000u)) { x <<= 16; s -= 16; }
        if(!(x & 0xff000000u)) { x <<= 8;  s -= 8; }
        if(!(x & 0xf0000000u)) { x <<= 4;  s -= 4; }
        if(!(x & 0xc0000000u)) { x <<= 2;  s -= 2; }
        if(!(x & 0x80000000u)) { x <<= 1;  s -= 1; }
        if(s>0) s--;
        if(s>7) s=7;
    }
    if(sd_hv>HOST_SPEC_V2) d=c; else d=(1<<s);
    if(d<=2) {d=2;s=0;}
#if SD_DEBUG
    uart_puts("sd_clk divisor ");uart_hex(d,4);uart_puts(", shift ");uart_hex(s,4);uart_putc('\n');
#endif
    if(sd_hv>HOST_SPEC_V2) h=(d&0x300)>>2;
    d=(((d&0x0ff)<<8)|h);
    *EMMC_CONTROL1=(*EMMC_CONTROL1&0xffff003f)|d; delaym(10);
    *EMMC_CONTROL1 |= C1_CLK_EN; delaym(10);
    cnt=10000; while(!(*EMMC_CONTROL1 & C1_CLK_STABLE) && cnt--) delaym(10);
    if(cnt<=0) {
        DBG("BOOTBOOT-ERROR: failed to get stable clock\n");
        return SD_ERROR;
    }
    return SD_OK;
}

/**
 * initialize EMMC to read SDHC card
 */
int sd_init()
{
    long r,cnt,ccs=0;
    // GPIO_CD
    r=*GPFSEL4; r&=~(7<<(7*3)); *GPFSEL4=r;
    *GPPUD=2; delay(150); *GPPUDCLK1=(1<<15); delay(150); *GPPUD=0; *GPPUDCLK1=0;
    r=*GPHEN1; r|=1<<15; *GPHEN1=r;

    // GPIO_CLK, GPIO_CMD
    r=*GPFSEL4; r|=(7<<(8*3))|(7<<(9*3)); *GPFSEL4=r;
    *GPPUD=2; delay(150); *GPPUDCLK1=(1<<16)|(1<<17); delay(150); *GPPUD=0; *GPPUDCLK1=0;

    // GPIO_DAT0, GPIO_DAT1, GPIO_DAT2, GPIO_DAT3
    r=*GPFSEL5; r|=(7<<(0*3)) | (7<<(1*3)) | (7<<(2*3)) | (7<<(3*3)); *GPFSEL5=r;
    *GPPUD=2; delay(150);
    *GPPUDCLK1=(1<<18) | (1<<19) | (1<<20) | (1<<21);
    delay(150); *GPPUD=0; *GPPUDCLK1=0;

    sd_hv = (*EMMC_SLOTISR_VER & HOST_SPEC_NUM) >> HOST_SPEC_NUM_SHIFT;
#if SD_DEBUG
    uart_puts("EMMC: GPIO set up\n");
#endif
    // Reset the card.
    *EMMC_CONTROL0 = 0; *EMMC_CONTROL1 |= C1_SRST_HC;
    cnt=10000; do{delaym(10);} while( (*EMMC_CONTROL1 & C1_SRST_HC) && cnt-- );
    if(cnt<=0) {
        DBG("BOOTBOOT-ERROR: failed to reset EMMC\n");
        return SD_ERROR;
    }
#if SD_DEBUG
    uart_puts("EMMC: reset OK\n");
#endif
    *EMMC_CONTROL1 |= C1_CLK_INTLEN | C1_TOUNIT_MAX;
    delaym(10);
    // Set clock to setup frequency.
    if((r=sd_clk(400000))) return r;
    *EMMC_INT_EN   = 0xffffffff;
    *EMMC_INT_MASK = 0xffffffff;
    sd_scr[0]=sd_scr[1]=sd_rca=sd_err=0;
    sd_cmd(CMD_GO_IDLE,0);
    if(sd_err) return sd_err;

    sd_cmd(CMD_SEND_IF_COND,0x000001AA);
    if(sd_err) return sd_err;
    cnt=6; r=0; while(!(r&ACMD41_CMD_COMPLETE) && cnt--) {
        delay(400);
        r=sd_cmd(CMD_SEND_OP_COND,ACMD41_ARG_HC);
#if SD_DEBUG
    uart_puts("EMMC: CMD_SEND_OP_COND returned ");
    if(r&ACMD41_CMD_COMPLETE)
        uart_puts("COMPLETE ");
    if(r&ACMD41_VOLTAGE)
        uart_puts("VOLTAGE ");
    if(r&ACMD41_CMD_CCS)
        uart_puts("CCS ");
    uart_hex(r,8);
    uart_putc('\n');
#endif
        if(sd_err!=SD_TIMEOUT && sd_err!=SD_OK ) {
            DBG("BOOTBOOT-ERROR: EMMC ACMD41 returned error\n");
            return sd_err;
        }
    }
    if(!(r&ACMD41_CMD_COMPLETE) || !cnt ) return SD_TIMEOUT;
    if(!(r&ACMD41_VOLTAGE)) return SD_ERROR;
    if(r&ACMD41_CMD_CCS) ccs=SCR_SUPP_CCS;

    sd_cmd(CMD_ALL_SEND_CID,0);

    sd_rca = sd_cmd(CMD_SEND_REL_ADDR,0);
#if SD_DEBUG
    uart_puts("EMMC: CMD_SEND_REL_ADDR returned ");
    uart_hex(sd_rca,8);
    uart_putc('\n');
#endif
    if(sd_err) return sd_err;

    if((r=sd_clk(25000000))) return r;

    sd_cmd(CMD_CARD_SELECT,sd_rca);
    if(sd_err) return sd_err;

    if(sd_status(SR_DAT_INHIBIT)) return SD_TIMEOUT;
    *EMMC_BLKSIZECNT = (1<<16) | 8;
    sd_cmd(CMD_SEND_SCR,0);
    if(sd_err) return sd_err;
    if(sd_int(INT_READ_RDY)) return SD_TIMEOUT;

    r=0; cnt=100000; while(r<2 && cnt) {
        if( *EMMC_STATUS & SR_READ_AVAILABLE )
            sd_scr[r++] = *EMMC_DATA;
        else
            delaym(1);
    }
    if(r!=2) return SD_TIMEOUT;
    if(sd_scr[0] & SCR_SD_BUS_WIDTH_4) {
        sd_cmd(CMD_SET_BUS_WIDTH,sd_rca|2);
        if(sd_err) return sd_err;
        *EMMC_CONTROL0 |= C0_HCTL_DWITDH;
    }
    // add software flag
#ifdef SD_DEBUG
    uart_puts("EMMC: supports ");
    if(sd_scr[0] & SCR_SUPP_SET_BLKCNT)
        uart_puts("SET_BLKCNT ");
    if(ccs)
        uart_puts("CCS ");
    uart_putc('\n');
#endif
    sd_scr[0]&=~SCR_SUPP_CCS;
    sd_scr[0]|=ccs;
    return SD_OK;
}

// comment out this include if you don't want FS/Z support
#include "../../osZ/etc/include/sys/fsZ.h"
// get filesystem drivers for initrd
#include "fs.h"

/*** other defines and structs ***/
#define COREMMIO_BASE 0xFFFFFFFFF8000000

typedef struct {
    uint32_t type[4];
    uint8_t  uuid[16];
    uint64_t start;
    uint64_t end;
    uint64_t flags;
    uint8_t  name[72];
} efipart_t;

typedef struct {
    char        jmp[3];
    char        oem[8];
    uint16_t    bps;
    uint8_t     spc;
    uint16_t    rsc;
    uint8_t     nf;
    uint16_t    nr;
    uint16_t    ts16;
    uint8_t     media;
    uint16_t    spf16;
    uint16_t    spt;
    uint16_t    nh;
    uint32_t    hs;
    uint32_t    ts32;
    uint32_t    spf32;
    uint32_t    flg;
    uint32_t    rc;
    char        vol[6];
    char        fst[8];
    char        dmy[20];
    char        fst2[8];
} __attribute__((packed)) bpb_t;

typedef struct {
    char        name[8];
    char        ext[3];
    char        attr[9];
    uint16_t    ch;
    uint32_t    attr2;
    uint16_t    cl;
    uint32_t    size;
} __attribute__((packed)) fatdir_t;

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t headersize;/* offset of bitmaps in file */
    uint16_t flags;     /* original PSF2 has 32 bit flags */
    uint8_t hotspot_x;  /* addition to OS/Z */
    uint8_t hotspot_y;
    uint32_t numglyph;
    uint32_t bytesperglyph;
    uint32_t height;
    uint32_t width;
    uint8_t glyphs;
} __attribute__((packed)) font_t;

extern volatile unsigned char _binary_font_psf_start;

/*** common variables ***/
file_t env;         // environment file descriptor
file_t initrd;      // initrd file descriptor
file_t core;        // kernel file descriptor
BOOTBOOT *bootboot; // the BOOTBOOT structure

// default environment variables. M$ states that 1024x768 must be supported
int reqwidth = 1024, reqheight = 768;
char *kernelname="sys/core";
unsigned char *kne;

// alternative environment name
char *cfgname="sys/config";

/* current cursor position */
int kx, ky;
/* maximum coordinates */
int maxx, maxy;

/**
 * Get a linear frame buffer
 */
int GetLFB(uint32_t width, uint32_t height)
{
    font_t *font = (font_t*)&_binary_font_psf_start;

    //query natural width, height if not given
    if(width==0 && height==0) {
        mbox[0] = 8*4;
        mbox[1] = MBOX_REQUEST;
        mbox[2] = 0x40003;  //get phy wh
        mbox[3] = 8;
        mbox[4] = 8;
        mbox[5] = 0;
        mbox[6] = 0;
        mbox[7] = 0;
        if(mbox_call(MBOX_CH_PROP,mbox) && mbox[5]!=0) {
            width=mbox[5];
            height=mbox[6];
        }
    }
    //if we already have a framebuffer, release it
    if(bootboot->fb_ptr!=NULL) {
        mbox[0] = 8*4;
        mbox[1] = MBOX_REQUEST;
        mbox[2] = 0x48001;  //release buffer
        mbox[3] = 8;
        mbox[4] = 8;
        mbox[5] = (uint32_t)(((uint64_t)bootboot->fb_ptr));
        mbox[6] = 0;
        mbox[7] = 0;
        mbox_call(MBOX_CH_PROP,mbox);
    }
    //check minimum resolution
    if(width<640) width=640;
    if(height<480) height=480;

    mbox[0] = 35*4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003;  //set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = width;        //FrameBufferInfo.width
    mbox[6] = height;       //FrameBufferInfo.height

    mbox[7] = 0x48004;  //set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = width;       //FrameBufferInfo.virtual_width
    mbox[11] = height;      //FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; //set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0;           //FrameBufferInfo.x_offset
    mbox[16] = 0;           //FrameBufferInfo.y.offset

    mbox[17] = 0x48005; //set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32;          //FrameBufferInfo.depth

    mbox[21] = 0x48006; //set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 0;       //RGB, not BGR preferably

    mbox[25] = 0x40001; //get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = PAGESIZE;    //FrameBufferInfo.pointer
    mbox[29] = 0;           //FrameBufferInfo.size

    mbox[30] = 0x40008; //get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0;           //FrameBufferInfo.pitch

    mbox[34] = 0;       //Arnold Schwarzenegger

    if(mbox_call(MBOX_CH_PROP,mbox) && mbox[20]==32 && mbox[27]==(MBOX_RESPONSE|8) && mbox[28]!=0) {
        mbox[28]&=0x3FFFFFFF;
        bootboot->fb_width=mbox[5];
        bootboot->fb_height=mbox[6];
        bootboot->fb_scanline=mbox[33];
        bootboot->fb_ptr=(void*)((uint64_t)mbox[28]);
        bootboot->fb_size=mbox[29];
        bootboot->fb_type=mbox[24]?FB_ABGR:FB_ARGB;
        kx=ky=0;
        maxx=bootboot->fb_width/(font->width+1);
        maxy=bootboot->fb_height/font->height;
        return 1;
    }
    return 0;
}

/**
 * display one literal unicode character
 */
void putc(char c)
{
    font_t *font = (font_t*)&_binary_font_psf_start;
    unsigned char *glyph = (unsigned char*)&_binary_font_psf_start +
     font->headersize + (c>0&&c<font->numglyph?c:0)*font->bytesperglyph;
    int offs = (ky * font->height * bootboot->fb_scanline) + (kx * (font->width+1) * 4);
    int x,y, line,mask;
    int bytesperline=(font->width+7)/8;
    if(c=='\r') {
        kx=0;
    } else
    if(c=='\n') {
        kx=0; ky++;
    } else {
        for(y=0;y<font->height;y++){
            line=offs;
            mask=1<<(font->width-1);
            for(x=0;x<font->width;x++){
                *((uint32_t*)((uint64_t)bootboot->fb_ptr + line))=((int)*glyph) & (mask)?color:0;
                mask>>=1;
                line+=4;
            }
            *((uint32_t*)((uint64_t)bootboot->fb_ptr + line))=0;
            glyph+=bytesperline;
            offs+=bootboot->fb_scanline;
        }
        kx++;
        if(kx>=maxx) {
            kx=0; ky++;
        }
    }
    // send it to serial too
    uart_putc(c);
}

/**
 * display a string
 */
void puts(char *s) { while(*s) putc(*s++); }

void ParseEnvironment(uint8_t *env)
{
    uint8_t *end=env+PAGESIZE;
    DBG(" * Environment\n");
    env--; env[PAGESIZE]=0; kne=NULL;
    while(env<end) {
        env++;
        // failsafe
        if(env[0]==0)
            break;
        // skip white spaces
        if(env[0]==' '||env[0]=='\t'||env[0]=='\r'||env[0]=='\n')
            continue;
        // skip comments
        if((env[0]=='/'&&env[1]=='/')||env[0]=='#') {
            while(env<end && env[0]!='\r' && env[0]!='\n' && env[0]!=0){
                env++;
            }
            env--;
            continue;
        }
        if(env[0]=='/'&&env[1]=='*') {
            env+=2;
            while(env[0]!=0 && env[-1]!='*' && env[0]!='/')
                env++;
        }
        // parse screen dimensions
        if(!memcmp(env,"screen=",7)){
            env+=7;
            reqwidth=atoi(env);
            while(env<end && *env!=0 && *(env-1)!='x') env++;
            reqheight=atoi(env);
        }
        // get kernel's filename
        if(!memcmp(env,"kernel=",7)){
            env+=7;
            kernelname=(char*)env;
            while(env<end && env[0]!='\r' && env[0]!='\n' &&
                env[0]!=' ' && env[0]!='\t' && env[0]!=0)
                    env++;
            kne=env;
            *env=0;
            env++;
        }
    }
}

/**
 * bootboot entry point
 */
int bootboot_main(uint64_t hcl)
{
    uint8_t *pe,bkp=0;
    uint32_t np,sp,r,pa,mp;
    efipart_t *part;
    volatile bpb_t *bpb;
    uint64_t entrypoint=0, bss=0, *paging, reg;
    MMapEnt *mmap;

    /* initialize UART */
    *UART0_CR = 0;         // turn off UART0
    *AUX_ENABLE = 0;       // turn off UART1

    /* set up clock for consistent divisor values */
    mbox[0] = 8*4;
    mbox[1] = MBOX_REQUEST;
    mbox[2] = 0x38002;     // set clock rate
    mbox[3] = 12;
    mbox[4] = 8;
    mbox[5] = 2;           // UART clock
    mbox[6] = 4000000;     // 4Mhz
    mbox[7] = 0;           // set turbo
    mbox_call(MBOX_CH_PROP,mbox);

#if CONSOLE == UART1
    *AUX_ENABLE |=1;       // enable UART1, AUX mini uart
    *AUX_MU_IER = 0;
    *AUX_MU_CNTL = 0;
    *AUX_MU_LCR = 3;       // 8 bits
    *AUX_MU_MCR = 0;
    *AUX_MU_IER = 0;
    *AUX_MU_IIR = 0xc6;    // disable interrupts
    *AUX_MU_BAUD = 270;    // 115200 baud
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(2<<12)|(2<<15);    // alt5
    *GPFSEL1 = r;
#else
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); // gpio14, gpio15
    r|=(4<<12)|(4<<15);    // alt0
    *GPFSEL1 = r;
#endif
    *GPPUD = 0;            // enable pins 14 and 15
    delay(150);
    *GPPUDCLK0 = (1<<14)|(1<<15);
    delay(150);
    *GPPUDCLK0 = 0;        // flush GPIO setup
#if CONSOLE == UART1
    *AUX_MU_CNTL = 3;      // enable Tx, Rx
#else
    *UART0_ICR = 0x7FF;    // clear interrupts
    *UART0_IBRD = 2;       // 115200 baud
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0b11<<5; // 8n1
//    *UART0_IMSC = 0x7F2;   // mask interrupts
    *UART0_CR = 0x301;     // enable Tx, Rx, FIFO
#endif

    /* create bootboot structure */
    bootboot = (BOOTBOOT*)&__bootboot;
    memcpy((void*)&bootboot->magic,BOOTBOOT_MAGIC,4);
    bootboot->protocol = PROTOCOL_STATIC;
    bootboot->loader_type = LOADER_RPI;
    bootboot->size = 128;
    bootboot->pagesize = PAGESIZE==4096? 12 : (PAGESIZE==65536 ? 16 : 14);
    bootboot->aarch64.mmio_ptr = COREMMIO_BASE;
    // set up a framebuffer so that we can write on screen
    if(!GetLFB(0, 0)) goto viderr;
    puts("Booting OS...\n");

    /* check for 4k granule and at least 36 bits address */
    asm volatile ("mrs %0, id_aa64mmfr0_el1" : "=r" (reg));
    pa=reg&0xF;
    if(reg&(0xF<<28) || pa<1) {
        puts("BOOTBOOT-PANIC: Hardware not supported\n");
        uart_puts("ID_AA64MMFR0_EL1 ");
        uart_hex(reg,8);
        uart_putc('\n');
        goto error;
    }
    /* initialize microsec delay */
    asm volatile ("mrs %0, cntfrq_el0" : "=r" (cntfrq));

    /* Raspbootin compatibility, see https://github.com/mrvn/raspbootin
     * We can receive INITRD from raspbootcom */
    uart_puts("\x03\x03\x03");
    // wait reply with timeout
    mp=10000;
#if CONSOLE == UART1
    r=(char)(*AUX_MU_IO);do{asm volatile("nop");}while(--mp>0 && !(*AUX_MU_LSR&0x01));
#else
    r=(char)(*UART0_DR);do{asm volatile("nop");}while(--mp>0 && *UART0_FR&0x10);
#endif
    if(mp>0) {
        // we got response from raspbootcom
        sp=uart_getc();
        sp|=uart_getc()<<8; sp|=uart_getc()<<16; sp|=uart_getc()<<24;
        if(sp>0 && sp<INITRD_MAXSIZE*1024*1024) {
            uart_puts("OK");
            initrd.size=sp;
            initrd.ptr=pe=(uint8_t*)&_end;
            while(sp--) *pe++ = uart_getc();
            goto gotinitrd;
        }
    }

    /* initialize SDHC card reader in EMMC */
    if(sd_init()) {
        puts("BOOTBOOT-PANIC: Unable to initialize SDHC card\n");
        goto error;
    }

    /* read and parse GPT table */
    r=sd_readblock(1,(unsigned char*)&__diskbuf,1);
    if(r==0 || memcmp((void*)&__diskbuf, "EFI PART", 8)) {
diskerr:
        puts("BOOTBOOT-PANIC: No boot partition\n");
        goto error;
    }
    // get number of partitions and size of partition entry
    np=*((uint32_t*)((char*)&__diskbuf+80)); sp=*((uint32_t*)((char*)&__diskbuf+84));
    if(np>127) np=127;
    // read GPT entries
    r=sd_readblock(*((uint32_t*)((char*)&__diskbuf+72)),(unsigned char*)&__diskbuf,(np*sp+511)/512);
    if(r==0) goto diskerr;
    part=NULL;
    // first, look for a partition with bootable flag
    for(r=0;r<np;r++) {
        part = (efipart_t*)((char*)&__diskbuf+r*sp);
        if((part->type[0]==0 && part->type[1]==0 && part->type[2]==0 && part->type[3]==0) || part->start==0) {
            r=np;
            break;
        }
        // EFI_PART_USED_BY_OS?
        if(part->flags&4) break;
    }
    // if none, look for specific partition types
    if(part==NULL || r>=np) {
        for(r=0;r<np;r++) {
            part = (efipart_t*)((char*)&__diskbuf+r*sp);
            if((part->type[0]==0 && part->type[1]==0 && part->type[2]==0 && part->type[3]==0) || part->start==0) {
                r=np;
                break;
            }
                // ESP?
            if((part->type[0]==0xC12A7328 && part->type[1]==0x11D2F81F) ||
                // or OS/Z root partition for this architecture?
                (part->type[0]==0x5A2F534F && (part->type[1]&0xFFFF)==0xAA64 && part->type[3]==0x746F6F72))
                break;
        }
    }
    if(part==NULL || r>=np) goto diskerr;
    r=sd_readblock(part->start,(unsigned char*)&_end,1);
    if(r==0) goto diskerr;
    initrd.ptr=NULL; initrd.size=0;
    // wait keypress with timeout
    mp=50;
#if CONSOLE == UART1
    r=(char)(*AUX_MU_IO);do{delaym(5);}while(--mp>0 && !(*AUX_MU_LSR&0x01));
#else
    r=(char)(*UART0_DR);do{delaym(5);}while(--mp>0 && *UART0_FR&0x10);
#endif
    if(mp!=0) {
        puts(" * Backup initrd\n");
        bkp=1;
    }
    //is it a FAT partition?
    bpb=(bpb_t*)&_end;
    if(!memcmp((void*)bpb->fst,"FAT16",5) || !memcmp((void*)bpb->fst2,"FAT32",5)) {
        // locate BOOTBOOT directory
        uint32_t data_sec, root_sec, clu=0, s, s2;
        fatdir_t *dir;
        uint32_t *fat32=(uint32_t*)((uint8_t*)&_end+bpb->rsc*512);
        uint16_t *fat16=(uint16_t*)fat32;
        uint8_t *ptr;
        data_sec=root_sec=((bpb->spf16?bpb->spf16:bpb->spf32)*bpb->nf)+bpb->rsc;
        //WARNING gcc generates a code for bpb->nr that cause unaligned exception
        s=*((uint32_t*)&bpb->nf);
        s>>=8;
        s&=0xFFFF;
        s<<=5;
        if(bpb->spf16>0) {
            data_sec+=(s+511)>>9;
        } else {
            root_sec+=(bpb->rc-2)*bpb->spc;
        }
        // load fat table
        r=sd_readblock(part->start+1,(unsigned char*)&_end+512,(bpb->spf16?bpb->spf16:bpb->spf32)+bpb->rsc);
        if(r==0) goto diskerr;
        pe=(uint8_t*)&_end+512+r;
        // load root directory
        r=sd_readblock(part->start+root_sec,(unsigned char*)pe,s/512+1);
        dir=(fatdir_t*)pe;
        while(dir->name[0]!=0 && memcmp(dir->name,"BOOTBOOT   ",11)) dir++;
        if(dir->name[0]!='B') goto diskerr;
        r=sd_readblock(part->start+(dir->cl+(dir->ch<<16)-2)*bpb->spc+data_sec,(unsigned char*)pe,bpb->spc);
        if(r==0) goto diskerr;
        dir=(fatdir_t*)pe;
        // locate environment and initrd
        while(dir->name[0]!=0) {
            if(!memcmp(dir->name,"CONFIG     ",11)) {
                s=dir->size<PAGESIZE?dir->size:PAGESIZE-1;
                clu=dir->cl+(dir->ch<<16);
                ptr=(void*)&__environment;
                while(s>0) {
                    s2=s>bpb->spc*512?bpb->spc*512:s;
                    r=sd_readblock(part->start+(clu-2)*bpb->spc+data_sec,ptr,s2<512?1:s2/512);
                    clu=bpb->spf16>0?fat16[clu]:fat32[clu];
                    ptr+=s2;
                    s-=s2;
                }
                clu=0;
            } else
            if(!memcmp(dir->name,bkp?"INITRD  BAK":"INITRD     ",11)) {
                clu=dir->cl+(dir->ch<<16);
                initrd.size=dir->size;
            }
            dir++;
        }
        // if initrd not found, try architecture specific name
        if(clu==0) {
            dir=(fatdir_t*)pe;
            while(dir->name[0]!=0) {
                if(!memcmp(dir->name,"AARCH64    ",11)) {
                    clu=dir->cl+(dir->ch<<16);
                    initrd.size=dir->size;
                    break;
                }
                dir++;
            }
        }
        // walk through cluster chain to load initrd
        if(clu!=0 && initrd.size!=0) {
            initrd.ptr=ptr=pe;
            s=initrd.size;
            while(s>0) {
                s2=s>bpb->spc*512?bpb->spc*512:s;
                r=sd_readblock(part->start+(clu-2)*bpb->spc+data_sec,ptr,s2<512?1:s2/512);
                clu=bpb->spf16>0?fat16[clu]:fat32[clu];
                ptr+=s2;
                s-=s2;
            }
        }
    } else {
        // initrd is on the entire partition
        r=sd_readblock(part->start,(unsigned char*)&_end+512,part->end-part->start);
        if(r==0) goto diskerr;
        initrd.ptr=(uint8_t*)&_end;
        initrd.size=r;
    }
gotinitrd:
    if(initrd.ptr==NULL || initrd.size==0) {
        puts("BOOTBOOT-PANIC: INITRD not found\n");
        goto error;
    }
#if INITRD_DEBUG
    uart_puts("Initrd at ");uart_hex((uint64_t)initrd.ptr,4);uart_putc(' ');uart_hex(initrd.size,4);uart_putc('\n');
#endif
    // uncompress if it's compressed
    if(initrd.ptr[0]==0x1F && initrd.ptr[1]==0x8B) {
        unsigned char *addr,f;
        volatile TINF_DATA d;
        DBG(" * Gzip compressed initrd\n");
        // skip gzip header
        addr=initrd.ptr+2;
        if(*addr++!=8) goto gzerr;
        f=*addr++; addr+=6;
        if(f&4) { r=*addr++; r+=(*addr++ << 8); addr+=r; }
        if(f&8) { while(*addr++ != 0); }
        if(f&16) { while(*addr++ != 0); }
        if(f&2) addr+=2;
        d.source = addr;
        memcpy((void*)&d.destSize,initrd.ptr+initrd.size-4,4);
        // decompress
        d.bitcount = 0;
        d.bfinal = 0;
        d.btype = -1;
        d.curlen = 0;
        if((uint8_t*)&_end+d.destSize<addr)
            d.dest=(uint8_t*)&_end;
        else
            d.dest=(uint8_t*)((uint64_t)(initrd.ptr+initrd.size+PAGESIZE-1)&~(PAGESIZE-1));
        initrd.ptr=(uint8_t*)d.dest;
        initrd.size=d.destSize;
#if INITRD_DEBUG
        uart_puts("Inflating to ");uart_hex((uint64_t)d.dest,4);uart_putc(' ');uart_hex(d.destSize,4);uart_putc('\n');
#endif
        puts(" * Inflating image...\r");
        do { r = uzlib_uncompress(&d); } while (!r);
        puts("                     \r");
        if (r != TINF_DONE) {
gzerr:      puts("BOOTBOOT-PANIC: Unable to uncompress\n");
            goto error;
        }
    }
    // copy the initrd to it's final position, making it properly aligned
    if((uint64_t)initrd.ptr!=(uint64_t)&_end) {
        memcpy((void*)&_end, initrd.ptr, initrd.size);
    }
    bootboot->initrd_ptr=(uint64_t)&_end;
    // round up to page size
    bootboot->initrd_size=(initrd.size+PAGESIZE-1)&~(PAGESIZE-1);
    DBG(" * Initrd loaded\n");
#if INITRD_DEBUG
    // dump initrd in memory
    uart_dump((void*)bootboot->initrd_ptr,8);
#endif

    // if no config, locate it in uncompressed initrd
    if(*((uint8_t*)&__environment)==0) {
        r=0; env.ptr=NULL;
        while(env.ptr==NULL && fsdrivers[r]!=NULL) {
            env=(*fsdrivers[r++])((unsigned char*)bootboot->initrd_ptr,cfgname);
        }
        if(env.ptr!=NULL)
            memcpy((void*)&__environment,(void*)(env.ptr),env.size<PAGESIZE?env.size:PAGESIZE-1);
    }

    // parse config
    ParseEnvironment((unsigned char*)&__environment);

    // locate sys/core
    entrypoint=0;
    r=0; core.ptr=NULL;
    while(core.ptr==NULL && fsdrivers[r]!=NULL) {
        core=(*fsdrivers[r++])((unsigned char*)bootboot->initrd_ptr,kernelname);
    }
    if(kne!=NULL)
        *kne='\n';
    // scan for the first executable
    if(core.ptr==NULL || core.size==0) {
        DBG(" * Autodetecting kernel\n");
        core.size=0;
        r=bootboot->initrd_size;
        core.ptr=(uint8_t*)bootboot->initrd_ptr;
        while(r-->0) {
            Elf64_Ehdr *ehdr=(Elf64_Ehdr *)(core.ptr);
            pe_hdr *pehdr=(pe_hdr*)(core.ptr + ((mz_hdr*)(core.ptr))->peaddr);
            if((!memcmp(ehdr->e_ident,ELFMAG,SELFMAG)||!memcmp(ehdr->e_ident,"OS/Z",4))&&
                ehdr->e_ident[EI_CLASS]==ELFCLASS64&&
                ehdr->e_ident[EI_DATA]==ELFDATA2LSB&&
                ehdr->e_machine==EM_AARCH64&&
                ehdr->e_phnum>0){
                    core.size=1;
                    break;
                }
            if(((mz_hdr*)(core.ptr))->magic==MZ_MAGIC && pehdr->magic == PE_MAGIC &&
                pehdr->machine == IMAGE_FILE_MACHINE_ARM64 && pehdr->file_type == PE_OPT_MAGIC_PE32PLUS) {
                    core.size=1;
                    break;
                }
            core.ptr++;
        }
    }
    if(core.ptr==NULL || core.size==0) {
        puts("BOOTBOOT-PANIC: Kernel not found in initrd\n");
        goto error;
    } else {
        Elf64_Ehdr *ehdr=(Elf64_Ehdr *)(core.ptr);
        pe_hdr *pehdr=(pe_hdr*)(core.ptr + ((mz_hdr*)(core.ptr))->peaddr);
        if((!memcmp(ehdr->e_ident,ELFMAG,SELFMAG)||!memcmp(ehdr->e_ident,"OS/Z",4))&&
            ehdr->e_ident[EI_CLASS]==ELFCLASS64&&
            ehdr->e_ident[EI_DATA]==ELFDATA2LSB&&
            ehdr->e_machine==EM_AARCH64&&
            ehdr->e_phnum>0){
                DBG(" * Parsing ELF64\n");
                Elf64_Phdr *phdr=(Elf64_Phdr *)((uint8_t *)ehdr+ehdr->e_phoff);
                for(r=0;r<ehdr->e_phnum;r++){
                    if(phdr->p_type==PT_LOAD && phdr->p_vaddr>>48==0xffff) {
                        core.ptr += phdr->p_offset;
                        // hack to keep symtab and strtab for shared libraries
                        core.size = phdr->p_filesz + (ehdr->e_type==3?0x4000:0);
                        bss = phdr->p_memsz - core.size;
                        entrypoint = ehdr->e_entry;
                        break;
                    }
                    phdr=(Elf64_Phdr *)((uint8_t *)phdr+ehdr->e_phentsize);
                }
        } else
        if(((mz_hdr*)(core.ptr))->magic==MZ_MAGIC && pehdr->magic == PE_MAGIC &&
            pehdr->machine == IMAGE_FILE_MACHINE_ARM64 && pehdr->file_type == PE_OPT_MAGIC_PE32PLUS &&
            (int64_t)pehdr->code_base>>48==0xffff) {
                DBG(" * Parsing PE32+\n");
                core.size = (pehdr->entry_point-pehdr->code_base) + pehdr->text_size + pehdr->data_size;
                bss = pehdr->bss_size;
                entrypoint = (int64_t)pehdr->entry_point;
        }
    }
#if EXEC_DEBUG
    uart_puts("Executable size ");
    uart_hex((uint64_t)core.size,4);
    uart_puts(" bss ");
    uart_hex((uint64_t)bss,4);
    uart_putc('\n');
    uart_dump((void*)core.ptr,4);
#endif
    if(core.size<2 || entrypoint==0) {
        puts("BOOTBOOT-PANIC: Kernel is not a valid executable\n");
        goto error;
    }
    // create core segment
    memcpy((void*)(bootboot->initrd_ptr+bootboot->initrd_size), core.ptr, core.size);
    core.ptr=(uint8_t*)(bootboot->initrd_ptr+bootboot->initrd_size);
    if(bss>0)
        memset(core.ptr + core.size, 0, bss);
    core.size = (core.size+bss+PAGESIZE-1)&~(PAGESIZE-1);
#if EXEC_DEBUG
    uart_puts("Core ");
    uart_hex((uint64_t)core.ptr,4);
    uart_puts(" to ");
    uart_hex((uint64_t)core.ptr+core.size,4);
    uart_putc('\n');
#endif

    /* generate memory map to bootboot struct */
    DBG(" * Memory Map\n");
    mmap=(MMapEnt *)&bootboot->mmap;

    // everything before the bootboot struct is free
    mmap->ptr=0; mmap->size=(uint64_t)&__bootboot | MMAP_FREE;
    mmap++; bootboot->size+=sizeof(MMapEnt);

    // mark bss reserved
    mmap->ptr=(uint64_t)&__bootboot; mmap->size=((uint64_t)&_end-(uint64_t)&__bootboot) | MMAP_USED;
    mmap++; bootboot->size+=sizeof(MMapEnt);

    r=bootboot->initrd_size + core.size;
    // after bss and before initrd is free
    if(bootboot->initrd_ptr-(uint64_t)&_end) {
        mmap->ptr=(uint64_t)&_end; mmap->size=(bootboot->initrd_ptr-(uint64_t)&_end) | MMAP_FREE;
        mmap++; bootboot->size+=sizeof(MMapEnt);
        // initrd is reserved (and add core's area to it)
        mmap->ptr=bootboot->initrd_ptr; mmap->size=r | MMAP_USED;
        mmap++; bootboot->size+=sizeof(MMapEnt);
    } else {
        mmap--; mmap->size+=r; mmap++;
    }
    r+=(uint32_t)bootboot->initrd_ptr;

    mbox[0]=8*4;
    mbox[1]=0;
    mbox[2]=0x10005; // get memory size
    mbox[3]=8;
    mbox[4]=0;
    mbox[5]=0;
    mbox[6]=0;
    mbox[7]=0;
    if(!mbox_call(MBOX_CH_PROP, mbox))
        // on failure (should never happen) assume 64Mb memory max
        mbox[6]=64*1024*1024;

    // everything after initrd to the top of memory is free
    mp=mbox[6]-r;
    mmap->ptr=r; mmap->size=mp | MMAP_FREE;
    mmap++; bootboot->size+=sizeof(MMapEnt);

    // MMIO area
    mmap->ptr=MMIO_BASE; mmap->size=((uint64_t)0x40000000-MMIO_BASE) | MMAP_MMIO;
    mmap++; bootboot->size+=sizeof(MMapEnt);

#if MEM_DEBUG
    /* dump memory map */
    mmap=(MMapEnt *)&bootboot->mmap;
    for(r=128;r<bootboot->size;r+=sizeof(MMapEnt)) {
        uart_hex(MMapEnt_Ptr(mmap),8);
        uart_putc(' ');
        uart_hex(MMapEnt_Ptr(mmap)+MMapEnt_Size(mmap)-1,8);
        uart_putc(' ');
        uart_hex(MMapEnt_Type(mmap),1);
        uart_putc(' ');
        switch(MMapEnt_Type(mmap)) {
            case MMAP_USED: uart_puts("reserved"); break;
            case MMAP_FREE: uart_puts("free"); break;
            case MMAP_ACPIFREE: uart_puts("acpifree"); break;
            case MMAP_ACPINVS: uart_puts("acpinvs"); break;
            case MMAP_MMIO: uart_puts("mmio"); break;
            default: uart_puts("unknown"); break;
        }
        uart_putc('\n');
        mmap++;
    }
#endif

    /* get linear framebuffer if requested resolution different than current */
    DBG(" * Screen VideoCore\n");
    if(reqwidth!=bootboot->fb_width || reqheight!=bootboot->fb_height) {
        if(!GetLFB(reqwidth, reqheight)) {
viderr:
            puts("BOOTBOOT-PANIC: VideoCore error, no framebuffer\n");
            goto error;
        }
    }
    kx=ky=0; color=0xFFDD33;

    /* create MMU translation tables in __paging */
    paging=(uint64_t*)&__paging;
    // TTBR0, identity L1
    paging[0]=(uint64_t)((uint8_t*)&__paging+2*PAGESIZE)|0b11|(3<<8)|(1<<10); //AF=1,Block=1,Present=1, SH=3 ISH, RO
    // identity L2
    paging[2*512]=(uint64_t)((uint8_t*)&__paging+3*PAGESIZE)|0b11|(3<<8)|(1<<10); //AF=1,Block=1,Present=1
    // identity L2 2M blocks
    mp>>=21;
    np=MMIO_BASE>>21;
    for(r=1;r<512;r++)
        paging[2*512+r]=(uint64_t)(((uint64_t)r<<21))|0b01|(1<<10)|(r>=np?(2<<8)|(1<<2)|(1L<<54):(3<<8)); //device SH=2 OSH
    // identity L3
    for(r=0;r<512;r++)
        paging[3*512+r]=(uint64_t)(r*PAGESIZE)|0b11|(1<<10)|(r<0x80||r>(uint32_t)((uint64_t)&_data/PAGESIZE)?0:(1<<7));
    // TTBR1, core L1
    paging[512+511]=(uint64_t)((uint8_t*)&__paging+4*PAGESIZE)|0b11|(3<<8)|(1<<10); //AF=1,Block=1,Present=1
    // core L2
    // map MMIO in kernel space
    for(r=0;r<32;r++)
        paging[4*512+448+r]=(uint64_t)(MMIO_BASE+((uint64_t)r<<21))|0b01|(2<<8)|(1<<10)|(1<<2)|(1L<<54); //OSH, Attr=1, NX
    // map framebuffer
    for(r=0;r<28;r++)
        paging[4*512+480+r]=(uint64_t)((uint8_t*)&__paging+(6+r)*PAGESIZE)|0b11|(2<<8)|(1<<10)|(2<<2)|(1L<<54); //OSH, Attr=2
    paging[4*512+511]=(uint64_t)((uint8_t*)&__paging+5*PAGESIZE)|0b11|(3<<8)|(1<<10);// pointer to core L3
    // core L3
    paging[5*512+0]=(uint64_t)((uint8_t*)&__bootboot)|0b11|(3<<8)|(1<<10)|(1L<<54);  // p, b, AF, ISH
    paging[5*512+1]=(uint64_t)((uint8_t*)&__environment)|0b11|(3<<8)|(1<<10)|(1L<<54);
    for(r=0;r<(core.size/PAGESIZE);r++)
        paging[5*512+2+r]=(uint64_t)((uint8_t *)core.ptr+(uint64_t)r*PAGESIZE)|0b11|(3<<8)|(1<<10);
#if MEM_DEBUG
    reg=r;
#endif
    paging[5*512+511]=(uint64_t)((uint8_t*)&__corestack)|0b11|(3<<8)|(1<<10)|(1L<<54); // core stack
    // core L3 (lfb)
    for(r=0;r<16*512;r++)
        paging[6*512+r]=(uint64_t)((uint8_t*)bootboot->fb_ptr+r*PAGESIZE)|0b11|(2<<8)|(1<<10)|(2<<2)|(1L<<54); //map framebuffer

#if MEM_DEBUG
    /* dump page translation tables */
    uart_puts("\nTTBR0\n L1 ");
    uart_hex((uint64_t)&__paging,8);
    uart_puts("\n  ");
    uart_hex((uint64_t)paging[0],8);
    uart_puts(" ...\n L2 ");
    uart_hex((uint64_t)&paging[2*512],8);
    uart_puts("\n  ");
    for(r=0;r<4;r++) { uart_hex(paging[2*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=mp-4;r<mp;r++) { uart_hex(paging[2*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=np;r<np+4;r++) { uart_hex(paging[2*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=508;r<512;r++) { uart_hex(paging[2*512+r],8); uart_putc(' '); }
    uart_puts("\n L3 "); uart_hex((uint64_t)&paging[3*512],8); uart_puts("\n  ");
    for(r=0;r<4;r++) { uart_hex(paging[3*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=127;r<131;r++) { uart_hex(paging[3*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=508;r<512;r++) { uart_hex(paging[3*512+r],8); uart_putc(' '); }

    uart_puts("\n\nTTBR1\n L1 ");
    uart_hex((uint64_t)&paging[512],8);
    uart_puts("\n  ... ");
    uart_hex((uint64_t)paging[512+511],8);
    uart_puts("\n L2 ");
    uart_hex((uint64_t)&paging[4*512],8);
    uart_puts("\n  ... (skipped 464) ... ");
    for(r=448;r<451;r++) { uart_hex(paging[4*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=480;r<484;r++) { uart_hex(paging[4*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=508;r<512;r++) { uart_hex(paging[4*512+r],8); uart_putc(' '); }
    uart_puts("\n L3 "); uart_hex((uint64_t)&paging[5*512],8); uart_puts("\n  ");
    for(r=0;r<4;r++) { uart_hex(paging[5*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=reg;r<reg+4;r++) { uart_hex(paging[5*512+r],8); uart_putc(' '); }
    uart_puts("...\n  ... ");
    for(r=508;r<512;r++) { uart_hex(paging[5*512+r],8); uart_putc(' '); }
    uart_puts("\n\n");
#endif
    // enable paging
    reg=(0xFF << 0) |    // Attr=0: normal, IWBWA, OWBWA, NTR
        (0x04 << 8) |    // Attr=1: device, nGnRE (must be OSH too)
        (0x44 <<16);     // Attr=2: non cacheable
    asm volatile ("msr mair_el1, %0" : : "r" (reg));
    reg=(0b00LL << 37) | // TBI=0, no tagging
        ((uint64_t)pa << 32) | // IPS=autodetected
        (0b10LL << 30) | // TG1=4k
        (0b11LL << 28) | // SH1=3 inner
        (0b01LL << 26) | // ORGN1=1 write back
        (0b01LL << 24) | // IRGN1=1 write back
        (0b0LL  << 23) | // EPD1 undocumented by ARM DEN0024A Fig 12-5, 12-6
        (25LL   << 16) | // T1SZ=25, 3 levels (512G)
        (0b00LL << 14) | // TG0=4k
        (0b11LL << 12) | // SH0=3 inner
        (0b01LL << 10) | // ORGN0=1 write back
        (0b01LL << 8) |  // IRGN0=1 write back
        (0b0LL  << 7) |  // EPD0 undocumented by ARM DEN0024A Fig 12-5, 12-6
        (25LL   << 0);   // T0SZ=25, 3 levels (512G)
    asm volatile ("msr tcr_el1, %0; isb" : : "r" (reg));
    asm volatile ("msr ttbr0_el1, %0" : : "r" ((uint64_t)&__paging+1));
    asm volatile ("msr ttbr1_el1, %0" : : "r" ((uint64_t)&__paging+1+PAGESIZE));
    asm volatile ("dsb ish; isb; mrs %0, sctlr_el1" : "=r" (reg));
    // set mandatory reserved bits
    reg|=0xC00800;
    reg&=~( (1<<25) |   // clear EE, little endian translation tables
            (1<<24) |   // clear E0E
            (1<<19) |   // clear WXN
            (1<<12) |   // clear I, no instruction cache
            (1<<4) |    // clear SA0
            (1<<3) |    // clear SA
            (1<<2) |    // clear C, no cache at all
            (1<<1));    // clear A, no aligment check
    reg|=(1<<0)/*|(1<<19)|(1<<12)|(1<<2)*/; // set M enable MMU, WXN, I instruction cache, C data cache
    asm volatile ("msr sctlr_el1, %0; isb" : : "r" (reg));

    // jump to core's _start
#if DEBUG
    uart_puts(" * Entry point ");
    uart_hex(entrypoint,8);
    uart_putc('\n');
#endif
    asm volatile ("mov sp,#-16; mov x30, %0; ret" : : "r" (entrypoint));

    // Wait until Enter or Space pressed, then reboot
error:
    while(r!='\n' && r!=' ') r=uart_getc();
    uart_puts("\n\n");

    // reset
    asm volatile("dsb sy; isb");
    *PM_WATCHDOG = PM_WDOG_MAGIC | 1;
    *PM_RTSC = PM_WDOG_MAGIC | PM_RTSC_FULLRST;
    while(1);
}
