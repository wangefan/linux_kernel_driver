#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/kernel.h>

#define CHARDEV_NAME "my-led-chardev"
#define CLASS_NAME "my-led-class"
#define DEV_NAME "my-led-device"

static int major;
static struct class *my_led_class;

static int my_led_open(struct inode *inode, struct file *file) {
  printk(KERN_INFO "my_led_open\n");
  return 0;
}

static int my_led_release(struct inode *inode, struct file *file) {
  printk(KERN_INFO "my_led_release\n");
  return 0;
}

static ssize_t my_led_read(struct file *file, char __user *buf, size_t count,
                           loff_t *ppos) {
  printk(KERN_INFO "my_led_read\n");
  return 0;
}

static ssize_t my_led_write(struct file *file, const char __user *buf,
                            size_t count, loff_t *ppos) {
  printk(KERN_INFO "my_led_write\n");
  return count;
}

static struct file_operations my_led_fops = {
    .owner = THIS_MODULE,
    .open = my_led_open,
    .release = my_led_release,
    .read = my_led_read,
    .write = my_led_write,
};

static int __init my_led_ini(void) {
  int ret = -1;

  printk(KERN_INFO "Initializing...\n");

  major = register_chrdev(0, CHARDEV_NAME, &my_led_fops);
  if (major < 0) {
    printk(KERN_ERR "Cannot register char device\n");
    return major;
  }

  my_led_class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(my_led_class)) {
    printk(KERN_ERR "Cannot create the struct class\n");
    ret = PTR_ERR(my_led_class);
    goto unregister_chrdev;
  }

  if (IS_ERR(
          device_create(my_led_class, NULL, MKDEV(major, 0), NULL, DEV_NAME))) {
    printk(KERN_ERR "Cannot create the device\n");
    ret = -EINVAL;
    goto class_destroy;
  }

  ret = 0;
  printk(KERN_INFO "my_led_init done\n");
  return ret;

class_destroy:
  class_destroy(my_led_class);
unregister_chrdev:
  unregister_chrdev(major, CHARDEV_NAME);
  printk(KERN_ERR "my_led_ini failed\n");
  return ret;
}

static void __exit my_led_exit(void)
{
    printk(KERN_INFO "my_led: Exiting...\n");
    device_destroy(my_led_class, MKDEV(major, 0));
    class_destroy(my_led_class);
    unregister_chrdev(major, CHARDEV_NAME);
    printk(KERN_INFO "my_led: Exit done\n");
}

module_init(my_led_ini);
module_exit(my_led_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");
MODULE_DESCRIPTION("My LED Driver");
