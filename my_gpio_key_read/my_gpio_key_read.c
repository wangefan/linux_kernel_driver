#include <linux/gpio/consumer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "my_chr_dev.h"

#define gpio_key_info(fmt, ...) pr_info("[my_GPIO]: " fmt, ##__VA_ARGS__)
#define gpio_key_err(fmt, ...) pr_err("[my_GPIO]: " fmt, ##__VA_ARGS__)

#define DRIVER_NAME "my_gpio_driver"

const char *DEV_CLASS_NAME = "my_gpio_key_class";
const char *DEV_NAME = "my_gpio_key_dev";
static struct my_char_device_info mydev_info;

static int my_chr_open(struct inode *inode, struct file *file) {
  gpio_key_info("my_chr_open\n");
  return 0;
}

static int my_chr_release(struct inode *inode, struct file *file) {
  gpio_key_info("my_chr_release\n");
  return 0;
}

static ssize_t my_chr_read(struct file *file, char __user *buf, size_t count,
                           loff_t *offset) {
  gpio_key_info("my_chr_read\n");
  return 0;
}

static ssize_t my_chr_write(struct file *file, const char __user *buf,
                            size_t count, loff_t *offset) {
  gpio_key_info("my_chr_write\n");
  return count;
}

static struct file_operations dyn_chr_fops = {
    .owner = THIS_MODULE,
    .open = my_chr_open,
    .release = my_chr_release,
    .read = my_chr_read,
    .write = my_chr_write,
};

struct my_gpio_key_info {
  struct gpio_desc *gpio_desc;
  int irq_id;
};

static struct my_gpio_key_info *g_gpio_key_infos = NULL;

static const struct of_device_id my_gpio_key_dt_match[] = {
    {
        .compatible = "my_gpio_key_driver",
    },
    {},
};

static irqreturn_t my_gpio_key_irq(int irq, void *dev_id) {
  struct my_gpio_key_info *gpio_key = (struct my_gpio_key_info *)dev_id;
  int key_status = gpiod_get_value(gpio_key->gpio_desc);
  if (key_status < 0) {
    gpio_key_err("Failed to get GPIO value\n");
    return IRQ_HANDLED;
  }
  // handle the interrupt
  gpio_key_info("GPIO %d triggered IRQ %d, key %d!\n",
                desc_to_gpio(gpio_key->gpio_desc), gpio_key->irq_id,
                key_status);

  return IRQ_HANDLED;
}

static int my_gpio_key_probe(struct platform_device *pdev) {
  int ret = -1, idx_gpio = 0;
  int gpio_count = 0;
  gpio_key_info("probe...\n");

  // get gpio count from device tree
  gpio_count = gpiod_count(&pdev->dev, "my");

  if (gpio_count <= 0) {
    gpio_key_err("Failed to get GPIO count\n");
    return -EINVAL;
  }
  gpio_key_info("Get the GPIO count: %d\n", gpio_count);

  // allocate memory for gpio_key_info
  g_gpio_key_infos =
      kzalloc(gpio_count * sizeof(struct my_gpio_key_info), GFP_KERNEL);
  if (!g_gpio_key_infos) {
    gpio_key_err("Failed to allocate memory for gpio_keys\n");
    return -ENOMEM;
  }

  // iterate through the GPIOs, save to g_gpio_key_infos
  for (idx_gpio = 0; idx_gpio < gpio_count; idx_gpio++) {
    g_gpio_key_infos[idx_gpio].gpio_desc =
        gpiod_get_index(&pdev->dev, "my", idx_gpio, GPIOD_IN);
    if (IS_ERR(g_gpio_key_infos[idx_gpio].gpio_desc)) {
      gpio_key_err("Failed to get GPIO %d\n", idx_gpio);
      return -EINVAL;
    }
    g_gpio_key_infos[idx_gpio].irq_id =
        gpiod_to_irq(g_gpio_key_infos[idx_gpio].gpio_desc);
    if (g_gpio_key_infos[idx_gpio].irq_id < 0) {
      gpio_key_err("Failed to get IRQ for GPIO %d\n",
                   desc_to_gpio(g_gpio_key_infos[idx_gpio].gpio_desc));
      return -EINVAL;
    }

    // request IRQ with flag IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING
    ret = request_irq(g_gpio_key_infos[idx_gpio].irq_id,
                      my_gpio_key_irq, // IRQ handler function
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "my_gpio_irq",
                      &g_gpio_key_infos[idx_gpio]);
    if (ret) {
      gpio_key_err("Failed to request IRQ %d\n",
                   g_gpio_key_infos[idx_gpio].irq_id);
      return ret;
    }
    gpio_key_info("Get the GPIO %d(th) id: %d, the converted IRQ: %d\n",
                  idx_gpio, desc_to_gpio(g_gpio_key_infos[idx_gpio].gpio_desc),
                  g_gpio_key_infos[idx_gpio].irq_id);
  }

  return ret;
}

static int my_gpio_key_remove(struct platform_device *pdev) {
  int idx_gpio = 0;
  int gpio_count = 0;
  gpio_key_info("remove...\n");
  // free irq
  gpio_count = gpiod_count(&pdev->dev, "my");
  for (idx_gpio = 0; idx_gpio < gpio_count; idx_gpio++) {
    free_irq(g_gpio_key_infos[idx_gpio].irq_id, &g_gpio_key_infos[idx_gpio]);
    gpiod_put(g_gpio_key_infos[idx_gpio].gpio_desc); // free GPIO descriptor
  }

  if (g_gpio_key_infos) {
    kfree(g_gpio_key_infos);
    g_gpio_key_infos = NULL;
  }

  return 0;
}

static struct platform_driver my_gpio_key_driver = {
    .driver =
        {
            .name = DRIVER_NAME,
            .of_match_table = my_gpio_key_dt_match,
        },
    .probe = my_gpio_key_probe,
    .remove = my_gpio_key_remove,
};

static int __init my_gpio_key_ini(void) {
  int ret = -1;

  gpio_key_info("Initializing...\n");

  ret = platform_driver_register(&my_gpio_key_driver);
  if (ret) {
    gpio_key_err("platform_driver_register failed!\n");
    return -ENODEV;
  }

  ret = register_my_char_device(&mydev_info, DEV_NAME,
                                DEV_CLASS_NAME, 0, 0, &dyn_chr_fops);
  if (ret < 0) {
    gpio_key_err("Failed to register char device\n");
    return ret;
  }

  gpio_key_info("Initializing done\n");
  return ret;
}

static void __exit my_gpio_key_exit(void) {
  gpio_key_info("Exiting...\n");
  unregister_my_char_device(&mydev_info);
  platform_driver_unregister(&my_gpio_key_driver);
  gpio_key_info("Exit done\n");
}

module_init(my_gpio_key_ini);
module_exit(my_gpio_key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");
MODULE_DESCRIPTION("My GPIO Driver");