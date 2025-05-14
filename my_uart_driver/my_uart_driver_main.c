#include "my_ring_buffer.h"
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/serial_core.h>
#include <linux/sysrq.h>
#include <linux/tty_flip.h>

#define print_my_uart_info(fmt, ...) pr_info("[my_uart]: " fmt, ##__VA_ARGS__)
#define print_my_uart_err(fmt, ...) pr_err("[my_uart]: " fmt, ##__VA_ARGS__)

static struct uart_port *my_uart_port;

static struct my_ring_buffer rx_buffer;
static struct my_ring_buffer tx_buffer;

static irqreturn_t rx_irq_handler(int irq, void *dev_id) {
  struct uart_port *port = dev_id;
  struct tty_port *tport = &port->state->port;
  unsigned long flags;
  int data_cnt, i;
  print_my_uart_info("rx_irq_handler\n");

  spin_lock_irqsave(&port->lock, flags);

  // consume data from rx buffer, send to discipline
  data_cnt = get_data_count(&rx_buffer);
  for (i = 0; i < data_cnt; i++) {
    unsigned char data;
    data = get_data(&rx_buffer);
    // put data to ldisc
    tty_insert_flip_char(tport, data, TTY_NORMAL);

    // update info
    port->icount.rx++;
  }

  spin_unlock_irqrestore(&port->lock, flags);

  // notify tty layer that we have data, could wake up
  // read() syscall
  tty_flip_buffer_push(tport);
  return IRQ_HANDLED;
}

static ssize_t my_hw_proc_file_show(struct file *file, char __user *buf,
                                    size_t count, loff_t *ppos) {
  int data_cnt, i, ret;
  unsigned char data;

  print_my_uart_info("my_hw_proc_file_show\n");
  data_cnt = get_data_count(&tx_buffer);

  data_cnt = (data_cnt > count) ? count : data_cnt;

  for (i = 0; i < data_cnt; i++) {
    data = get_data(&tx_buffer);
    ret = copy_to_user(buf + i, &data, 1);
  }

  return data_cnt;
}

static ssize_t my_hw_proc_file_generate(struct file *file,
                                        const char __user *buf, size_t count,
                                        loff_t *ppos) {
  int data_cnt = 0, i = 0;
  unsigned char data;
  print_my_uart_info("my_hw_proc_file_generate\n");

  // check if buffer is full
  if (is_full(&rx_buffer)) {
    print_my_uart_info("Rx Buffer is full, cannot generate data\n");
    return -EBUSY;
  }
  // write data to rx buffer
  data_cnt = min(count, get_space_count(&rx_buffer));
  for (i = 0; i < data_cnt; i++) {
    if (copy_from_user(&data, buf + i, 1)) {
      print_my_uart_err("Failed to copy data from user\n");
      return -EFAULT;
    }
    put_data(&rx_buffer, data);
  }

  // fire irq to notify get rx data
  // In real case, this should be done by hardware
  // here would trigger the rx_irq_handler
  irq_set_irqchip_state(my_uart_port->irq, IRQCHIP_STATE_PENDING, 1);

  return data_cnt;
}

static const struct file_operations my_hw_proc_file_fops = {
    .read = my_hw_proc_file_show,
    .write = my_hw_proc_file_generate,
};

static struct proc_dir_entry *my_hw_proc_file;

/* uart_ops begin */
static unsigned int my_uart_tx_empty(struct uart_port *port) {
  /* 因为要发送的数据瞬间存入buffer */
  return 1;
}

static void my_uart_set_mctrl(struct uart_port *port, unsigned int mctrl) {}

static unsigned int my_uart_get_mctrl(struct uart_port *port) { return 0; }

static void my_uart_stop_tx(struct uart_port *port) {}

static void my_uart_start_tx(struct uart_port *port) {
  struct circ_buf *xmit = &port->state->xmit;
  unsigned char data;
  print_my_uart_info("my_uart_start_tx\n");
  while (!uart_circ_empty(xmit)) {
    if (is_full(&tx_buffer)) {
      print_my_uart_info("Buffer is full, stopping transmission\n");
      break;
    }

    // get data from xmit buffer
    data = xmit->buf[xmit->tail];
    xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
    port->icount.tx++;

    // save to local tx buffer
    put_data(&tx_buffer, data);
  }

  // This is to notify app that the xmit buffer is in
  // low water mark, could be write to /dev/tty to tx
  // more data
  if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
    uart_write_wakeup(port);
}

static int my_uart_startup(struct uart_port *port) { return 0; }
static void my_uart_set_termios(struct uart_port *port,
                                struct ktermios *termios,
                                struct ktermios *old) {
  return;
}

static void my_uart_stop_rx(struct uart_port *port) {}

static void my_uart_shutdown(struct uart_port *port) {}

static const char *my_uart_type(struct uart_port *port) {
  return "MY_UART_TYPE";
}
/* uart_ops end */

static struct uart_ops my_uart_ops = {
    .tx_empty = my_uart_tx_empty,
    .set_mctrl = my_uart_set_mctrl,
    .get_mctrl = my_uart_get_mctrl,
    .stop_tx = my_uart_stop_tx,
    .start_tx = my_uart_start_tx,
    .stop_rx = my_uart_stop_rx,
    //.enable_ms    = NULL,
    //.break_ctl    = NULL,
    .startup = my_uart_startup,
    .shutdown = my_uart_shutdown,
    //.flush_buffer = NULL,
    .set_termios = my_uart_set_termios,
    .type = my_uart_type,
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
  int rx_irq, ret;

  print_my_uart_info("probe\n");

  // init buffer
  init_buffer(&rx_buffer);
  init_buffer(&tx_buffer);

  // create proc entry to emulate HW generating/cat data
  my_hw_proc_file = proc_create("my_tty", 0, NULL, &my_hw_proc_file_fops);

  // get irq
  rx_irq = platform_get_irq(pdev, 0);

  // allocate uart_port
  my_uart_port = devm_kzalloc(&pdev->dev, sizeof(struct uart_port), GFP_KERNEL);

  // set uart_port parameters
  my_uart_port->dev = &pdev->dev;
  my_uart_port->iotype = UPIO_MEM;
  my_uart_port->irq = rx_irq;
  my_uart_port->fifosize = 32;
  my_uart_port->ops = &my_uart_ops;
  my_uart_port->flags = UPF_BOOT_AUTOCONF;
  my_uart_port->type = PORT_8250;

  // request irq
  ret = devm_request_irq(&pdev->dev, rx_irq, rx_irq_handler, 0,
                         dev_name(&pdev->dev), my_uart_port);

  uart_add_one_port(&my_uart_drv, my_uart_port);
  return 0;
}

static int my_uart_remove(struct platform_device *pdev) {
  print_my_uart_info("remove\n");
  uart_remove_one_port(&my_uart_drv, my_uart_port);
  proc_remove(my_hw_proc_file);
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