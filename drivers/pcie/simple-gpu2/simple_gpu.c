#include <linux/cdev.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/msi.h>

#include "sgpu_spec.h"

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("a simple pcie gpu driver");
MODULE_AUTHOR("lipracer <lipracer@gmail.com>");

struct sgpu_bar {
  u64 start;
  u64 end;
  u64 len;
  void __iomem *mmio;
};

struct sgpu_dev {
  struct pci_dev *pdev;
  struct sgpu_bar mmio_bar;
  struct sgpu_bar msix_bar;
  struct sgpu_bar dma_bar;

  void* dma_buffer;
  dma_addr_t dma_handle;

  dev_t minor;
  dev_t major;
  struct cdev cdev;

  struct msix_entry *entries;
};

static struct class *sgpu_class;

static const char *const kDeviceName = "sgpu";

static struct pci_device_id sgpu_id_tbl[] = {
    {PCI_DEVICE(SGPU_VENDOR_ID, SGPU_DEVICE_ID)},
    {},
};

MODULE_DEVICE_TABLE(pci, sgpu_id_tbl);

static int sgpu_open(struct inode *inode, struct file *fp) {
  printk(KERN_WARNING "sgpu_open inode=%p\n", inode);
  struct sgpu_dev *sdev = container_of(inode->i_cdev, struct sgpu_dev, cdev);
  dev_info(&sdev->pdev->dev, "sgpu sgpu_open\n");
  fp->private_data = sdev;
  return 0;
}

static int sgpu_mmap(struct file *fp, struct vm_area_struct *vma) {
  struct sgpu_dev *sdev = fp->private_data;
  dev_info(&sdev->pdev->dev, "sgpu sgpu_mmap\n");
  return 0;
}

static long sgpu_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
  struct sgpu_dev *sdev = fp->private_data;
  return 0;
}

static long sgpu_read(struct file *fp, char __user *buffer, size_t len,
                      loff_t *offset) {
  struct sgpu_dev *sdev = fp->private_data;
  return 0;
}

static long sgpu_write(struct file *fp, const char __user *buffer, size_t len,
                       loff_t *offset) {
  struct sgpu_dev *sdev = fp->private_data;
  dev_info(&sdev->pdev->dev, "sgpu sgpu_write\n");
  copy_from_user(sdev->dma_buffer, buffer, len);
  writeq(len, sdev->dma_bar.mmio + DMA_SRC_LEN_REG);
  writeq(1, sdev->dma_bar.mmio + DMA_STATUS_REG);
  return len;
}

static const struct file_operations sgpu_fops = {
    .owner = THIS_MODULE,
    .open = sgpu_open,
    .mmap = sgpu_mmap,
    .read = sgpu_read,
    .write = sgpu_write,
    .unlocked_ioctl = sgpu_ioctl,
};

static int sgpu_init_bar(struct pci_dev *pdev, struct sgpu_bar *bar,
                         int index) {
  bar->start = pci_resource_start(pdev, index);
  bar->end = pci_resource_end(pdev, index);
  bar->len = pci_resource_len(pdev, index);

  dev_err(&(pdev->dev),
          "bar %d start address start:0x%lx end:0x%lx len:0x%lx\n", index,
          bar->start, bar->end, bar->len);

  bar->mmio = pci_iomap(pdev, index, bar->len);
  if (!bar->mmio) {
    dev_err(&(pdev->dev), "cannot map BAR %u\n", index);
    return -ENOMEM;
  }

  return 0;
}

static int sgpu_dev_init(struct sgpu_dev *sdev, struct pci_dev *pdev) {
  int err;
  sdev->pdev = pdev;
  dev_info(&pdev->dev, "sgpu_dev_init\n");
  err = pci_request_regions(pdev, kDeviceName);

  if (err)
  {
    dev_err(&pdev->dev, "pci_request_regions err=%d\n", err);
    return err;
  }

  pci_set_dma_mask(pdev, DMA_BIT_MASK(32));
  pci_set_master(pdev);

  sgpu_init_bar(pdev, &sdev->msix_bar, SGPU_MSIX_BAR_INDEX);
  sgpu_init_bar(pdev, &sdev->mmio_bar, SGPU_MMIO_BAR_INDEX);
  sgpu_init_bar(pdev, &sdev->dma_bar, SGPU_DMA_BAR_INDEX);

  sdev->dma_buffer =
      dma_alloc_coherent(&pdev->dev, 1024, &sdev->dma_handle, GFP_KERNEL);
  dev_info(&pdev->dev, "dma_alloc_coherent sdev->dma_buffer=%p\n",
           sdev->dma_buffer);

  writeq(sdev->dma_handle, sdev->dma_bar.mmio + DMA_SRC_ADDR_REG);

  pci_set_drvdata(pdev, sdev);
  return 0;
}

static struct sgpu_dev *sgpu_alloc_dev(void) {
  return kmalloc(sizeof(struct sgpu_dev), GFP_KERNEL);
}

static irqreturn_t my_irq_handler(int irq, void *dev_id) {
  struct sgpu_dev *sgpu_dev = (struct sgpu_dev *)(dev_id);
  dev_info(&(sgpu_dev->pdev->dev), "my_irq_handler irq=%d \n", irq);
  // struct my_dev *dev = dev_id;
  // if (!irq_for_my_dev(dev))
  //     return IRQ_NONE;
  // clear_hw_interrupt(dev);
  // wake_up_process(dev->task);
  return IRQ_HANDLED;
}

void fill_msix_entry(struct pci_dev *pdev, void __iomem *bar2_virt, int i,
                     uint64_t msg_addr, uint32_t msg_data) {
  void __iomem *entry = bar2_virt + i * MSIX_ENTRY_SIZE;

  writel((uint32_t)(msg_addr & 0xFFFFFFFF), entry + MSG_ADDR_OFFSET);

  writel((uint32_t)(msg_addr >> 32), entry + MSG_UPPER_OFFSET);

  writel(msg_data, entry + MSG_DATA_OFFSET);

  writel(0, entry + MSG_CTL_OFFSET);
}

static void init_irq_entries(struct sgpu_dev *sdev) {
  int err, i, num_entries;

  u16 control;
  u32 table_offset;
  int msix_cap;
  int bar;

  dev_info(&sdev->pdev->dev, "init_irq_entries\n");
  msix_cap = pci_find_capability(sdev->pdev, PCI_CAP_ID_MSIX);
  if (!msix_cap) {
    dev_err(&sdev->pdev->dev, "MSI-X capability not found\n");
    return -ENODEV;
  }

  pci_read_config_word(sdev->pdev, msix_cap + PCI_MSIX_FLAGS, &control);
  pci_read_config_dword(sdev->pdev, msix_cap + PCI_MSIX_TABLE, &table_offset);

  bar = table_offset & PCI_MSIX_TABLE_BIR;
  dev_info(&sdev->pdev->dev, "MSI-X uses BAR %d\n", bar);

  num_entries = SGPU_MSIX_VECTOR_COUNT;
  num_entries = pci_alloc_irq_vectors(sdev->pdev, bar, SGPU_MSIX_VECTOR_COUNT,
                                      PCI_IRQ_MSIX);
  dev_info(&(sdev->pdev->dev), "sgpu pci_alloc_irq_vectors irq count=%d \n",
           num_entries);
  if (num_entries <= 0) {
    dev_err(&(sdev->pdev->dev), "sgpu pci_alloc_irq_vectors - error=%d\n",
            num_entries);
  }

  sdev->entries = kcalloc(num_entries, sizeof(*sdev->entries), GFP_KERNEL);
  for (i = 0; i < num_entries; i++) {
    sdev->entries[i].entry = i;
    // sdev->entries[i].vector = 0;
  }

  // num_entries =
  //     pci_enable_msix_range(sdev->pdev, sdev->entries, 2, num_entries);
  // err = pci_enable_msix_exact(sdev->pdev, sdev->entries, num_entries);
  // if (err) {
  //   dev_err(&(sdev->pdev->dev), "sgpu pci_enable_msix_exact - error=%d\n", err);
  // }
  // dev_info(&(sdev->pdev->dev), "sgpu pci_enable_msix_range - ret=%d\n",
  //          num_entries);

  for (i = 0; i < num_entries; i++) {
    sdev->entries[i].vector = pci_irq_vector(sdev->pdev, i);

    // dev_info(&(sdev->pdev->dev), "sgpu pci_irq_vector vector=%d \n",
    //          sdev->entries[i].vector);

    err = request_irq(sdev->entries[i].vector, my_irq_handler,
                      IRQF_SHARED | IRQF_TRIGGER_HIGH,
                      dev_name(&sdev->pdev->dev), sdev);
    if (err) {
      dev_err(&(sdev->pdev->dev), "sgpu request_irq - error=%d\n", err);
    }
    // fill_msix_entry(sdev->pdev, sdev->msix_bar.mmio, i, 0xFEE00000, i);
  }
  pci_set_drvdata(sdev->pdev, sdev->entries);
}

static int sgpu_probe(struct pci_dev *pdev, const struct pci_device_id *id) {
  int err;
  int mem_bars;
  struct sgpu_dev *sdev;
  dev_t dev_num;
  struct device *dev;

  dev_info(&pdev->dev, "enrty sgpu_probe");

  alloc_chrdev_region(&dev_num, 0, 1, kDeviceName);

  sdev = sgpu_alloc_dev();
  if (sdev == NULL) {
    err = -ENOMEM;
    dev_err(&pdev->dev, "sgpu_alloc_device failed\n");
    goto err_sgpu_alloc;
  }

  dev_info(&pdev->dev, "pci_enable_device");
  err = pci_enable_device(pdev);
  dev_info(&pdev->dev, "pci_enable_device after");
  if (err) {
    dev_err(&pdev->dev, "sgpu_enable_device failed\n");
    goto err_sgpu_enable;
  }

  sdev->minor = MINOR(dev_num);
  sdev->major = MAJOR(dev_num);

  sgpu_dev_init(sdev, pdev);

  cdev_init(&sdev->cdev, &sgpu_fops);

  init_irq_entries(sdev);

  dev_info(&pdev->dev, "sgpu probe - success\n");

  iowrite32(0xff, sdev->mmio_bar.mmio);

  // device_create(sgpu_class, &pdev->dev, dev_num, NULL, kDeviceName);

  err = cdev_add(&sdev->cdev, MKDEV(sdev->major, sdev->minor), 4);
  if (err) {
    dev_err(&(pdev->dev), "cdev_add failed\n");
  }

  /* create /dev/ node via udev */
  dev = device_create(sgpu_class, &pdev->dev, MKDEV(sdev->major, sdev->minor),
                      sdev, kDeviceName);

  return 0;

err_req_region:
err_sgpu_enable:
err_sgpu_alloc:
  dev_err(&(pdev->dev), "sgpu_probe failed with error=%d\n", err);
  return err;
}

static void sgpu_remove(struct pci_dev *pdev) {
  struct sgpu_dev *sgpu_dev = pci_get_drvdata(pdev);
  device_destroy(sgpu_class, MKDEV(sgpu_dev->major, sgpu_dev->minor));
  kfree(sgpu_dev);
  dev_info(&(pdev->dev), "sgpu remove - success\n");
}

static struct pci_driver sgpu_pci_driver = {
    .name = kDeviceName,
    .id_table = sgpu_id_tbl,
    .probe = sgpu_probe,
    .remove = sgpu_remove,
};

static void sgpu_module_exit(void) {
  pci_unregister_driver(&sgpu_pci_driver);
  class_destroy(sgpu_class);
  pr_debug("sgpu_module_exit finished successfully\n");
}

static char *sgpu_devnode(struct device *dev, umode_t *mode) {
  if (mode)
    *mode = 0666;
  return kasprintf(GFP_KERNEL, "sgpu/%s", dev_name(dev));
}

static int __init sgpu_module_init(void) {
  int err;

  sgpu_class = class_create(THIS_MODULE, kDeviceName);
  if (IS_ERR(sgpu_class)) {
    pr_err("class_create error\n");
    err = PTR_ERR(sgpu_class);
    return err;
  }

  sgpu_class->devnode = sgpu_devnode;
  err = pci_register_driver(&sgpu_pci_driver);
  if (err) {
    pr_err("pci_register_driver error\n");
    goto err_pci;
  }

  pr_debug("sgpu_module_init finished successfully\n");
  return 0;
err_pci:
  class_destroy(sgpu_class);
  pr_err("sgpu_module_init failed with err=%d\n", err);
  return err;
}

module_init(sgpu_module_init);
module_exit(sgpu_module_exit);

MODULE_VERSION("4.20.0+ mod_unload ");
