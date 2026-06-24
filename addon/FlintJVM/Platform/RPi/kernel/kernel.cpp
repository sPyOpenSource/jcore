
#include "kernel.h"
#include <circle/koptions.h>
#include <circle/memory.h>
#include <circle/string.h>
#include <circle/util.h>
#include <stdint.h>
#include <string.h>
#include "flint.h"
#include "flint_system_api.h"

static CKernel *kernel = nullptr;

// Reserve 1MB for JVM heap
static uint8_t jvm_heap[0x100000] __attribute__((aligned(16)));

extern "C" {
    void rpi_uart_puts(const char *s) {
        if (kernel) {
            kernel->WriteSerial(s);
        }
    }

    void rpi_uart_putc(char c) {
        if (kernel) {
            char buf[2] = {c, 0};
            kernel->WriteSerial(buf);
        }
    }

    uint64_t rpi_get_ticks(void) {
        uint64_t ticks;
        asm volatile("mrs %0, cntvct_el0" : "=r"(ticks));
        return ticks;
    }
    bool Flint_runToMain(const char *cls);
}

CKernel::CKernel(void)
: m_ActLED(),
  m_Serial()
{
    kernel = this;
}

CKernel::~CKernel(void)
{
    kernel = nullptr;
}

boolean CKernel::Initialize(void)
{
    if (!m_Serial.Initialize(115200)) {
        return FALSE;
    }

    m_Serial.Write("FlintJVM-RPi: Initializing...\r\n", 31);

    return TRUE;
}

TShutdownMode CKernel::Run(void)
{
    m_Serial.Write("FlintJVM-RPi: Starting JVM...\r\n", 30);

    void *heapStart = jvm_heap;
    void *heapEnd = (void *)((uintptr_t)jvm_heap + sizeof(jvm_heap));
    FlintAPI::System::initHeap(heapStart, heapEnd);

    if (Flint_runToMain("com/rpi/HelloRPi")) {
        m_Serial.Write("FlintJVM-RPi: JVM execution completed.\r\n", 40);
    } else {
        m_Serial.Write("FlintJVM-RPi: JVM execution failed.\r\n", 37);
    }

    return ShutdownHalt;
}
