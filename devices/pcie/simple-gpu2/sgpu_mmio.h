#ifndef SGPU_MMIO_H
#define SGPU_MMIO_H

#include "sgpu_qemu.h"

typedef SimpleGPU SimpleGPU;

void gpu_mmio_init(SimpleGPU *dev, Error **errp);

#endif