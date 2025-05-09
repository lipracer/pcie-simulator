#include <linux/cdev.h>
#include <linux/highmem.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");
MODULE_DESCRIPTION("a simple pcie gpu driver");
MODULE_AUTHOR("lipracer <lipracer@gmail.com>");

#define SGPU_VENDOR_ID 0x1b36
#define SGPU_DEVICE_ID 0x1100

struct sgpu_bar {
  u64 start;
  u64 end;
  u64 len;
  void __iomem *mmio;
};

struct sgpu_dev {
  struct pci_dev *pdev;
  struct sgpu_bar bar;
  dev_t minor;
  dev_t major;
  struct cdev cdev;
};

static struct class *sgpu_class;

static const char *const kDeviceName = "sgpu";

static struct pci_device_id sgpu_id_tbl[] = {
    {PCI_DEVICE(SGPU_VENDOR_ID, SGPU_DEVICE_ID)},
    {},
};

MODULE_DEVICE_TABLE(pci, sgpu_id_tbl);

static int sgpu_open(struct inode *inode, struct file *fp) {
  struct pci_dev *pci_dev = (struct pci_dev *)inode->i_cdev;
  dev_info(&(pci_dev->dev), "sgpu sgpu_open\n");
  return 0;
}

static int sgpu_mmap(struct file *fp, struct vm_area_struct *vma) { return 0; }

static long sgpu_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
  return 0;
}

static const struct file_operations sgpu_fops = {
    .owner = THIS_MODULE,
    .open = sgpu_open,
    .mmap = sgpu_mmap,
    .unlocked_ioctl = sgpu_ioctl,
};

static int sgpu_dev_init(struct sgpu_dev *sdev, struct pci_dev *pdev) {
  const unsigned int bar = 0;
  sdev->pdev = pdev;

  sdev->bar.start = pci_resource_start(pdev, bar);
  sdev->bar.end = pci_resource_end(pdev, bar);
  sdev->bar.len = pci_resource_len(pdev, bar);

  dev_err(&(pdev->dev), "bar 0 start address start:0x%lx end:0x%lx len:0x%lx\n",
          sdev->bar.start, sdev->bar.end, sdev->bar.len);

  sdev->bar.mmio = pci_iomap(pdev, bar, sdev->bar.len);
  if (!sdev->bar.mmio) {
    dev_err(&(pdev->dev), "cannot map BAR %u\n", bar);
    return -ENOMEM;
  }
  pci_set_drvdata(pdev, sdev);
  iowrite32(0x12345678, sdev->bar.mmio);
  iowrite32(0x12345678, (char *)sdev->bar.mmio + 4);
  return 0;
}

static struct sgpu_dev *sgpu_alloc_dev(void) {
  return kmalloc(sizeof(struct sgpu_dev), GFP_KERNEL);
}

static int sgpu_probe(struct pci_dev *pdev, const struct pci_device_id *id) {
  int err;
  int mem_bars;
  struct sgpu_dev *sgpu_dev;
  dev_t dev_num;
  struct device *dev;

  sgpu_dev = sgpu_alloc_dev();
  if (sgpu_dev == NULL) {
    err = -ENOMEM;
    dev_err(&(pdev->dev), "sgpu_alloc_device failed\n");
    goto err_sgpu_alloc;
  }

  err = pci_enable_device(pdev);
  if (err) {
    dev_err(&(pdev->dev), "sgpu_enable_device failed\n");
    goto err_sgpu_enable;
  }

  sgpu_dev->minor = MINOR(dev_num);
  sgpu_dev->major = MAJOR(dev_num);

  sgpu_dev_init(sgpu_dev, pdev);

  cdev_init(&sgpu_dev->cdev, &sgpu_fops);

  dev_info(&(pdev->dev), "sgpu probe - success\n");

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
