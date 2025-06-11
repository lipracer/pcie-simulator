#include "sgpu.h"
#include "sgpu_mmio.h"

#include "hw/hw.h"
#include "hw/pci/msi.h"
#include "hw/pci/pci.h"
#include "qapi/visitor.h"
#include "qemu/log.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qemu/module.h"
#include "qemu/osdep.h"
#include "qemu/timer.h"
#include "qemu/units.h"
#include "qom/object.h"

typedef struct SimpleGPU SimpleGPU;
DECLARE_INSTANCE_CHECKER(SimpleGPU, GPU, TYPE_SIMPLE_GPU)

static void pci_gpu_register_types(void);
static void gpu_class_init(ObjectClass *class, void *data);
static void pci_gpu_realize(PCIDevice *pdev, Error **errp);
static void pci_gpu_uninit(PCIDevice *pdev);

type_init(pci_gpu_register_types)

    static const TypeInfo gpu_info = {
        .name = TYPE_SIMPLE_GPU,
        .parent = TYPE_PCI_DEVICE,
        .instance_size = sizeof(SimpleGPU),
        .class_init = gpu_class_init,
        .interfaces =
            (InterfaceInfo[]){
                {INTERFACE_PCIE_DEVICE},
                {},
            },
};

static void pci_gpu_register_types(void) { type_register_static(&gpu_info); }

static void gpu_class_init(ObjectClass *class, void *data) {
  GPU_LOG("Class init\n");
  DeviceClass *dc = DEVICE_CLASS(class);
  PCIDeviceClass *k = SIMPLE_GPU_CLASS(class);

  k->realize = pci_gpu_realize;
  k->exit = pci_gpu_uninit;
  k->vendor_id = SGPU_VENDOR_ID;
  k->device_id = SGPU_DEVICE_ID;
  k->revision = SGPU_REVISION;
  k->class_id = PCI_CLASS_OTHERS;

  set_bit(DEVICE_CATEGORY_MISC, dc->categories);
  dc->desc = SIMPLE_GPU_DESC;
}

static void pci_gpu_realize(PCIDevice *pdev, Error **errp) {
  GPU_LOG("GPU Realize\n");
  SimpleGPU *gpu = SIMPLE_GPU(pdev);
  uint8_t *conf = pdev->config;
  
  // pci_config_set_interrupt_pin(conf, 1);
  pcie_endpoint_cap_init(pdev, 0x80);

  gpu_msix_init(gpu, errp);
  gpu_mmio_init(gpu, errp);
  gpu_dma_init(gpu, errp);
}

static void pci_gpu_uninit(PCIDevice *pdev) { GPU_LOG("GPU un-init\n"); }
