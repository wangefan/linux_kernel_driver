# This line means to generate the kernel module my_eeprom_driver.ko.
obj-m := my_eeprom_driver.o

# This line means my_eeprom_driver.ko are composed of the following object files.
my_eeprom_driver-objs := my_eeprom_driver_main.o my_chr_dev.o

# Userspace application
APP := my_eeprom_app

# Optional: for cross compile, override with make CC=...
CC := $(CROSS_COMPILE)gcc

# Should set cross-compile env before make
# export ARCH=arm
# export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-
# export PATH=$PATH:/media/yf/yf-ssd/my_project/linux_kernel_course/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin
# export KERN_DIR=/media/yf/yf-ssd/my_project/linux_kernel_course/100ask_imx6ull-sdk/Linux-4.9.88
# one line: export ARCH=arm CROSS_COMPILE=arm-buildroot-linux-gnueabihf- PATH=$PATH:/media/yf/yf-ssd/my_project/linux_kernel_course/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin KERN_DIR=/media/yf/yf-ssd/my_project/linux_kernel_course/100ask_imx6ull-sdk/Linux-4.9.88

# compile my_gpio_key_application
all:
	make -C $(KERN_DIR) M=$(PWD) modules
	$(CC) $(APP).c -o $(APP)

clean:
	make -C $(KERN_DIR) M=$(PWD) clean
	$(RM) $(APP)
