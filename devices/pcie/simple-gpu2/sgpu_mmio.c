#include "sgpu.h"

static const uint64_t kMMIO_SIZE = 1024ULL;

static uint64_t sgpu_mmio_read(void *opaque, hwaddr addr, unsigned int size) {
  SimpleGPU *dev = opaque;
  uint64_t val = ~0ULL;
  GPU_LOG("sgpu_mmio_read addr:%p size:%d\n", addr, size);

  return val;
}

static void sgpu_mmio_write(void *opaque, hwaddr addr, uint64_t value,
                            unsigned int size) {
  SimpleGPU *dev = opaque;
  uint64_t val = ~0ULL;
  GPU_LOG("sgpu_mmio_write addr:%p value:%lx size:%d\n", addr, value, size);

  if (value == 0xFF) {
    launch_kernel(dev);
  }
}

const MemoryRegionOps sgpu_mmio_ops = {
    .read = sgpu_mmio_read,
    .write = sgpu_mmio_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid =
        {
            .min_access_size = 4,
            .max_access_size = 8,
        },
    .impl =
        {
            .min_access_size = 4,
            .max_access_size = 8,
        },
};

void gpu_mmio_init(SimpleGPU *dev, Error **errp) {
  GPU_LOG("gpu_mmio_init\n");

  memory_region_init_io(&dev->mmio, OBJECT(dev), &sgpu_mmio_ops, dev,
                        "sgpu-mmio", kMMIO_SIZE);
  pci_register_bar(&dev->pci_dev, SGPU_MMIO_BAR_INDEX,
                   PCI_BASE_ADDRESS_SPACE_MEMORY, &dev->mmio);
}
