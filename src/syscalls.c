
#include <syscalls.h>

int HandleInterrupt(int esp)
{
  console(9, "inter");
    /*CPUState* cpu = (CPUState*)esp;


    switch(cpu->eax)
    {
        case 4:
            printf((char*)cpu->ebx);
            break;

        default:
            break;
    }*/


    return 0;//esp;
}
