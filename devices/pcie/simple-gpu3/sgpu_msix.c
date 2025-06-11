// clang-format off
#include "sgpu.h"
#include "hw/pci/msi.h"
// clang-format on

static const uint64_t kMMIO_SIZE = 16 * SGPU_MSIX_VECTOR_COUNT * 2;
// SGPU_MSIX_VECTOR_COUNT * PCI_MSIX_ENTRY_SIZE * 2;

struct MSIXTableEntry {
  uint64_t msg_addr;
  uint32_t msg_data;
  uint32_t vector_ctl;
};

static uint64_t sgpu_mxix_mmio_read(void *opaque, hwaddr addr,
                                    unsigned int size) {
  SimpleGPU *dev = opaque;
  uint64_t val = ~0ULL;
  GPU_LOG("sgpu_mxix_mmio_read addr:%p size:%d\n", addr, size);

  return val;
}

static void sgpu_msix_mmio_write(void *opaque, hwaddr addr, uint64_t value,
                                 unsigned int size) {
  SimpleGPU *sdev = opaque;
  uint64_t val = ~0ULL;
  MSIMessage message;

  PCIDevice *dev = &sdev->pci_dev;
  int idx = (addr - SGPU_MSIX_T_BAR_OFFSET) / PCI_MSIX_ENTRY_SIZE;

  GPU_LOG("sgpu_msix_mmio_write addr:%p value:%lx size:%d vector:%d\n", addr,
          value, size, idx);

  // switch (addr % PCI_MSIX_ENTRY_SIZE) {
  // case MSG_ADDR_OFFSET:
  //   message.address = (message.address & ~0xFFFFFFFFULL) | value;
  //   break;
  // case MSG_UPPER_OFFSET:
  //   message.address =
  //       (message.address & 0xFFFFFFFFULL) | ((uint64_t)value << 32);
  //   break;
  // case MSG_DATA_OFFSET:
  //   message.data = value;
  //   break;
  // case MSG_CTL_OFFSET:
  //   // dev->msix_table[idx].vector_control = data;
  //   break;
  // }
  // msix_set_message(dev, idx, message);
}

const MemoryRegionOps sgpu_msix_mmio_ops = {
    .read = sgpu_mxix_mmio_read,
    .write = sgpu_msix_mmio_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
    .impl =
        {
            .min_access_size = 2,
            .max_access_size = 8,
        },
};

void gpu_msix_mmio_init(SimpleGPU *dev, Error **errp) {
  GPU_LOG("gpu_msix_mmio_init\n");

  GPU_LOG("memory_region_init_io\n");
  memory_region_init_io(&dev->msix.msix_bar, OBJECT(dev), &sgpu_msix_mmio_ops,
                        dev, "sgpu-msix", kMMIO_SIZE);

  GPU_LOG("pci_register_bar\n");
  pci_register_bar(&dev->pci_dev, SGPU_MSIX_BAR_INDEX,
                   PCI_BASE_ADDRESS_SPACE_MEMORY | PCI_BASE_ADDRESS_MEM_TYPE_64,
                   &dev->msix.msix_bar);
}

void gpu_msix_init(SimpleGPU *dev, Error **errp) {
  int i;
  GPU_LOG("gpu_msix_init\n");
#if 0
  gpu_msix_mmio_init(dev, errp);

  if (msix_init(&dev->pci_dev, SGPU_MSIX_VECTOR_COUNT, &dev->msix.msix_bar,
                SGPU_MSIX_BAR_INDEX, SGPU_MSIX_T_BAR_OFFSET,
                &dev->msix.msix_bar, SGPU_MSIX_BAR_INDEX,
                SGPU_MSIX_P_BAR_OFFSET, 0, errp) < 0) {
    GPU_LOG("gpu_msix_init failed!");
    return;
  }

  // if (msix_init(&dev->pci_dev, SGPU_MSIX_VECTOR_COUNT, &dev->msix.msix_bar,
  //               SGPU_MSIX_BAR_INDEX, 0x2000, &dev->msix.msix_bar,
  //               SGPU_MSIX_BAR_INDEX, 0x3800, 0x68, errp) < 0) {
  //   GPU_LOG("gpu_msix_init failed!");
  //   return;
  // }

#else
  if (msix_init_exclusive_bar(&dev->pci_dev, SGPU_MSIX_VECTOR_COUNT,
                              SGPU_MSIX_BAR_INDEX, errp) < 0) {
    GPU_LOG("msix_init_exclusive_bar failed!");
    return;
  }
  if (!msix_present(&dev->pci_dev)) {
    GPU_LOG("msix_present failed!");
    return;
  }
  // uint8_t *config;
  // config = dev->pci_dev.config + dev->pci_dev.msix_cap;
  // pci_set_word_by_mask(config + PCI_MSIX_FLAGS, PCI_MSIX_FLAGS_QSIZE,
  //                      SGPU_MSIX_VECTOR_COUNT - 1);
#endif
  PCIDevice *d = PCI_DEVICE(dev);
  for (i = 0; i < SGPU_MSIX_VECTOR_COUNT; i++) {
    msix_vector_use(d, i);
  }
}

static bool is_valid_vector(SimpleGPU *dev, int vector) {
  PCIDevice *pcie_dev = &dev->pci_dev;
  if (msix_enabled(pcie_dev)) {
    if (!msix_is_masked(pcie_dev, vector)) {
      return true;
    }
  }
  return false;
}

void launch_kernel(SimpleGPU *dev)
{
  int vector;
  uint64_t addr;
  for (vector = 0; vector < 4; ++vector)
  {
    if (!is_valid_vector(dev, 0))
    {
      GPU_LOG("invalid vector=%d\n", vector);
      continue;
    }
    GPU_LOG("launch_kernel\n");
    msix_notify(dev, vector);
  }
}
