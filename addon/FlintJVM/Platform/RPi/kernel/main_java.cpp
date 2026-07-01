
#include "flint.h"

extern "C" {
    void rpi_uart_puts(const char *s);
}

extern "C" {
    bool Flint_runToMain(const char *cls) {
        rpi_uart_puts("  Flint::runToMain called\r\n");
        rpi_uart_puts("  newExecution...\r\n");
        FExec *exec = Flint::newExecution(NULL);
        if(exec == NULL) {
            rpi_uart_puts("  newExecution FAILED\r\n");
            return false;
        }
        rpi_uart_puts("  findClass...\r\n");
        JClass *mainCls = Flint::findClass(NULL, cls);
        if(mainCls == NULL) {
            rpi_uart_puts("  findClass FAILED\r\n");
            return false;
        }
        rpi_uart_puts("  getMainMethodInfo...\r\n");
        MethodInfo *method = mainCls->getClassLoader()->getMainMethodInfo(NULL);
        if(method == NULL) {
            rpi_uart_puts("  getMainMethodInfo FAILED\r\n");
            return false;
        }
        rpi_uart_puts("  exec->run...\r\n");
        return exec->run(method, 1, NULL);
    }
}
