#ifndef _MY_EEPROM_IOCTL_H_
#define _MY_EEPROM_IOCTL_H_

#ifdef __KERNEL__
#include <linux/types.h>    // For kernel uint32_t
#else
#include <stdint.h>         // For userspace uint32_t
#endif

#include <linux/ioctl.h>     // Always needed for ioctl definitions

#define MY_EEPROM_MAGIC  'E'
#define MY_EEPROM_MAX_SIZE 256 // Maximum EEPROM size

struct my_eeprom_data {
    uint32_t offset;   // EEPROM offset
    uint32_t count;    // Number of bytes to read/write

#ifdef __KERNEL__
    uint8_t __user *buf; // kernel space expects __user pointer
#else
    uint8_t *buf;         // userspace just a normal pointer
#endif
};

#define MY_EEPROM_READ   _IOWR(MY_EEPROM_MAGIC, 1, struct my_eeprom_data)
#define MY_EEPROM_WRITE  _IOW(MY_EEPROM_MAGIC, 2, struct my_eeprom_data)

#endif /* _MY_EEPROM_IOCTL_H_ */
