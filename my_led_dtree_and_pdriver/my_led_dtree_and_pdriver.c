#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>

#define led_info(fmt, ...) pr_info("[my_LED]: " fmt, ##__VA_ARGS__)

static int __init my_led_dtree_and_pdriver_ini(void) {
  int ret = -1;

  led_info("Initializing...\n");

  ret = 0;

  led_info("Initializing done\n");
  return ret;
}

static void __exit my_led_dtree_and_pdriver_exit(void) {
  led_info("Exiting...\n");

  led_info("Exit done\n");
}

module_init(my_led_dtree_and_pdriver_ini);
module_exit(my_led_dtree_and_pdriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");
MODULE_DESCRIPTION("My LED Dtree Driver");
