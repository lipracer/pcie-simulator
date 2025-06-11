#include "sgpu.h"

static uint64_t sgpu_dma_read(void *opaque, hwaddr addr, unsigned int size) {
  SimpleGPU *dev = opaque;
  uint64_t val = ~0ULL;
  GPU_LOG("sgpu_dma_read addr:%p size:%d\n", addr, size);

  return val;
}

static void sgpu_dma_write(void *opaque, hwaddr addr, uint64_t value,
                           unsigned int size) {
  SimpleGPU *dev = opaque;
  uint64_t val = ~0ULL;
  GPU_LOG("sgpu_dma_write addr:%p value:%lx size:%d\n", addr, value, size);

  if (addr == DMA_SRC_ADDR_REG) {
    dev->dma.raddr = value;
  } else if (addr == DMA_DST_ADDR_REG) {
    dev->dma.waddr = value;
  } else if (addr == DMA_SRC_LEN_REG) {
    dev->dma.src_len = value;
  } else if (addr == DMA_DST_LEN_REG) {
    dev->dma.dst_len = value;
  } else if (addr == DMA_STATUS_REG) {
    dev->dma.status = value;
    if (dev->dma.status == kReadReady) {
      char buffer[20] = {0};
      dma_memory_read(dev->dma.as, dev->dma.raddr, buffer, dev->dma.src_len,
                      MEMTXATTRS_UNSPECIFIED);
      GPU_LOG("dma read msg:%s", buffer);
    }
  }
}

const MemoryRegionOps sgpu_dma_ops = {
    .read = sgpu_dma_read,
    .write = sgpu_dma_write,
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

void gpu_dma_init(SimpleGPU *dev, Error **errp) {
  GPU_LOG("gpu_dma_init\n");
  dev->dma.as = pci_get_address_space(&dev->pci_dev);
  memory_region_init_io(&dev->dma.dma_bar, OBJECT(dev), &sgpu_dma_ops, dev,
                        "sgpu-dma", 1024);
  pci_register_bar(&dev->pci_dev, SGPU_DMA_BAR_INDEX,
                   PCI_BASE_ADDRESS_SPACE_MEMORY, &dev->dma.dma_bar);
}
