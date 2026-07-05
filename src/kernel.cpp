
#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <jvm/jvm_bridge.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/ata.h>
#include <filesystem/msdospart.h>
#include <filesystem/fat.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <vfs/vfs.h>
#include <multitasking.h>
#include "flint.h"



#include <drivers/amd_am79c973.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>


//#define GRAPHICSMODE


using namespace myos;
using namespace myos::drivers;
using namespace myos::filesystem;
using namespace myos::hardwarecommunication;
using namespace myos::gui;
using namespace myos::net;


void printf(const char* str)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    static uint8_t x = 0, y = 0;

    for(int i = 0; str[i] != '\0'; ++i)
    {
        // Serial output (COM1)
        while (!(myos::hardwarecommunication::Port8Bit::Read8(0x3f8 + 5) & 0x20));
        myos::hardwarecommunication::Port8Bit::Write8(0x3f8, str[i]);

        switch(str[i])
        {
            case '\n':
                x = 0;
                y++;
                break;
            default:
                VideoMemory[80 * y + x] = 0x0F00 | str[i];
                x++;
                break;
        }

        if(x >= 80)
        {
            x = 0;
            y++;
        }

        if(y >= 25)
        {
            for(y = 0; y < 25; y++)
                for(x = 0; x < 80; x++)
                    VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0xFF00) | ' ';
            x = 0;
            y = 0;
        }
    }
}

void printfHex(uint8_t key)
{
    char* foo = "00";
    char* hex = "0123456789ABCDEF";
    foo[0] = hex[(key >> 4) & 0xF];
    foo[1] = hex[key & 0xF];
    printf(foo);
}

void printfHex16(uint16_t key)
{
    printfHex((key >> 8) & 0xFF);
    printfHex( key & 0xFF);
}

void printfHex64(uint64_t key)
{
    printfHex((key >> 56) & 0xFF);
    printfHex((key >> 48) & 0xFF);
    printfHex((key >> 40) & 0xFF);
    printfHex((key >> 32) & 0xFF);
    printfHex((key >> 24) & 0xFF);
    printfHex((key >> 16) & 0xFF);
    printfHex((key >> 8 ) & 0xFF);
    printfHex((key      ) & 0xFF);
}


class PrintfKeyboardEventHandler : public KeyboardEventHandler
{
public:
    void OnKeyDown(char c)
    {
        char* foo = " ";
        foo[0] = c;
        printf(foo);
    }
};

class MouseToConsole : public MouseEventHandler
{
    int8_t x, y;
public:

    MouseToConsole()
    {
        uint16_t* VideoMemory = (uint16_t*)0xb8000;
        x = 40;
        y = 12;
        VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4
                                | (VideoMemory[80 * y + x] & 0xF000) >> 4
                                | (VideoMemory[80 * y + x] & 0x00FF);
    }

    virtual void OnMouseMove(int xoffset, int yoffset)
    {
        static uint16_t* VideoMemory = (uint16_t*)0xb8000;
        VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4
                                | (VideoMemory[80 * y + x] & 0xF000) >> 4
                                | (VideoMemory[80 * y + x] & 0x00FF);

        x += xoffset;
        if(x >= 80) x = 79;
        if(x < 0) x = 0;
        y += yoffset;
        if(y >= 25) y = 24;
        if(y < 0) y = 0;

        VideoMemory[80 * y + x] = (VideoMemory[80 * y + x] & 0x0F00) << 4
                                | (VideoMemory[80 * y + x] & 0xF000) >> 4
                                | (VideoMemory[80 * y + x] & 0x00FF);
    }

};

class PrintfUDPHandler : public UserDatagramProtocolHandler
{
public:
    void HandleUserDatagramProtocolMessage(UserDatagramProtocolSocket* socket, common::uint8_t* data, common::uint16_t size)
    {
        char* foo = " ";
        for(int i = 0; i < size; i++)
        {
            foo[0] = data[i];
            printf(foo);
        }
    }
};


class PrintfTCPHandler : public TransmissionControlProtocolHandler
{
public:
    bool HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket, common::uint8_t* data, common::uint16_t size)
    {
        char* foo = " ";
        for(int i = 0; i < size; i++)
        {
            foo[0] = data[i];
            printf(foo);
        }


        if(size > 9
            && data[0] == 'G'
            && data[1] == 'E'
            && data[2] == 'T'
            && data[3] == ' '
            && data[4] == '/'
            && data[5] == ' '
            && data[6] == 'H'
            && data[7] == 'T'
            && data[8] == 'T'
            && data[9] == 'P'
        )
        {
            socket->Send((uint8_t*)"HTTP/1.1 200 OK\r\nServer: MyOS\r\nContent-Type: text/html\r\n\r\n<html><head><title>My Operating System</title></head><body><b>My Operating System</b> http://www.AlgorithMan.de</body></html>\r\n", 184);
            socket->Disconnect();
        }


        return true;
    }
};


void sysprintf(char* str)
{
    asm("int $0x80" : : "D" (4), "S" (str));
}

void taskA()
{
    while(true)
        sysprintf("A");
}

void taskB()
{
    while(true)
        sysprintf("B");
}


typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}


static uint32_t getMemUpper(const void* mbd, uint32_t magic)
{
    if(magic == 0x36d76289)
    {
        uint32_t totalSize = *(uint32_t*)mbd;
        uint8_t* tagPtr = (uint8_t*)mbd + 8;
        while((uint32_t)(tagPtr - (uint8_t*)mbd) < totalSize)
        {
            uint32_t tagType = *(uint32_t*)tagPtr;
            uint32_t tagSize = *(uint32_t*)(tagPtr + 4);
            if(tagType == 4)
                return *(uint32_t*)(tagPtr + 12);
            if(tagType == 0)
                break;
            tagPtr += (tagSize + 7) & ~7;
        }
        return 0;
    }
    else
    {
        return *(uint32_t*)(((size_t)mbd) + 8);
    }
}

extern "C" void kernelMain(const void* multiboot_structure, uint32_t multiboot_magic)
{
    printf("Hello World! --- http://www.AlgorithMan.de\n\r");

    GlobalDescriptorTable gdt;
    GlobalDescriptorTable::activeGDT = &gdt;


    size_t heap = 10 * 1024 * 1024;
    uint32_t memUpper = getMemUpper(multiboot_structure, multiboot_magic);
    MemoryManager memoryManager(heap, memUpper * 1024 - heap - 10 * 1024);

    printf("heap: 0x");
    printfHex((heap >> 24) & 0xFF);
    printfHex((heap >> 16) & 0xFF);
    printfHex((heap >> 8 ) & 0xFF);
    printfHex((heap      ) & 0xFF);

    void* allocated = memoryManager.malloc(1024);
    printf("\n\rallocated: 0x");
    printfHex64((uint64_t)allocated);
    printf("\n\r");

    TaskManager taskManager;
    TaskManager::activeTaskManager = &taskManager;
    /*
    Task task1(&gdt, taskA);
    Task task2(&gdt, taskB);
    taskManager.AddTask(&task1);
    taskManager.AddTask(&task2);
    */
    
    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80);

    printf("Initializing Hardware, Stage 1\n\r");

    #ifdef GRAPHICSMODE
        Desktop desktop(320, 200, 0x00, 0x00, 0xA8);
    #endif

    DriverManager drvManager;

        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);


        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);

        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        #ifdef GRAPHICSMODE
            VideoGraphicsArray vga;
        #endif

    printf("Initializing Hardware, Stage 2\n\r");
        drvManager.ActivateAll();

    printf("Initializing Hardware, Stage 3\n\r");

    #ifdef GRAPHICSMODE
        vga.SetMode(320, 200, 8);
        Window win1(&desktop, 10, 10, 20, 20, 0xA8, 0x00, 0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40, 15, 30, 30, 0x00, 0xA8, 0x00);
        desktop.AddChild(&win2);
    #endif

    printf("\n\n\n\r");

    printf("\nS-ATA primary master: ");
    AdvancedTechnologyAttachment ata0m(true, 0x1F0);
    ata0m.Identify();

    /*printf("\nS-ATA primary slave: ");
    AdvancedTechnologyAttachment ata0s(false, 0x1F0);
    ata0s.Identify();*/
    printf("VFS initializing...\n\r");
    bool vfsOk = false;
    for (uint8_t part = 0; part < 5; part++) {
        if (myos::vfs::VFS::Initialize(&ata0m, part)) {
            vfsOk = true;
            break;
        }
    }
    if (!vfsOk) {
        printf("Trying secondary master...\n\r");
        AdvancedTechnologyAttachment ata1m(true, 0x170);
        ata1m.Identify();
        for (uint8_t part = 0; part < 5; part++) {
            if (myos::vfs::VFS::Initialize(&ata1m, part)) {
                vfsOk = true;
                break;
            }
        }
    }
    printf("VFS initialized (or failed).\n\r");
    //ata0m.Write28(0, (uint8_t*)"http://www.AlgorithMan.de", 25);
    //ata0m.Flush();
    //ata0m.Read28(0, 25);

    //printf("\nS-ATA secondary master: ");
    //AdvancedTechnologyAttachment ata1m(true, 0x170);
    //ata1m.Identify();

    //printf("\nS-ATA secondary slave: ");
    //AdvancedTechnologyAttachment ata1s(false, 0x170);
    //ata1s.Identify();
    // third: 0x1E8
    // fourth: 0x168


    amd_am79c973* eth0 = (amd_am79c973*)(drvManager.drivers[2]);


    // IP Address
    uint8_t ip1 = 192, ip2 = 168, ip3 = 90, ip4 = 15;
    uint32_t ip_be = ((uint32_t)ip4 << 24)
                | ((uint32_t)ip3 << 16)
                | ((uint32_t)ip2 << 8)
                | (uint32_t)ip1;
    eth0->SetIPAddress(ip_be);
    EtherFrameProvider etherframe(eth0);
    AddressResolutionProtocol arp(&etherframe);

    // IP Address
    uint8_t ip21 = 192, ip22 = 168, ip23 = 90, ip24 = 218;
    uint32_t ip2_be = ((uint32_t)ip24 << 24)
                | ((uint32_t)ip23 << 16)
                | ((uint32_t)ip22 << 8)
                | (uint32_t)ip21;

    // IP Address of the default gateway
    uint8_t gip1 = 192, gip2 = 168, gip3 = 90, gip4 = 254;
    uint32_t gip_be = ((uint32_t)gip4 << 24)
                   | ((uint32_t)gip3 << 16)
                   | ((uint32_t)gip2 << 8)
                   | (uint32_t)gip1;

    uint8_t subnet1 = 255, subnet2 = 255, subnet3 = 255, subnet4 = 0;
    uint32_t subnet_be = ((uint32_t)subnet4 << 24)
                   | ((uint32_t)subnet3 << 16)
                   | ((uint32_t)subnet2 << 8)
                   | (uint32_t)subnet1;

    InternetProtocolProvider ipv4(&etherframe, &arp, gip_be, subnet_be);
    InternetControlMessageProtocol icmp(&ipv4);
    UserDatagramProtocolProvider udp(&ipv4);
    TransmissionControlProtocolProvider tcp(&ipv4);


    // printf("Activating interrupts...\n");
    // interrupts.Activate();
    // printf("Interrupts active.\n");

    //printf("\n\n\n\n");

    //arp.BroadcastMACAddress(gip_be);
    //arp.BroadcastMACAddress(ip2_be);

    //PrintfTCPHandler tcphandler;
    //TransmissionControlProtocolSocket* tcpsocket = tcp.Listen(80);
    //TransmissionControlProtocolSocket* tcpsocket = tcp.Connect(ip2_be, 1234);
    //tcp.Bind(tcpsocket, &tcphandler);
    //tcpsocket->Send((uint8_t*)"Hello TCP!", 10);


    //icmp.RequestEchoReply(gip_be);

    //PrintfUDPHandler udphandler;
    //UserDatagramProtocolSocket* udpsocket = udp.Connect(ip2_be, 1234);
    //udp.Bind(udpsocket, &udphandler);
    //udpsocket->Send((uint8_t*)"Hello UDP!", 10);

    //UserDatagramProtocolSocket* udpsocket = udp.Listen(1234);
    //udp.Bind(udpsocket, &udphandler);


    printf("Starting JVM...\n\r");
    myos::jvm::JVMBridge::Initialize();
    printf("Running JVM main...\n\r");
    Flint::runToMain("x86os/Hello");
    printf("JVM finished.\n\r");

    while(1)
    {
    /*#ifdef GRAPHICSMODE
        desktop.Draw(&vga);
    #endif*/
        
        __asm__ volatile("hlt"); 
    }
}
