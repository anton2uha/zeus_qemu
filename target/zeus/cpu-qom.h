#pragma once
#ifndef ZEUS_CPU_QOM_H
#define ZEUS_CPU_QOM_H

#include "hw/core/cpu.h"

#define TYPE_ZEUS_CPU "zeus-cpu"

OBJECT_DECLARE_CPU_TYPE(ZeusCPU, ZeusCPUClass, ZEUS_CPU)

#define ZEUS_CPU_TYPE_SUFFIX "-" TYPE_ZEUS_CPU
#define ZEUS_CPU_TYPE_NAME(model) model ZEUS_CPU_TYPE_SUFFIX

#endif
