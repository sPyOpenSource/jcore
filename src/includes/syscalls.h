
#ifndef __MYOS__SYSCALLS_H
#define __MYOS__SYSCALLS_H

#include <common/types.h>
#include <hardwarecommunication/interrupts.h>
#include <multitasking.h>

namespace myos
{
    class SyscallHandler : public hardwarecommunication::InterruptHandler
    {
    public:
        SyscallHandler(hardwarecommunication::InterruptManager* interruptManager, myos::common::uint8_t InterruptNumber);
        ~SyscallHandler();

            virtual myos::common::uint64_t HandleInterrupt(myos::common::uint64_t esp);
    };
}


#endif
