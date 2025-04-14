#include <linux/init.h>
#include <linux/module.h>
#include "my_chr_dev.h"

static int major = 0; // Major number, 0 means dynamic allocation
static int minor = 0; // Minor number, 0 means dynamic allocation

const char *dyn_dev_class_name = "dyn_chr_dev_class";
const char *dyn_dev_device_name = "dyn_chr_dev_device";
static struct my_char_device_info mydev_info;

static struct file_operations dyn_chr_fops = {
    .owner = THIS_MODULE,
};

static struct task_struct *worker_thread;

/* Entry */
static int __init dyn_chr_dev_init(void) {
  printk(KERN_INFO "dyn_chr_dev_init\n");
  int ret;

  pr_info("dyn_chr_dev_init\n");
  ret = register_my_char_device(&mydev_info, dyn_dev_device_name,
                                dyn_dev_class_name, 0, 0, &dyn_chr_fops);
  if (ret < 0) {
    pr_err("Failed to register char device\n");
    return ret;
  }

  pr_info("dyn_chr_dev_init done\n");
  return 0;
}

/* Exit */
static void __exit dyn_chr_dev_exit(void) {
  unregister_my_char_device(&mydev_info);
  printk(KERN_INFO "dyn_chr_dev_exit ok\n");
}

module_init(dyn_chr_dev_init);
module_exit(dyn_chr_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");