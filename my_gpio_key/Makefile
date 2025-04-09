# This line would compile the module my_gpio_key.c
# to generate the kernel module my_gpio_key.ko
# ps:
#   if multiple *.o files to one *.o file, use += to add them:
# ex:
#   my_led-y := my_led1.o my_led2.o
#   obj-m += my_led.o
obj-m += my_gpio_key.o


# Should set cross-compile env before make
# export ARCH=arm
# export CROSS_COMPILE=arm-buildroot-linux-gnueabihf-
# export PATH=$PATH:/media/yf/yf-ssd/my_project/linux_kernel_course/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin
# export KERN_DIR=/media/yf/yf-ssd/my_project/linux_kernel_course/100ask_imx6ull-sdk/Linux-4.9.88
# one line: export ARCH=arm CROSS_COMPILE=arm-buildroot-linux-gnueabihf- PATH=$PATH:/media/yf/yf-ssd/my_project/linux_kernel_course/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/bin KERN_DIR=/media/yf/yf-ssd/my_project/linux_kernel_course/100ask_imx6ull-sdk/Linux-4.9.88

all:
	make -C $(KERN_DIR) M=$(PWD) modules

clean:
	make -C $(KERN_DIR) M=$(PWD) clean
