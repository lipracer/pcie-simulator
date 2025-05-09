#ifndef SIMPLE_GPU_H
#define SIMPLE_GPU_H

#include "qemu/osdep.h"
#include "hw/pci/pci.h"
#include "hw/pci/pci_device.h"

#define TYPE_SIMPLE_GPU "sgpu"
#define SIMPLE_GPU_DESC "a simple GPU device"

OBJECT_DECLARE_TYPE(SimpleGPU, SimpleGPUClass, SIMPLE_GPU);

typedef struct SimpleGPUClass {
    /*
     * OOP hack : Our parent class (PCIDeviceClass) is part of the strutct
     * The idea is to be able to access it from our own class
     */
    PCIDeviceClass parent_class;
} SimpleGPUClass;

typedef struct SimpleGPU {
    /*< private >*/
    /* OOP hack : Our parent (PCIDevice) is part of the struct */
    PCIDevice pci_dev;
    /*< public >*/

} SimpleGPU;


#endif /* SIMPLE_GPU_H */