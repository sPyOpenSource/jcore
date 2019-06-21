#include <filesystem/msdospart.h>
#include <filesystem/FatStructs.h>

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::filesystem;

void printf(char*);
void printfHex(uint8_t);

void MSDOSPartitionTable::ReadPartitions(myos::drivers::AdvancedTechnologyAttachment *hd)
{
  MasterBootRecord mbr;
  hd->Read28(0, (uint8_t*)&mbr, (uint16_t)sizeof(MasterBootRecord));

  /*printf("MBR: ");
  for(int i = 446; i < 446 + 4 * 16; i++){
    printfHex(((uint8_t*)&mbr)[i]);
    printf(" ");
  }
  printf("\n");*/

  if(mbr.magicnumber != 0xAA55){
    printf("illegal MBR");
    return;
  }

  for(int i = 0; i < 4; i++){
    if(mbr.parimaryPartition[i].partition_id == 0x00)
      continue;

    printf(" Partition ");
    printfHex(i & 0xff);

    if(mbr.parimaryPartition[i].bootable == 0x80)
      printf(" bootable");
    else
      printf(" not bootable. Type ");

    printfHex(mbr.parimaryPartition[i].partition_id);

    ReadBiosBlock(hd, mbr.parimaryPartition[i].start_lba);
  }
}
