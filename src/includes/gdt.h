#ifndef __MYOS__GDT_H
#define __MYOS__GDT_H

#include <common/types.h>

namespace myos
{
    class GlobalDescriptorTable
    {
        public:
            static GlobalDescriptorTable *activeGDT;

            class SegmentDescriptor
            {
                private:
                    common::uint64_t value;
                public:
                    SegmentDescriptor(common::uint64_t val) : value(val) {}
                    common::uint64_t Base() { return 0; }
                    common::uint64_t Limit() { return 0; }
            } __attribute__((packed));

            struct TSSDescriptor {
                common::uint32_t limit_lo;
                common::uint32_t base_lo;
                common::uint32_t base_mid;
                common::uint8_t type;
                common::uint8_t flags;
                common::uint32_t base_hi;
                common::uint32_t reserved;
            } __attribute__((packed));

            private:
            SegmentDescriptor nullSegmentSelector;
            SegmentDescriptor codeSegmentSelector;
            SegmentDescriptor dataSegmentSelector;
            TSSDescriptor tssDescriptor;

        public:

            GlobalDescriptorTable();
            ~GlobalDescriptorTable();

            common::uint16_t CodeSegmentSelector();
            common::uint16_t DataSegmentSelector();
    };
}

#endif