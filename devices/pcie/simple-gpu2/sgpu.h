#ifndef SIMPLE_GPU_H
#define SIMPLE_GPU_H

#include "sgpu_spec.h"
#include "sgpu_qemu.h"
#include "sgpu_msix.h"
#include "sgpu_mmio.h"
#include "sgpu_dma.h"

#define GPU_LOG(...) qemu_log(__VA_ARGS__)

#define TYPE_SIMPLE_GPU "sgpu"
#define SIMPLE_GPU_DESC "a simple GPU device"

OBJECT_DECLARE_TYPE(SimpleGPU, SimpleGPUClass, SIMPLE_GPU);

typedef struct GpuRegs {
} GpuRegs;

typedef struct SimpleGPUClass {
  /*
   * OOP hack : Our parent class (PCIDevice) is part of the strutct
   * The idea is to be able to access it from our own class
   */
  PCIDeviceClass parent_class;
} SimpleGPUClass;

typedef struct SimpleGPU {
  /*< private >*/
  /* OOP hack : Our parent (PCIDevice) is part of the struct */
  PCIDevice pci_dev;
  /*< public >*/
  MemoryRegion mmio;

  MSIXBar msix;

  DMABar dma;

  GpuRegs regs;

} SimpleGPU;

#endif /* SIMPLE_GPU_H */