#include <linux/gpio/consumer.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define led_info(fmt, ...) pr_info("[my_LED]: " fmt, ##__VA_ARGS__)
#define led_err(fmt, ...) pr_err("[my_LED]: " fmt, ##__VA_ARGS__)

#define DRIVER_NAME "my_led_dtree_and_pdriver"
#define CLASS_NAME "my_led_class"

enum MY_LED_STATE {
  led_off = 0,
  led_on = 1,
  led_max,
};

// define file mode, all readable, user and group are writiable
#define MY_LED_RW_ATTR (S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP)

static struct gpio_desc* my_led_gpio = NULL;
static int my_led_state;

static ssize_t my_led_show(struct class *class, struct class_attribute *attr,
                           char *buf);
static ssize_t my_led_store(struct class *class, struct class_attribute *attr,
                            const char *buf, size_t count);

static void set_led_state(enum MY_LED_STATE state) {
  led_info("set_led_state: %d\n", state);
  if (state == led_on) {
    gpiod_set_value(my_led_gpio, 1);
    my_led_state = led_on;
  } else {
    gpiod_set_value(my_led_gpio, 0);
    my_led_state = led_off;
  }
}

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
  int ret = -1;
  ret = gpiod_get_value(my_led_gpio);
  if (ret < 0) {
    led_err("gpiod_get_value failed!\n");
    return ret;
  }
  if (ret == 0) {
    my_led_state = led_off;
  } else {
    my_led_state = led_on;
  }
  return sprintf(buf, "%d\n", ret);
}

static ssize_t my_led_store(struct class *class, struct class_attribute *attr,
                            const char *buf, size_t count) {
  int cmd;
  led_info("store...\n");

  if (kstrtoint(buf, 0, &cmd)) {
    led_err("kstrtoint cmd:%d failed!\n", cmd);
    return -EINVAL;
  }
  if (cmd < 0 || cmd >= led_max) {
    led_err("Invalid command: %d\n", cmd);
    return -EINVAL;
  }
  if (cmd == led_on) {
    set_led_state(led_on);
    led_info("LED on\n");
  } else if (cmd == led_off) {
    set_led_state(led_off);
    led_info("LED off\n");
  } else {
    led_err("Invalid command: %d\n", cmd);
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
      goto clean_class_and_files;
    }
  }

  // get the GPIO from device tree
  my_led_gpio = gpiod_get(&pdev->dev, "my-led", GPIOD_ASIS);
  if (IS_ERR(my_led_gpio)) {
    led_err("gpiod_get failed!\n");
    ret = PTR_ERR(my_led_gpio);
    goto clean_class_and_files;
  }

  // set the GPIO direction with default value 0
  // 0 means logic off, 1 means logic on LED
  ret = gpiod_direction_output(my_led_gpio, 0);
  if (ret) {
    led_err("gpiod_direction_output failed!\n");
    goto clean_class_and_files;
  }

  // turn led off
  set_led_state(led_off);

  led_info("probe ok\n");
  return ret;

clean_class_and_files:
  while (--idx_class_attr >= 0)
    class_remove_file(&my_led_class, &my_led_class_attrs[idx_class_attr]);
  class_unregister(&my_led_class);
  return ret;
}

static int my_led_remove(struct platform_device *pdev) {
  int idx_class_attr = 0;
  led_info("remove...\n");
  gpiod_put(my_led_gpio);
  my_led_gpio = NULL;
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
