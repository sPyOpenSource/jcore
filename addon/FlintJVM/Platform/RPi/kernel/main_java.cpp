
#include "flint.h"

extern "C" {
    bool Flint_runToMain(const char *cls) {
        return Flint::runToMain(cls);
    }
}
