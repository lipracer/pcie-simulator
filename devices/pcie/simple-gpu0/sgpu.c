#include "sgpu.h"

#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/units.h"
#include "hw/pci/pci.h"
#include "hw/hw.h"
#include "hw/pci/msi.h"
#include "qemu/timer.h"
#include "qom/object.h"
#include "qemu/main-loop.h" /* iothread mutex */
#include "qemu/module.h"
#include "qapi/visitor.h"

#define SGPU_VENDOR_ID 0x1b36
#define SGPU_DEVICE_ID 0x1100
#define SGPU_REVISION 0x01

typedef struct SimpleGPU SimpleGPU;
DECLARE_INSTANCE_CHECKER(SimpleGPU, GPU, TYPE_SIMPLE_GPU)

static void pci_gpu_register_types(void);
static void gpu_instance_init(Object *obj);
static void gpu_class_init(ObjectClass *class, void *data);
static void pci_gpu_realize(PCIDevice *pdev, Error **errp);
static void pci_gpu_uninit(PCIDevice *pdev);

type_init(pci_gpu_register_types)

static const TypeInfo gpu_info = {
    .name = TYPE_SIMPLE_GPU,
    .parent = TYPE_PCI_DEVICE,
    .instance_size = sizeof(SimpleGPU),
    // .instance_init = gpu_instance_init,
    .class_init = gpu_class_init,
    .interfaces =
        (InterfaceInfo[]){
            {INTERFACE_PCIE_DEVICE},
            {},
        },
};

static void pci_gpu_register_types(void)
{
    type_register_static(&gpu_info);
}

static void gpu_instance_init(Object *obj)
{
	printf("GPU instance init\n");
}

static void gpu_class_init(ObjectClass *class, void *data)
{
	printf("Class init\n");
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

static void pci_gpu_realize(PCIDevice *pdev, Error **errp)
{
	printf("GPU Realize\n");
    //GpuState *gpu = GPU(pdev);
    //uint8_t *pci_conf = pdev->config;
}

static void pci_gpu_uninit(PCIDevice *pdev)
{
	printf("GPU un-init\n");
}
