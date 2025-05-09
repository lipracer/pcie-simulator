#ifndef SGPU_MSIX_H
#define SGPU_MSIX_H

#include "sgpu_qemu.h"

typedef struct MSIXBar {
  MemoryRegion msix_bar;
} MSIXBar;

typedef struct SimpleGPU SimpleGPU;

void gpu_msix_init(SimpleGPU *dev, Error **errp);

void launch_kernel(SimpleGPU *dev);

#endif
