#ifndef __MYOS__MULTITASKING_H
#define __MYOS__MULTITASKING_H

#include <common/types.h>
#include <gdt.h>

namespace myos
{
    struct CPUState
    {
        common::uint64_t r15;
        common::uint64_t r14;
        common::uint64_t r13;
        common::uint64_t r12;
        common::uint64_t r11;
        common::uint64_t r10;
        common::uint64_t r9;
        common::uint64_t r8;
        common::uint64_t rax;
        common::uint64_t rbx;
        common::uint64_t rcx;
        common::uint64_t rdx;
        common::uint64_t rsi;
        common::uint64_t rdi;
        common::uint64_t rbp;
        common::uint64_t error;
        common::uint64_t rip;
        common::uint64_t cs;
        common::uint64_t rflags;
        common::uint64_t rsp;
        common::uint64_t ss;
    } __attribute__((packed));


    class Task
    {
    friend class TaskManager;
    private:
        common::uint8_t stack[8192]; // 8 KiB
        CPUState* cpustate;
    public:
        Task(GlobalDescriptorTable *gdt, void entrypoint());
        ~Task();
    };


    class TaskManager
    {
    private:
        Task* tasks[256];
        int numTasks;
        int currentTask;
    public:
        TaskManager();
        ~TaskManager();
        bool AddTask(Task* task);
        CPUState* Schedule(CPUState* cpustate);
    };
}

#endif
