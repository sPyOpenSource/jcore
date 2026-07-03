#ifndef MMC2_H
#define MMC2_H

#include <stdint.h>
#include <stdbool.h>

/* AM572x MMC2 base address */
#define MMC2_BASE           0x480AD000

/* MMC/SD Register Offsets (OMAP HSMMC) */
#define MMCHS_SYSCONFIG     (MMC2_BASE + 0x010)
#define MMCHS_SYSSTATUS     (MMC2_BASE + 0x014)
#define MMCHS_CON           (MMC2_BASE + 0x02C)
#define MMCHS_BLK           (MMC2_BASE + 0x030)
#define MMCHS_ARG           (MMC2_BASE + 0x038)
#define MMCHS_CMD           (MMC2_BASE + 0x03C)
#define MMCHS_RSP10         (MMC2_BASE + 0x040)
#define MMCHS_RSP32         (MMC2_BASE + 0x044)
#define MMCHS_RSP54         (MMC2_BASE + 0x048)
#define MMCHS_RSP76         (MMC2_BASE + 0x04C)
#define MMCHS_DATA          (MMC2_BASE + 0x050)
#define MMCHS_PSTATE        (MMC2_BASE + 0x054)
#define MMCHS_HCTL          (MMC2_BASE + 0x058)
#define MMCHS_SYSCTL        (MMC2_BASE + 0x05C)
#define MMCHS_STAT          (MMC2_BASE + 0x060)
#define MMCHS_IE            (MMC2_BASE + 0x064)
#define MMCHS_ISE           (MMC2_BASE + 0x068)
#define MMCHS_CSRE          (MMC2_BASE + 0x06C)
#define MMCHS_SYSTEST       (MMC2_BASE + 0x06C)
#define MMCHS_CAPA          (MMC2_BASE + 0x070)
#define MMCHS_CAPA2         (MMC2_BASE + 0x074)
#define MMCHS_CUR_CAPA      (MMC2_BASE + 0x078)
#define MMCHS_FEAT          (MMC2_BASE + 0x07C)
#define MMCHS_ADMA_ES       (MMC2_BASE + 0x084)
#define MMCHS_ADMA_SAL      (MMC2_BASE + 0x088)
#define MMCHS_DLL           (MMC2_BASE + 0x0B4)
#define MMCHS_DLL2          (MMC2_BASE + 0x0B8)

/* SYSCONFIG bits */
#define SYSCONFIG_SOFTRESET     (1U << 1)
#define SYSCONFIG_AUTOIDLE      (1U << 0)

/* SYSSTATUS bits */
#define SYSSTATUS_RESETDONE     (1U << 0)

/* CON bits */
#define CON_DW8             (1U << 5)
#define CON_DVAL_SHIFT      9
#define CON_DVAL_MASK       (0x1F << CON_DVAL_SHIFT)
#define CON_INIT            (1U << 1)
#define CON_OD              (1U << 0)

/* CMD bits */
#define CMD_INDX_SHIFT      24
#define CMD_CMD_EN          (1U << 16)
#define CMD_CEATA           (1U << 15)
#define CMD_DP_SHIFT        12
#define CMD_DP_NONE         (0U << 12)
#define CMD_DP_READ         (1U << 12)
#define CMD_DP_WRITE        (2U << 12)
#define CMD_BC              (1U << 11)
#define CMD_AC              (1U << 10)
#define CMD_CCCE            (1U << 9)
#define CMD_CCC_EXP         (1U << 8)
#define CMD_MSMS            (1U << 7)
#define CMD_MSMS_SG         (1U << 6)

/* PSTATE bits */
#define PSTATE_CMDI         (1U << 0)
#define PSTATE_DATI         (1U << 1)
#define PSTATE_BRE          (1U << 11)
#define PSTATE_BWE          (1U << 10)
#define PSTATE_CDET         (1U << 9)
#define PSTATE_BUSY         (1U << 16)
#define PSTATE_DATL_SHIFT   20
#define PSTATE_DATL_MASK    (0x1F << PSTATE_DATL_SHIFT)

/* SYSCTL bits */
#define SYSCTL_SRC          (1U << 25)
#define SYSCTL_SRA          (1U << 24)
#define SYSCTL_CLKD_SHIFT   6
#define SYSCTL_CLKD_MASK    (0x3FF << SYSCTL_CLKD_SHIFT)
#define SYSCTL_DKO_MASK     (0xF << 16)
#define SYSCTL_ICE          (1U << 1)
#define SYSCTL_CLKEN        (1U << 0)

/* STAT bits */
#define STAT_CC             (1U << 0)
#define STAT_TC             (1U << 1)
#define STAT_BWR            (1U << 4)
#define STAT_BRR            (1U << 5)
#define STAT_CIRQ           (1U << 8)
#define STAT_ERRI           (1U << 15)
#define STAT_CTO            (1U << 16)
#define STAT_CCRC           (1U << 17)
#define STAT_CEB            (1U << 18)
#define STAT_CIE            (1U << 19)
#define STAT_DTO            (1U << 20)
#define STAT_DCRC           (1U << 21)
#define STAT_DEB            (1U << 22)
#define STAT_ACE            (1U << 24)
#define STAT_ADE            (1U << 25)

/* CAPA bits */
#define CAPA_VS18           (1U << 26)
#define CAPA_VS30           (1U << 25)
#define CAPA_VS33           (1U << 24)

/* HCTL bits */
#define HCTL_DTW_SHIFT      1
#define HCTL_DTW_MASK       (3U << HCTL_DTW_SHIFT)
#define HCTL_DTW_1BIT       (0U << HCTL_DTW_SHIFT)
#define HCTL_DTW_4BIT       (1U << HCTL_DTW_SHIFT)
#define HCTL_DTW_8BIT       (2U << HCTL_DTW_SHIFT)
#define HCTL_SDBP           (1U << 8)
#define HCTL_SDVS_SHIFT     9
#define HCTL_SDVS_MASK      (7U << HCTL_SDVS_SHIFT)
#define HCTL_SDVS_VS30      (5U << HCTL_SDVS_SHIFT)
#define HCTL_SDVS_VS18      (3U << HCTL_SDVS_SHIFT)

/* MMC Commands */
#define MMC_CMD0            0
#define MMC_CMD2            2
#define MMC_CMD3            3
#define MMC_CMD7            7
#define MMC_CMD8            8
#define MMC_CMD9            9
#define MMC_CMD12           12
#define MMC_CMD13           13
#define MMC_CMD16           16
#define MMC_CMD17           17
#define MMC_CMD18           18
#define MMC_CMD24           24
#define MMC_CMD55           55
#define MMC_ACMD6           6
#define MMC_ACMD23          23
#define MMC_ACMD41          41

/* ACMD41 bits */
#define ACMD41_HCS          (1U << 30)
#define ACMD41_S18R         (1U << 24)
#define ACMD41_VOLTAGE      (0x00FF8000)

/* CMD8 pattern */
#define CMD8_CHECK_PATTERN  0xAA
#define CMD8_VOLTAGE_27_36  0x1

/* Response types for mmc2_send_cmd */
#define RESP_NONE           0
#define RESP_R1             1
#define RESP_R1b            2
#define RESP_R2             3
#define RESP_R3             4
#define RESP_R6             5
#define RESP_R7             6

/* Block device interface */
struct mmc_block_dev {
    int (*read_blocks)(uint32_t lba, uint32_t count, void *buf);
    int (*write_blocks)(uint32_t lba, uint32_t count, const void *buf);
    uint32_t block_size;
    uint32_t num_blocks;
    bool is_sdhc;
};

/* MMC2 state */
struct mmc2_state {
    uint32_t rca;
    uint32_t clock_hz;
    bool is_sdhc;
    bool is_4bit;
    bool ready;
};

/* API */
void mmc2_write(uint32_t reg, uint32_t val);
uint32_t mmc2_read(uint32_t reg);
void mmc2_send_cmd(uint32_t cmd_idx, uint32_t arg, int resp_type);
uint32_t mmc2_get_response(int resp_idx);
int mmc2_init(void);
int mmc2_read_blocks(uint32_t lba, uint32_t count, void *buf);
int mmc2_write_blocks(uint32_t lba, uint32_t count, const void *buf);

#endif /* MMC2_H */