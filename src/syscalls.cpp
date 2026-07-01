
#include <syscalls.h>

using namespace myos;
using namespace myos::common;
using namespace myos::hardwarecommunication;

SyscallHandler::SyscallHandler(InterruptManager* interruptManager, uint8_t InterruptNumber)
:    InterruptHandler(interruptManager, InterruptNumber  + interruptManager->HardwareInterruptOffset())
{
}

SyscallHandler::~SyscallHandler()
{
}


void printf(char*);

uint64_t SyscallHandler::HandleInterrupt(uint64_t esp)
{
    CPUState* cpu = (CPUState*)esp;


    switch(cpu->rax)
    {
        case 4:
            printf((char*)cpu->rbx);
            break;

        default:
            break;
    }


    return esp;
}
