
#ifndef _kernel_h
#define _kernel_h

#include <circle/actled.h>
#include <circle/koptions.h>
#include <circle/peripherals/serial.h>
#include <circle/types.h>
#include <string.h>

enum TShutdownMode
{
    ShutdownNone,
    ShutdownHalt,
    ShutdownReboot
};

class CKernel
{
public:
    CKernel (void);
    ~CKernel (void);

    boolean Initialize (void);

    TShutdownMode Run (void);

    void WriteSerial(const char *s) {
        m_Serial.Write(s, strlen(s));
    }

private:
    CActLED m_ActLED;
    CSerialDevice m_Serial;
};

#endif
