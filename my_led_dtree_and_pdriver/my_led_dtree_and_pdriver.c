#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define led_info(fmt, ...) pr_info("[my_LED]: " fmt, ##__VA_ARGS__)
#define led_err(fmt, ...) pr_err("[my_LED]: " fmt, ##__VA_ARGS__)

#define DRIVER_NAME "my_led_dtree_and_pdriver"

static const struct of_device_id my_led_dt_match[] = {
    {
        .compatible = "my_led_dtree_and_pdriver,my_leddriver",
    },
    {},
};

static int my_led_probe(struct platform_device *pdev) {
  led_info("probe...\n");
  return 0;
}

static int my_led_remove(struct platform_device *pdev) {
  led_info("remove...\n");
  return 0;
}

static int my_led_suspend(struct platform_device *pdev, pm_message_t state) {
  led_info("suspend...\n");
  return 0;
}

static int my_led_resume(struct platform_device *pdev) {
  led_info("resume...\n");
  return 0;
}

static void my_led_shutdown(struct platform_device *pdev) {
  led_info("shutdown...\n");
}

static struct platform_driver my_led_driver = {
    .driver =
        {
            .name = DRIVER_NAME,
            .of_match_table = my_led_dt_match,
        },
    .probe = my_led_probe,
    .remove = my_led_remove,
    .suspend = my_led_suspend,
    .resume = my_led_resume,
    .shutdown = my_led_shutdown,
};

static int __init my_led_dtree_and_pdriver_ini(void) {
  int ret = -1;

  led_info("Initializing...\n");

  ret = platform_driver_register(&my_led_driver);
  if (ret) {
    led_err("platform_driver_register failed!\n");
    return -ENODEV;
  }

  led_info("Initializing done\n");
  return ret;
}

static void __exit my_led_dtree_and_pdriver_exit(void) {
  led_info("Exiting...\n");
  platform_driver_unregister(&my_led_driver);
  led_info("Exit done\n");
}

module_init(my_led_dtree_and_pdriver_ini);
module_exit(my_led_dtree_and_pdriver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");
MODULE_DESCRIPTION("My LED Dtree Driver");
