#ifndef CLASSES_H
#define CLASSES_H

#include "core/load.h"

typedef void (*classexec_f) (Class * cl);

#include "core/domain.h"

void executeForallClasses(DomainDesc * domain, classexec_f func);

#endif
