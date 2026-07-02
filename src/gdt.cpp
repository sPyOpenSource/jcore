#include <gdt.h>
using namespace myos;
using namespace myos::common;

GlobalDescriptorTable *GlobalDescriptorTable::activeGDT = nullptr;

GlobalDescriptorTable::GlobalDescriptorTable()
    : nullSegmentSelector(0),
      codeSegmentSelector((1ULL << 43) | (1ULL << 44) | (1ULL << 47) | (1ULL << 53)),
      dataSegmentSelector((1ULL << 44) | (1ULL << 47) | (1ULL << 41))
{
    uint8_t gdt_ptr[10];
    gdt_ptr[0] = (uint8_t)(sizeof(GlobalDescriptorTable) - 1);
    gdt_ptr[1] = (uint8_t)((sizeof(GlobalDescriptorTable) - 1) >> 8);
    
    uint64_t base = (uint64_t)this;
    for(int i = 0; i < 8; i++)
        gdt_ptr[2 + i] = (uint8_t)(base >> (i * 8));
    
    asm volatile("lgdt (%0)" : : "r" (gdt_ptr) : "memory");
}

GlobalDescriptorTable::~GlobalDescriptorTable()
{
}

uint16_t GlobalDescriptorTable::CodeSegmentSelector()
{
    return 8;
}

uint16_t GlobalDescriptorTable::DataSegmentSelector()
{
    return 16;
}