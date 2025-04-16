#ifndef MY_CHR_DEV_H
#define MY_CHR_DEV_H

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

struct my_char_device_info {
  dev_t dev_num;
  struct cdev cdev;
  struct class *class;
  struct device *device;
};

int register_my_char_device(struct my_char_device_info *dev_info,
                            const char *dev_name, const char *class_name,
                            int major, int minor,
                            const struct file_operations *fops);

void unregister_my_char_device(struct my_char_device_info *dev_info);

#endif