#ifndef SGPU_SPEC_H
#define SGPU_SPEC_H

#define SGPU_VENDOR_ID 0x1b36
#define SGPU_DEVICE_ID 0x1100
#define SGPU_REVISION 0x01

#define SGPU_MSIX_VECTOR_COUNT 4
#define SGPU_MSIX_BAR_INDEX 4
#define SGPU_MMIO_BAR_INDEX 0
#define SGPU_DMA_BAR_INDEX 1

#define MSIX_ENTRY_SIZE PCI_MSIX_ENTRY_SIZE

#define SGPU_MSIX_T_BAR_OFFSET 0
#define SGPU_MSIX_P_BAR_OFFSET                                                 \
  (SGPU_MSIX_T_BAR_OFFSET + SGPU_MSIX_VECTOR_COUNT * PCI_MSIX_ENTRY_SIZE)

#define MSG_ADDR_OFFSET 0x0
#define MSG_UPPER_OFFSET 0x4
#define MSG_DATA_OFFSET 0x8
#define MSG_CTL_OFFSET 0xC

#define DMA_SRC_ADDR_REG 0
#define DMA_SRC_LEN_REG (DMA_SRC_ADDR_REG + 8)
#define DMA_DST_ADDR_REG (DMA_SRC_LEN_REG + 8)
#define DMA_DST_LEN_REG (DMA_DST_ADDR_REG + 8)
#define DMA_STATUS_REG (DMA_DST_LEN_REG + 8)

#endif