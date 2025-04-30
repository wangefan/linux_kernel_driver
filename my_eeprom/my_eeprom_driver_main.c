#include "my_chr_dev.h"
#include "my_eeprom_ioctl.h"
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/uaccess.h>
/*
consider useing module_i2c_driver(my_i2c_driver);
==============================================
static int __init my_i2c_driver_init(void)
{
    return i2c_add_driver(&my_i2c_driver);
}

static void __exit my_i2c_driver_exit(void)
{
    i2c_del_driver(&my_i2c_driver);
}

module_init(my_i2c_driver_init);
module_exit(my_i2c_driver_exit);
*/

#define eeprom_info(fmt, ...) pr_info("[my_eeprom]: " fmt, ##__VA_ARGS__)
#define eeprom_err(fmt, ...) pr_err("[my_eeprom]: " fmt, ##__VA_ARGS__)

#define DRIVER_NAME "my_i2c_eeprom_driver"
#define DEV_CLASS_NAME "my_eeprom_class"
#define DEV_NAME "my_eeprom"

static struct i2c_client *g_client;
static struct my_char_device_info g_mydev_info;

static int wait_eeprom_ready(struct i2c_client *client) {
  int retries = 10;

  while (retries--) {
    if (i2c_smbus_write_byte(client, 0x00) >= 0) {
      eeprom_info("EEPROM is ready after write\n");
      return 0;
    }
    msleep(1);
  }

  eeprom_err("EEPROM not ready after write (timeout)\n");
  return -ETIMEDOUT;
}

static long my_eeprom_ioctl(struct file *file, unsigned int cmd,
                            unsigned long arg) {
  struct my_eeprom_data eeprom_data;
  int ret = -1;
  u8 offset = 0;
  char buf[MY_EEPROM_MAX_SIZE] = {0};
  struct i2c_msg msg[2];
  struct i2c_adapter *adapter = to_i2c_adapter(g_client->dev.parent);

  eeprom_info("ioctl\n");
  if (copy_from_user(&eeprom_data, (void __user *)arg,
                     sizeof(struct my_eeprom_data))) {
    eeprom_err("Failed to copy data from user\n");
    return -EFAULT;
  }

  switch (cmd) {
  case MY_EEPROM_READ:
    eeprom_info("MY_EEPROM_READ, offset:%d, count:%d\n", eeprom_data.offset,
                eeprom_data.count);

    // random read byte
    // write the offset to the EEPROM
    msg[0].addr = g_client->addr;
    msg[0].flags = 0; // write
    msg[0].len = 1;
    offset = eeprom_data.offset;
    msg[0].buf = (u8 *)&offset;

    // read the data from the EEPROM
    msg[1].addr = g_client->addr;
    msg[1].flags = I2C_M_RD; // read
    msg[1].len = eeprom_data.count;
    msg[1].buf = buf;

    ret = i2c_transfer(adapter, msg, 2);
    if (ret != 2) {
      eeprom_err("i2c_transfer failed, ret = %d\n", ret);
      return -EIO;
    }

    if (copy_to_user(eeprom_data.buf, buf, eeprom_data.count)) {
      eeprom_err("copy_to_user failed\n");
      return -EFAULT;
    }

    ret = 0;
    break;
  case MY_EEPROM_WRITE:
    if (eeprom_data.offset + eeprom_data.count - 1 >= MY_EEPROM_MAX_SIZE ||
        eeprom_data.count > MY_EEPROM_MAX_SIZE - eeprom_data.offset) {
      eeprom_err("Data size exceeds max size\n");
      return -EINVAL;
    }
    buf[0] = eeprom_data.offset;
    if (copy_from_user(&buf[1], eeprom_data.buf, eeprom_data.count)) {
      eeprom_err("Failed to copy data from user\n");
      return -EFAULT;
    }
    msg[0].addr = g_client->addr;
    msg[0].flags = 0; // write
    msg[0].len = eeprom_data.count + 1;
    msg[0].buf = buf;

    ret = i2c_transfer(adapter, msg, 1);
    if (ret != 1) {
      eeprom_err("i2c_transfer failed, ret = %d\n", ret);
      return -EIO;
    }
    ret = wait_eeprom_ready(g_client);
    if (ret < 0)
      return ret;
    ret = 0;
    break;
  default:
    eeprom_err("Invalid command\n");
    return -EINVAL;
  }
  return ret;
}

static struct file_operations eeprom_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = my_eeprom_ioctl,
};

static int my_i2c_eeprom_probe(struct i2c_client *client,
                               const struct i2c_device_id *id) {
  int ret = -1;
  eeprom_info("probe\n");

  // create char dev to be accessed
  ret = register_my_char_device(&g_mydev_info, DEV_NAME, DEV_CLASS_NAME, 0, 0,
                                &eeprom_fops);
  if (ret < 0) {
    eeprom_err("Failed to register char device\n");
    return ret;
  }
  g_client = client;
  return 0;
}

static int my_i2c_eeprom_remove(struct i2c_client *client) {
  eeprom_info("remove\n");
  unregister_my_char_device(&g_mydev_info);
  return 0;
}

static const struct of_device_id my_eeprom_of_match[] = {
    {.compatible = "myvendor,my-i2c-eeprom-match"}, {}};

// This is provided for passing id matching check,
// we`re not using this field actually.
static const struct i2c_device_id my_eeprom_of_ids[] = {
    {"xxxx", (kernel_ulong_t)NULL}, {/*End of list*/}};

static struct i2c_driver my_i2c_eeprom_driver = {
    .driver =
        {
            .name = DRIVER_NAME,
            .owner = THIS_MODULE,
            .of_match_table = my_eeprom_of_match,
        },
    .probe = my_i2c_eeprom_probe,
    .remove = my_i2c_eeprom_remove,
    .id_table = my_eeprom_of_ids,
};

static int __init my_eeprom_driver_init(void) {
  eeprom_info("driver initialized\n");
  return i2c_add_driver(&my_i2c_eeprom_driver);
}

static void __exit my_eeprom_driver_exit(void) {
  eeprom_info("driver exited\n");
  i2c_del_driver(&my_i2c_eeprom_driver);
}

module_init(my_eeprom_driver_init);
module_exit(my_eeprom_driver_exit);
MODULE_LICENSE("GPL");