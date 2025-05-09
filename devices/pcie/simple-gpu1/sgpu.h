#ifndef SIMPLE_GPU_H
#define SIMPLE_GPU_H

#include "spec.h"

#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_device.h"
#include "qemu/log.h"

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
  GpuRegs regs;

} SimpleGPU;

#endif /* SIMPLE_GPU_H */