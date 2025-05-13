#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>

#define print_my_uart_info(fmt, ...) pr_info("[my_uart]: " fmt, ##__VA_ARGS__)
#define print_my_uart_err(fmt, ...) pr_err("[my_uart]: " fmt, ##__VA_ARGS__)

static struct uart_port *my_uart_port;

static struct uart_ops my_uart_ops = {
    .tx_empty = NULL,
    .set_mctrl = NULL,
    .get_mctrl = NULL,
    .stop_tx = NULL,
    .start_tx = NULL,
    .stop_rx = NULL,
    //.enable_ms    = NULL,
    //.break_ctl    = NULL,
    .startup = NULL,
    .shutdown = NULL,
    //.flush_buffer = NULL,
    .set_termios = NULL,
    .type = NULL,
    //.config_port  = NULL,
    //.verify_port  = NULL,
};

static struct uart_driver my_uart_drv = {
    .owner = THIS_MODULE,
    .driver_name = "my_tty",
    .dev_name = "my_tty_dev",
    .major = 0,
    .minor = 0,
    .nr = 1,
};

static const struct of_device_id my_uart_of_match[] = {
    {.compatible = "me,my-uart-driver-match"}, {}};

static int my_uart_probe(struct platform_device *pdev) {

  print_my_uart_info("probe\n");

  // allocate uart_port
  my_uart_port = devm_kzalloc(&pdev->dev, sizeof(struct uart_port), GFP_KERNEL);

  // set uart_port parameters
  my_uart_port->dev = &pdev->dev;
  my_uart_port->iotype = UPIO_MEM;
  my_uart_port->fifosize = 32;
  my_uart_port->ops = &my_uart_ops;
  my_uart_port->flags = UPF_BOOT_AUTOCONF;
  my_uart_port->type = PORT_8250;

  uart_add_one_port(&my_uart_drv, my_uart_port);
  return 0;
}

static int my_uart_remove(struct platform_device *pdev) {
  print_my_uart_info("remove\n");
  uart_remove_one_port(&my_uart_drv, my_uart_port);
  uart_unregister_driver(&my_uart_drv);
  return 0;
}

static struct platform_driver my_uart_platform_driver = {
    .driver =
        {
            .name = "my_uart_platform_driver_name",
            .of_match_table = my_uart_of_match,
        },
    .probe = my_uart_probe,
    .remove = my_uart_remove,
};

static int __init my_uart_init(void) {
  int ret;

  print_my_uart_info("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

  ret = uart_register_driver(&my_uart_drv);

  if (ret)
    return ret;

  return platform_driver_register(&my_uart_platform_driver);
}

static void __exit my_uart_exit(void) {
  print_my_uart_info("%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
  platform_driver_unregister(&my_uart_platform_driver);
  uart_unregister_driver(&my_uart_drv);
}

module_init(my_uart_init);
module_exit(my_uart_exit);

MODULE_LICENSE("GPL");