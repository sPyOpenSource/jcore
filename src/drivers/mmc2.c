#include "mmc2.h"

static struct mmc2_state mmc2_state_storage = {0};

/* Poll for a STAT bit, timeout in loop iterations */
static int wait_for_stat(uint32_t mask, uint32_t timeout) {
    while (timeout--) {
        uint32_t stat = mmc2_read(MMCHS_STAT);
        if (stat & mask) {
            /* Clear the status bit(s) we matched */
            mmc2_write(MMCHS_STAT, stat & mask);
            return 0;
        }
        if (stat & (STAT_ERRI | STAT_CTO | STAT_CCRC | STAT_DTO | STAT_DCRC)) {
            mmc2_write(MMCHS_STAT, stat);
            return -1;
        }
    }
    return -2; /* timeout */
}

void mmc2_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t *)reg = val;
}

uint32_t mmc2_read(uint32_t reg) {
    return *(volatile uint32_t *)reg;
}

void mmc2_send_cmd(uint32_t cmd_idx, uint32_t arg, int resp_type) {
    /* Wait for command line idle */
    uint32_t pstate;
    do {
        pstate = mmc2_read(MMCHS_PSTATE);
    } while (pstate & PSTATE_CMDI);

    /* Clear status */
    mmc2_write(MMCHS_STAT, mmc2_read(MMCHS_STAT));

    /* Set argument */
    mmc2_write(MMCHS_ARG, arg);

    /* Build command register */
    uint32_t cmd = (cmd_idx << CMD_INDX_SHIFT) | CMD_CMD_EN;

    switch (resp_type) {
    case RESP_NONE:
        cmd |= CMD_DP_NONE;
        break;
    case RESP_R1:
    case RESP_R1b:
        cmd |= CMD_DP_NONE | CMD_CCC_EXP;
        break;
    case RESP_R2:
        cmd |= CMD_DP_NONE | CMD_CCC_EXP | CMD_CEATA;
        break;
    case RESP_R3:
        cmd |= CMD_DP_NONE | CMD_CCC_EXP | CMD_CEATA;
        break;
    case RESP_R6:
        cmd |= CMD_DP_NONE | CMD_CCC_EXP;
        break;
    case RESP_R7:
        cmd |= CMD_DP_NONE | CMD_CCC_EXP;
        break;
    }

    mmc2_write(MMCHS_CMD, cmd);

    /* Wait for command complete */
    int ret = wait_for_stat(STAT_CC, 1000000);
    (void)ret;
}

uint32_t mmc2_get_response(int resp_idx) {
    switch (resp_idx) {
    case 0: return mmc2_read(MMCHS_RSP10);
    case 1: return mmc2_read(MMCHS_RSP32);
    case 2: return mmc2_read(MMCHS_RSP54);
    case 3: return mmc2_read(MMCHS_RSP76);
    default: return 0;
    }
}

/* SD card initialization sequence */
int mmc2_init(void) {
    struct mmc2_state *state = &mmc2_state_storage;
    uint32_t resp;

    /* Step 1: Reset the MMC controller */
    mmc2_write(MMCHS_SYSCONFIG, SYSCONFIG_SOFTRESET);
    while (!(mmc2_read(MMCHS_SYSSTATUS) & SYSSTATUS_RESETDONE));

    /* Step 2: Set initial clock to 400 kHz (identification mode) */
    uint32_t sysctl = mmc2_read(MMCHS_SYSCTL);
    sysctl &= ~(SYSCTL_CLKD_MASK | SYSCTL_DKO_MASK);
    sysctl |= (0x80 << SYSCTL_CLKD_SHIFT); /* CLKD = 128 -> ~400 kHz from 64 MHz */
    sysctl |= SYSCTL_ICE | SYSCTL_CLKEN;
    mmc2_write(MMCHS_SYSCTL, sysctl);

    /* Step 3: Enable SD bus power */
    uint32_t hctl = mmc2_read(MMCHS_HCTL);
    hctl |= HCTL_SDBP;
    hctl &= ~HCTL_SDVS_MASK;
    hctl |= HCTL_SDVS_VS30; /* 3.0V */
    mmc2_write(MMCHS_HCTL, hctl);

    /* Wait for bus power */
    while (!(mmc2_read(MMCHS_HCTL) & HCTL_SDBP));

    /* Step 4: Set CON to SD mode (not MMC) */
    uint32_t con = mmc2_read(MMCHS_CON);
    con &= ~(CON_DW8 | CON_OD);
    mmc2_write(MMCHS_CON, con);

    /* Step 5: CMD0 - GO_IDLE_STATE */
    mmc2_send_cmd(MMC_CMD0, 0, RESP_NONE);

    /* Step 6: CMD8 - SEND_IF_COND (check SDHC support) */
    mmc2_send_cmd(MMC_CMD8, (CMD8_VOLTAGE_27_36 << 8) | CMD8_CHECK_PATTERN, RESP_R7);
    resp = mmc2_get_response(0);
    state->is_sdhc = ((resp & 0xFF) == CMD8_CHECK_PATTERN);

    /* Step 7: ACMD41 - SD_SEND_OP_COND */
    uint32_t acmd41_arg = ACMD41_VOLTAGE;
    if (state->is_sdhc) {
        acmd41_arg |= ACMD41_HCS;
    }

    int timeout = 1000000;
    do {
        mmc2_send_cmd(MMC_CMD55, 0, RESP_R1); /* next cmd is app cmd */
        mmc2_send_cmd(MMC_ACMD41, acmd41_arg, RESP_R3);
        resp = mmc2_get_response(0);
        if (timeout-- == 0) return -1;
    } while (!(resp & (1U << 31))); /* wait for card busy clear */

    if (state->is_sdhc && (resp & (1U << 30))) {
        /* Card is SDHC/SDXC */
    }

    /* Step 8: CMD2 - ALL_SEND_CID */
    mmc2_send_cmd(MMC_CMD2, 0, RESP_R2);
    /* CID is in RSP10, RSP32, RSP54, RSP76 */
    uint32_t cid[4];
    cid[0] = mmc2_get_response(0);
    cid[1] = mmc2_get_response(1);
    cid[2] = mmc2_get_response(2);
    cid[3] = mmc2_get_response(3);
    (void)cid;

    /* Step 9: CMD3 - SEND_RELATIVE_ADDR */
    mmc2_send_cmd(MMC_CMD3, 0, RESP_R6);
    resp = mmc2_get_response(0);
    state->rca = resp & 0xFFFF0000;

    /* Step 10: CMD9 - SEND_CSD */
    mmc2_send_cmd(MMC_CMD9, state->rca, RESP_R2);
    uint32_t csd[4];
    csd[0] = mmc2_get_response(0);
    csd[1] = mmc2_get_response(1);
    csd[2] = mmc2_get_response(2);
    csd[3] = mmc2_get_response(3);
    (void)csd;

    /* Step 11: CMD7 - SELECT_CARD */
    mmc2_send_cmd(MMC_CMD7, state->rca, RESP_R1b);

    /* Step 12: ACMD6 - SET_BUS_WIDTH (4-bit) */
    mmc2_send_cmd(MMC_CMD55, state->rca, RESP_R1);
    mmc2_send_cmd(MMC_ACMD6, 2, RESP_R1); /* 2 = 4-bit bus */
    state->is_4bit = true;

    /* Step 13: Switch to 4-bit mode in HCTL */
    hctl = mmc2_read(MMCHS_HCTL);
    hctl &= ~HCTL_DTW_MASK;
    hctl |= HCTL_DTW_4BIT;
    mmc2_write(MMCHS_HCTL, hctl);

    /* Step 14: Increase clock for normal operation */
    mmc2_write(MMCHS_SYSCTL, SYSCTL_SRC); /* soft reset */
    while (mmc2_read(MMCHS_SYSCTL) & SYSCTL_SRA);

    sysctl = mmc2_read(MMCHS_SYSCTL);
    sysctl &= ~(SYSCTL_CLKD_MASK | SYSCTL_DKO_MASK);
    sysctl |= (4 << SYSCTL_CLKD_SHIFT); /* CLKD = 4 -> ~25 MHz from 200 MHz */
    sysctl |= SYSCTL_ICE | SYSCTL_CLKEN;
    mmc2_write(MMCHS_SYSCTL, sysctl);

    /* Wait for clock stable */
    while (!(mmc2_read(MMCHS_SYSCTL) & SYSCTL_CLKEN));

    /* Step 15: Set block size to 512 */
    mmc2_write(MMCHS_BLK, 512);
    mmc2_write(MMCHS_CON, con | (1 << CON_DVAL_SHIFT)); /* set dval */

    state->ready = true;
    return 0;
}

int mmc2_read_blocks(uint32_t lba, uint32_t count, void *buf) {
    struct mmc2_state *state = &mmc2_state_storage;
    if (!state->ready) return -1;

    uint8_t *ptr = (uint8_t *)buf;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t addr;
        if (state->is_sdhc) {
            addr = lba + i; /* SDHC uses block addressing */
        } else {
            addr = (lba + i) * 512; /* SD uses byte addressing */
        }

        /* CMD17 - READ_SINGLE_BLOCK */
        mmc2_write(MMCHS_BLK, 512);
        mmc2_send_cmd(MMC_CMD17, addr, RESP_R1);

        /* Wait for data ready */
        if (wait_for_stat(STAT_BRR, 1000000) < 0) return -2;

        /* Read data from DATA port (PIO) */
        for (int j = 0; j < 128; j++) {
            uint32_t word = mmc2_read(MMCHS_DATA);
            ptr[0] = (word >> 0) & 0xFF;
            ptr[1] = (word >> 8) & 0xFF;
            ptr[2] = (word >> 16) & 0xFF;
            ptr[3] = (word >> 24) & 0xFF;
            ptr += 4;
        }

        /* Wait for transfer complete */
        wait_for_stat(STAT_TC, 1000000);
    }

    return 0;
}

int mmc2_write_blocks(uint32_t lba, uint32_t count, const void *buf) {
    struct mmc2_state *state = &mmc2_state_storage;
    if (!state->ready) return -1;

    const uint8_t *ptr = (const uint8_t *)buf;

    for (uint32_t i = 0; i < count; i++) {
        uint32_t addr;
        if (state->is_sdhc) {
            addr = lba + i;
        } else {
            addr = (lba + i) * 512;
        }

        /* CMD24 - WRITE_SINGLE_BLOCK */
        mmc2_write(MMCHS_BLK, 512);
        mmc2_send_cmd(MMC_CMD24, addr, RESP_R1);

        /* Wait for buffer write enable */
        if (wait_for_stat(STAT_BWR, 1000000) < 0) return -2;

        /* Write data to DATA port (PIO) */
        for (int j = 0; j < 128; j++) {
            uint32_t word = ((uint32_t)ptr[3] << 24) |
                            ((uint32_t)ptr[2] << 16) |
                            ((uint32_t)ptr[1] << 8) |
                            ((uint32_t)ptr[0] << 0);
            mmc2_write(MMCHS_DATA, word);
            ptr += 4;
        }

        /* Wait for transfer complete */
        wait_for_stat(STAT_TC, 1000000);
    }

    return 0;
}
