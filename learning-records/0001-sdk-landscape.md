# Established the TI Processor SDK Landscape

The user needed to understand what the "official SDK for BeagleBone AI" actually is. Key insight established: TI ships **two** Processor SDKs (Linux + RTOS), and for baremetal work the RTOS SDK's PDK is the primary resource. Critically, TI supports the AM5729 SoC but NOT the BBAI board — the BBAI is community-supported. The board-specific init (pinmux, DDR, clock tree) must be handwritten, using the TI EVM board files and the BeagleBoard U-Boot fork as reference.

**Evidence:** User demonstrated prior understanding of the boot chain (MLO → U-Boot → kernel), SD card partitioning, and manual image creation. The lesson confirmed they know where our existing code (<code>main.c</code>, <code>boot.asm</code>) maps to SDK components.

**Implications:** Future sessions can dive into specific SDK components (CSL headers, PDK drivers, U-Boot board porting). The SoC-vs-board distinction means the user will need to write their own board init — this is a concrete skill to develop.
