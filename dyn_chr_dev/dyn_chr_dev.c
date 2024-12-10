#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

dev_t dyn_chr_dev_id = 0;
static int major = 0; // Major number, 0 means dynamic allocation
static int minor = 0; // Minor number, 0 means dynamic allocation
static char *dyn_dev_name = "dyn_chr_dev";
static char *dyn_dev_class_name = "dyn_chr_dev_class";
static char *dyn_dev_device_name = "dyn_chr_dev_device";

/* For char dev */
static struct cdev dyn_chr_cdev;
static struct file_operations dyn_chr_fops = {
    .owner = THIS_MODULE,
};

/* For auto creating device without mknod */
static struct class *dyn_dev_class;

static struct task_struct *worker_thread;

/* Entry */
static int __init dyn_chr_dev_init(void) {
  printk(KERN_INFO "dyn_chr_dev_init\n");
  if (major) {
    dyn_chr_dev_id = MKDEV(major, 0); // Create a device number with minor 0
    if(register_chrdev_region(dyn_chr_dev_id, 1, dyn_dev_name) < 0) {
      printk(KERN_ERR "register_chrdev_region failed\n");
      return -1;
    }
    printk(KERN_INFO "register_chrdev_region: major=%d minor=%d\n", major, minor);
  } else {
    if(alloc_chrdev_region(&dyn_chr_dev_id, 0, 1, dyn_dev_name) < 0) {
      printk(KERN_ERR "alloc_chrdev_region failed\n");
      return -1;
    }
    major = MAJOR(dyn_chr_dev_id);
    minor = MINOR(dyn_chr_dev_id);
    printk(KERN_INFO "alloc_chrdev_region: major=%d minor=%d\n", major, minor);
  }

  // Initialize cdev
  cdev_init(&dyn_chr_cdev, &dyn_chr_fops);
  dyn_chr_cdev.owner = THIS_MODULE;
  if (cdev_add(&dyn_chr_cdev, dyn_chr_dev_id, 1) < 0) {
    printk(KERN_ERR "cdev_add failed\n");
    goto unregister_chrdev;
  }
  printk(KERN_INFO "Initialize and add cdev ok\n");

  // Creating struct class
  if (IS_ERR(dyn_dev_class = class_create(THIS_MODULE, dyn_dev_class_name))) {
    printk(KERN_ERR "Cannot create the struct class\n");
    goto cleanup_cdev;
  }

  // Creating device
  if (IS_ERR(device_create(dyn_dev_class, NULL, dyn_chr_dev_id, NULL,
                           dyn_dev_device_name))) {
    printk(KERN_ERR "Cannot create the device\n");
    goto cleanup_class;
  }

  printk(KERN_INFO "Create class and device ok\n");
  return 0;

cleanup_device:
  device_destroy(dyn_dev_class, dyn_chr_dev_id);
cleanup_class:
  class_destroy(dyn_dev_class);
cleanup_cdev:
  cdev_del(&dyn_chr_cdev);
unregister_chrdev:
  unregister_chrdev_region(dyn_chr_dev_id, 1);
  printk(KERN_ERR "dyn_chr_dev_init failed\n");
  return -1;
}

/* Exit */
static void __exit dyn_chr_dev_exit(void) {
  device_destroy(dyn_dev_class, dyn_chr_dev_id);
  class_destroy(dyn_dev_class);
  cdev_del(&dyn_chr_cdev);
  unregister_chrdev_region(dyn_chr_dev_id, 1);
  printk(KERN_INFO "dyn_chr_dev_exit ok\n");
}

module_init(dyn_chr_dev_init);
module_exit(dyn_chr_dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");