#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#define CHARDEV_NAME "my-led-chardev"
#define CLASS_NAME "my-led-class"
#define DEV_NAME "my-led-device"

static int major;
static struct class *my_led_class;

// registers
// 0x020C406C, default is enabled
// static int* CLK_ENABLE_CCGR1;

#define IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03_ADDR 0x02290014
static volatile unsigned int *iomux_to_gpio = NULL;

#define GPIO_DIR_ADDR 0x020AC004
static volatile unsigned int *gpio_dir = NULL;

#define GPIO_DATA_ADDR 0x020AC000
static volatile unsigned int *gpio_data = NULL;

static int my_led_open(struct inode *inode, struct file *file) {
  printk(KERN_INFO "my_led_open\n");
  // 1.enable clock, it is already enabled
  // 2.set pad to gpio
  *iomux_to_gpio &= ~0xf; // clear 4 bits to 0
  *iomux_to_gpio |= 5;    // set 4 bits to 0b0101, means to set pad to gpio

  // 3.set gpio direction to output
  *gpio_dir |= 1 << 3; // set bit 3 to 1, means to set gpio direction to output

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
  char kValbuf[8] = {0}; // capable to put input data (ex："1\n")
  int val = 0;
  if (count > sizeof(kValbuf) - 1)
    return -EINVAL;

  if (copy_from_user(kValbuf, buf, count)) {
    return -EFAULT;
  }
  kValbuf[count] = '\0';
  if (kstrtoint(kValbuf, 10, &val) < 0) {
    printk(KERN_ERR "Invalid input: %s\n", kValbuf);
    return -EINVAL;
  }
  printk(KERN_INFO "my_led_write: val = %d\n", val);
  if (val == 0) {
    *gpio_data |= 1 << 3; // set bit 3 to 1, means to turn off the led
  } else if (val == 1) {
    *gpio_data &= ~(1 << 3); // set bit 3 to 0, means to turn on the led
  } else {
    return -EINVAL;
  }
  return count;
}

static int __init my_led_dtree_and_pdriver_ini(void) {
  int ret = -1;

  printk(KERN_INFO "Initializing...\n");

  major = register_chrdev(0, CHARDEV_NAME, &my_led_dtree_and_pdriver_fops);
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

  // obtain the address of the registers
  iomux_to_gpio = ioremap(IOMUXC_SW_MUX_CTL_PAD_GPIO5_IO03_ADDR, 4);
  gpio_dir = ioremap(GPIO_DIR_ADDR, 4);
  gpio_data = ioremap(GPIO_DATA_ADDR, 4);

  printk(KERN_INFO "my_led_init done\n");
  return ret;

class_destroy:
  class_destroy(my_led_class);
unregister_chrdev:
  unregister_chrdev(major, CHARDEV_NAME);
  printk(KERN_ERR "my_led_ini failed\n");
  return ret;
}

static void __exit my_led_dtree_and_pdriver_exit(void) {
  printk(KERN_INFO "my_led: Exiting...\n");

  // release the registers
  iounmap(iomux_to_gpio);
  iounmap(gpio_dir);
  iounmap(gpio_data);

  device_destroy(my_led_dtree_and_pdriver_class, MKDEV(major, 0));
  class_destroy(my_led_dtree_and_pdriver_class);
  unregister_chrdev(major, CHARDEV_NAME);
  printk(KERN_INFO "my_led_dtree_and_pdriver: Exit done\n");
}

module_init(my_led_dtree_and_pdriver_ini);
module_exit(my_led_dtree_and_pdriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");
MODULE_DESCRIPTION("My LED Dtree Driver");
