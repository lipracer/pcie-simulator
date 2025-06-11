#ifndef SIMPLE_GPU_H
#define SIMPLE_GPU_H

#include <linux/cdev.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/msi.h>
#include <linux/pci.h>

struct sgpu_bar {
  u64 start;
  u64 end;
  u64 len;
  void __iomem *mmio;
};

typedef struct sgpu_dev {
  struct pci_dev *pdev;
  struct sgpu_bar mmio_bar;
  struct sgpu_bar msix_bar;
  struct sgpu_bar dma_bar;

  void *dma_buffer;
  dma_addr_t dma_handle;

  dev_t minor;
  dev_t major;
  struct cdev cdev;

  struct msix_entry *entries;
} sgpu_dev;

typedef struct sgpu_job {
  int irq_entry;
} sgpu_job;

#endif
