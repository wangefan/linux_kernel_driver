Usage:
./my_gpio_key_app_sig /dev/my_gpio_key_driver

note:
User space:
1. register signal handler
2. fcntl to set user space app pid/set fasync flag
3. add drv_fasync(usually call fasync_helper) function that
   set/unset fa_file, which keep the userspace pid info
4. trigger signal via kill_fasync in IRQ in the driver.