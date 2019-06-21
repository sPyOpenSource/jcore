#include <filesystem/FatStructs.h>

using namespace myos;
using namespace myos::common;
//using namespace myos::filesystem;
using namespace myos::drivers;

void printf(char*);
void printfHex(uint8_t);

void ReadBiosBlock(myos::drivers::AdvancedTechnologyAttachment *hd, common::uint32_t partitionOffset)
{
  fbs_t bpb;
  hd->Read28(partitionOffset, (uint8_t*)&bpb, (uint16_t)sizeof(fbs_t));

  /*printf("sectors per cluster: ");
  printfHex(bpb.sectorsPerCluster);
  printf("\n");*/

  /*for(int i = 0x00; i < sizeof(BiosParameterBlock32); i++){
    printfHex(((uint8_t*)&bpb)[i]);
    printf(" ");
  }
  printf("\n");*/

  uint32_t fatStart = partitionOffset + bpb.bpb.reservedSectorCount;
  uint32_t fatSize = bpb.bpb.sectorsPerFat32;

  uint32_t dataStart = fatStart + fatSize * bpb.bpb.fatCount;

  uint32_t rootStart = dataStart + bpb.bpb.sectorsPerCluster * (bpb.bpb.fat32RootCluster - 2);
  rootStart = partitionOffset + 6 + bpb.bpb.reservedSectorCount;
  dir_t dirent[16];
  hd->Read28(rootStart, (uint8_t*)&dirent, (uint16_t)(16 * sizeof(dir_t)));

  for(int i = 0x00; i < sizeof(dir_t); i++){
    printfHex(((uint8_t*)&dirent)[i]);
    printf(" ");
  }
  printf("\n");

  for(int i = 0; i < 16; i++)
  {
    if(dirent[i].name[0] == 0x00)
      break;

    if((dirent[i].attributes & 0x0f) == 0x0f)
      continue;

    char* foo = "        \n";
    for(int j = 0; j < 8; j++)
      foo[j] = dirent[i].name[j];
    printf(foo);

    if((dirent[i].attributes & 0x10) == 0x10) // directory
      continue;

    uint32_t firstFileCluster = ((uint32_t)dirent[i].firstClusterHigh) << 16
                              | ((uint32_t)dirent[i].firstClusterLow);

    int32_t SIZE = dirent[i].fileSize;
    int32_t nextFileCluster = firstFileCluster;
    uint8_t buffer[513];
    uint8_t fatbuffer[513];

    while(SIZE > 0)
    {
      //uint32_t fileSector = dataStart + bpb.sectorsPerCluster * (fileCluster - 2);
      uint32_t fileSector = partitionOffset + 38 + bpb.bpb.reservedSectorCount + bpb.bpb.sectorsPerCluster * (nextFileCluster - 2);
      int sectorOffset = 0;

      for(; SIZE > 0; SIZE -= 512)
      {
        hd->Read28(fileSector + sectorOffset, buffer, 512);

        buffer[SIZE > 512 ? 512 : SIZE] = '\0';
        printf((char*)buffer);

        if(++sectorOffset > bpb.bpb.sectorsPerCluster)
          break;
      }

      uint32_t fatSectorForCurrentCluster = nextFileCluster / (512 / sizeof(uint32_t));
      hd->Read28(fatStart + fatSectorForCurrentCluster, fatbuffer, 512);
      uint32_t fatOffsetInSectorForCurrentCluster = nextFileCluster % (512 / sizeof(uint32_t));
      nextFileCluster = ((uint32_t*)&fatbuffer)[fatOffsetInSectorForCurrentCluster] & 0x0FFFFFFF;
    }
    printf("\n");
  }
}
