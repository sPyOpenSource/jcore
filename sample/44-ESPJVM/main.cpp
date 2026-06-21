
#include <circle/startup.h>
#include <circle/types.h>
#include "kernel.h"

int main(void) {
    CKernel Kernel;
    if (!Kernel.Initialize()) {
        halt();
        return EXIT_HALT;
    }

    TShutdownMode ShutdownMode = Kernel.Run();

    switch (ShutdownMode) {
    case ShutdownReboot:
        reboot();
        return EXIT_REBOOT;

    case ShutdownHalt:
    default:
        halt();
        return EXIT_HALT;
    }
}

