#ifndef SGPU_DMA_H
#define SGPU_DMA_H

#include "sgpu_qemu.h"

typedef SimpleGPU SimpleGPU;

typedef enum {
  kNone,
  kReadReady,
} DmaStatus;

typedef struct DMABar {
  AddressSpace* as;
  MemoryRegion dma_bar;
  uint64_t raddr;
  uint64_t src_len;
  uint64_t waddr;
  uint64_t dst_len;
  DmaStatus status;
} DMABar;

void gpu_dma_init(SimpleGPU *dev, Error **errp);

#endif