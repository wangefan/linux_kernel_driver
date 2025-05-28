#include <linux/gpio/consumer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>

#define my_ir_info(fmt, ...) pr_info("[MY_IR_INPUT]: " fmt, ##__VA_ARGS__)
#define my_ir_err(fmt, ...) pr_err("[MY_IR_INPUT]: " fmt, ##__VA_ARGS__)

#define DRIVER_NAME "my_ir_input_driver_name"

struct gpio_desc *g_ir_gpio_pin;
static int g_irq;

static irqreturn_t my_ir_isr(int irq, void *dev_id) {
  my_ir_info("IR interrupt triggered on GPIO %d\n",
             desc_to_gpio(g_ir_gpio_pin));
  return IRQ_HANDLED;
}

static const struct of_device_id my_ir_input_dt_match[] = {
    {
        .compatible = "my_ir_input_driver",
    },
    {},
};

static int my_ir_input_probe(struct platform_device *pdev) {
  int ret = -1;
  my_ir_info("probe...\n");

  g_ir_gpio_pin = gpiod_get(&pdev->dev, "my", 0);
  if (IS_ERR(g_ir_gpio_pin))
    my_ir_err("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

  g_irq = gpiod_to_irq(g_ir_gpio_pin);

  ret =
      request_irq(g_irq, my_ir_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                  "my_ir_isr_name", NULL);
  if (ret) {
    my_ir_err("Failed to request IRQ %d for GPIO %d\n", g_irq,
              desc_to_gpio(g_ir_gpio_pin));
    gpiod_put(g_ir_gpio_pin);
    return ret;
  }

  return ret;
}

static int my_ir_input_remove(struct platform_device *pdev) {
  my_ir_info("remove...\n");
  free_irq(g_irq, NULL);
  gpiod_put(g_ir_gpio_pin);
  return 0;
}

static struct platform_driver g_my_ir_input_driver = {
    .driver =
        {
            .name = DRIVER_NAME,
            .of_match_table = my_ir_input_dt_match,
        },
    .probe = my_ir_input_probe,
    .remove = my_ir_input_remove,
};

static int __init my_ir_input_ini(void) {
  int ret = -1;

  my_ir_info("Initializing...\n");

  ret = platform_driver_register(&g_my_ir_input_driver);
  if (ret) {
    my_ir_err("platform_driver_register failed!\n");
    return -ENODEV;
  }

  my_ir_info("Initializing done\n");
  return ret;
}

static void __exit my_ir_input_exit(void) {
  my_ir_info("Exiting...\n");
  platform_driver_unregister(&g_my_ir_input_driver);
  my_ir_info("Exit done\n");
}

module_init(my_ir_input_ini);
module_exit(my_ir_input_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");
MODULE_DESCRIPTION("My GPIO Driver");