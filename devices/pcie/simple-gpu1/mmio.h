#ifndef MMIO_H
#define MMIO_H

#include "sgpu.h"

typedef SimpleGPU SimpleGPU;

void gpu_mmio_init(SimpleGPU *dev, Error **errp);

#endif