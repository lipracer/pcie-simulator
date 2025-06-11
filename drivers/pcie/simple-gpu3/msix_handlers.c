#include "msix_handlers.h"
#include "simple_gpu_module.h"

irqreturn_t on_job_complete(int irq, void *dev_id) {
  struct sgpu_dev *sgpu_dev = (struct sgpu_dev *)(dev_id);
  dev_info(&(sgpu_dev->pdev->dev), "%s irq=%d \n", __func__, irq);
  return IRQ_HANDLED;
}

irqreturn_t on_pagefault(int irq, void *dev_id) {
  struct sgpu_dev *sgpu_dev = (struct sgpu_dev *)(dev_id);
  dev_info(&(sgpu_dev->pdev->dev), "%s irq=%d \n", __func__, irq);
  return IRQ_HANDLED;
}

irqreturn_t on_dma_read_complete(int irq, void *dev_id) {
  struct sgpu_dev *sgpu_dev = (struct sgpu_dev *)(dev_id);
  dev_info(&(sgpu_dev->pdev->dev), "%s irq=%d \n", __func__, irq);
  return IRQ_HANDLED;
}

irqreturn_t on_dma_write_complete(int irq, void *dev_id) {
  struct sgpu_dev *sgpu_dev = (struct sgpu_dev *)(dev_id);
  dev_info(&(sgpu_dev->pdev->dev), "%s irq=%d \n", __func__, irq);
  return IRQ_HANDLED;
}
