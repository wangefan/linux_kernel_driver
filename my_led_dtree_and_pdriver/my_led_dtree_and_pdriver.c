#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define led_info(fmt, ...) pr_info("[my_LED]: " fmt, ##__VA_ARGS__)
#define led_err(fmt, ...) pr_err("[my_LED]: " fmt, ##__VA_ARGS__)

#define DRIVER_NAME "my_led_dtree_and_pdriver"
#define CLASS_NAME "my_led_class"

// define file mode, all readable, user and group are writiable
#define MY_LED_RW_ATTR (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP)

static ssize_t my_led_show(struct class *class, struct class_attribute *attr,
                           char *buf);
static ssize_t my_led_store(struct class *class, struct class_attribute *attr,
                            const char *buf, size_t count);

static struct class_attribute my_led_class_attrs[] = {
    __ATTR(led_ctrl, MY_LED_RW_ATTR, my_led_show, my_led_store),
};

static struct class my_led_class = {
    .name = CLASS_NAME,
};

static const struct of_device_id my_led_dt_match[] = {
    {
        .compatible = "my_led_dtree_and_pdriver,my_leddriver",
    },
    {},
};

static ssize_t my_led_show(struct class *class, struct class_attribute *attr,
                           char *buf) {
  led_info("show...\n");
  return sprintf(buf, "LED status: ON\n");
}

static ssize_t my_led_store(struct class *class, struct class_attribute *attr,
                            const char *buf, size_t count) {
  led_info("store...\n");
  if (strncmp(buf, "on", 2) == 0) {
    led_info("LED ON\n");
  } else if (strncmp(buf, "off", 3) == 0) {
    led_info("LED OFF\n");
  } else {
    led_err("Invalid command\n");
    return -EINVAL;
  }
  return count;
}

static int my_led_probe(struct platform_device *pdev) {
  int ret = -1;
  int class_attr_num = 0, idx_class_attr = 0;

  led_info("probe...\n");

  // register the class
  ret = class_register(&my_led_class);
  if (ret) {
    led_err("class_register failed!\n");
    return ret;
  }

  // create the class
  class_attr_num = ARRAY_SIZE(my_led_class_attrs);
  for (idx_class_attr = 0; idx_class_attr < class_attr_num; idx_class_attr++) {
    ret = class_create_file(&my_led_class, &my_led_class_attrs[idx_class_attr]);
    if (ret) {
      led_err("class_create_file failed!\n");
      goto err;
    }
  }

  led_info("probe ok\n");
  return ret;
err:
  while (--idx_class_attr >= 0)
    class_remove_file(&my_led_class, &my_led_class_attrs[idx_class_attr]);
  class_unregister(&my_led_class);
  return ret;
}

static int my_led_remove(struct platform_device *pdev) {
  int idx_class_attr = 0;
  led_info("remove...\n");
  for (idx_class_attr = 0; idx_class_attr < ARRAY_SIZE(my_led_class_attrs);
       idx_class_attr++)
    class_remove_file(&my_led_class, &my_led_class_attrs[idx_class_attr]);
  class_unregister(&my_led_class);
  led_info("remove ok\n");
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
