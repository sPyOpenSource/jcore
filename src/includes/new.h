#ifndef __MYOS__NEW_H
#define __MYOS__NEW_H

#include <stddef.h>

inline void* operator new(size_t, void* ptr) noexcept {
    return ptr;
}

#endif
