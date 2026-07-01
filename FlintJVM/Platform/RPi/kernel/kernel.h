
#ifndef _kernel_h
#define _kernel_h

#include <circle/actled.h>
#include <circle/types.h>

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

private:
    CActLED m_ActLED;
};

#endif
