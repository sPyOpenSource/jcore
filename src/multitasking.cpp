
#include <multitasking.h>

using namespace myos;
using namespace myos::common;


Task::Task(GlobalDescriptorTable *gdt, void entrypoint())
{
    cpustate = (CPUState*)(stack + 8192 - sizeof(CPUState));

    cpustate->rax = 0;
    cpustate->rbx = 0;
    cpustate->rcx = 0;
    cpustate->rdx = 0;

    cpustate->rsi = 0;
    cpustate->rdi = 0;
    cpustate->rbp = 0;

    cpustate->r8 = 0;
    cpustate->r9 = 0;
    cpustate->r10 = 0;
    cpustate->r11 = 0;
    cpustate->r12 = 0;
    cpustate->r13 = 0;
    cpustate->r14 = 0;
    cpustate->r15 = 0;

    cpustate->rip = (uint64_t)entrypoint;
    cpustate->cs = gdt->CodeSegmentSelector();
    cpustate->rflags = 0x202;
}

Task::~Task()
{
}


TaskManager::TaskManager()
{
    numTasks = 0;
    currentTask = -1;
}

TaskManager::~TaskManager()
{
}

bool TaskManager::AddTask(Task* task)
{
    if(numTasks >= 256)
        return false;
    tasks[numTasks++] = task;
    return true;
}

CPUState* TaskManager::Schedule(CPUState* cpustate)
{
    if(numTasks <= 0)
        return cpustate;

    if(currentTask >= 0)
        tasks[currentTask]->cpustate = cpustate;

    if(++currentTask >= numTasks)
        currentTask %= numTasks;
    return tasks[currentTask]->cpustate;
}
