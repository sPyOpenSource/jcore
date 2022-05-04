/* DO NOT MODIFIY - AUTOMATICALLY GENERATED FILE */
/* FILE_CAN_BE_DELETED_AT_ANY_TIME */

#ifndef SYMBOLS_H
#define SYMBOLS_H

struct symbols_s {
  unsigned long name;
  unsigned long addr;
  unsigned long size;
};

#define FKTSIZE_real_to_prot 0x00000000
#define FKTADDR_real_to_prot 0x00009000
#define FKTSIZE__start 0x00000000
#define FKTADDR__start 0x00009000
#define FKTSIZE_testreal 0x00000000
#define FKTADDR_testreal 0x000090c9
#define FKTSIZE_vesa2_detect 0x00000000
#define FKTADDR_vesa2_detect 0x0000908b
#define FKTSIZE_vesa2_get_mode_info 0x00000000
#define FKTADDR_vesa2_get_mode_info 0x000090dd
#define FKTSIZE_vesa2_set_mode 0x00000000
#define FKTADDR_vesa2_set_mode 0x00009120

static char strings[] __attribute__ ((unused)) = "real_to_prot\0_start\0testreal\0vesa2_detect\0vesa2_get_mode_info\0vesa2_set_mode\0";

static struct symbols_s symbols[] __attribute__ ((unused)) = {{ 0, 0x00009000, 0x00000000 },
{ 13, 0x00009000, 0x00000000 },
{ 20, 0x000090c9, 0x00000000 },
{ 29, 0x0000908b, 0x00000000 },
{ 42, 0x000090dd, 0x00000000 },
{ 62, 0x00009120, 0x00000000 },

};
static int n_symbols __attribute__ ((unused)) = 6;
#endif
