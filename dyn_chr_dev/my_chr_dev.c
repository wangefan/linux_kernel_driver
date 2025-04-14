#include "my_chr_dev.h"
#include <linux/module.h>
#include <linux/slab.h>

int register_my_char_device(struct my_char_device_info *dev_info,
                            const char *dev_name, const char *class_name,
                            int major, int minor,
                            const struct file_operations *fops) {
  int ret;

  // handle dev number
  if (major > 0) {
    dev_info->dev_num = MKDEV(major, minor);
    ret = register_chrdev_region(dev_info->dev_num, 1, dev_name);
  } else {
    ret = alloc_chrdev_region(&dev_info->dev_num, minor, 1, dev_name);
  }
  if (ret < 0) {
    pr_err("Failed to allocate device number\n");
    return ret;
  }

  // handle cdev
  cdev_init(&dev_info->cdev, fops);
  dev_info->cdev.owner = THIS_MODULE;

  ret = cdev_add(&dev_info->cdev, dev_info->dev_num, 1);
  if (ret) {
    pr_err("Failed to add cdev\n");
    goto unregister_chrdev;
  }

  // handle class
  dev_info->class = class_create(THIS_MODULE, class_name);
  if (IS_ERR(dev_info->class)) {
    pr_err("Failed to create class\n");
    ret = PTR_ERR(dev_info->class);
    goto del_cdev;
  }

  // handle device node
  dev_info->device =
      device_create(dev_info->class, NULL, dev_info->dev_num, NULL, dev_name);
  if (IS_ERR(dev_info->device)) {
    pr_err("Failed to create device\n");
    ret = PTR_ERR(dev_info->device);
    goto destroy_class;
  }

  return 0;

destroy_class:
  class_destroy(dev_info->class);
del_cdev:
  cdev_del(&dev_info->cdev);
unregister_chrdev:
  unregister_chrdev_region(dev_info->dev_num, 1);
  return ret;
}

void unregister_my_char_device(struct my_char_device_info *dev_info) {
  device_destroy(dev_info->class, dev_info->dev_num);
  class_destroy(dev_info->class);
  cdev_del(&dev_info->cdev);
  unregister_chrdev_region(dev_info->dev_num, 1);
}

MODULE_LICENSE("GPL");
EXPORT_SYMBOL(register_my_char_device);
EXPORT_SYMBOL(unregister_my_char_device);