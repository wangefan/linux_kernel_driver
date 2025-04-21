Usage:
./my_gpio_key_app_poll /dev/my_gpio_key_driver

note:
1. Add poll before read data in application.
2. drv_poll should call poll_wait, which will
   put current thread in queue to be waked from
   IRQ. it won`t put thread again if it exist.
3. drv_poll return the status:
   has data to read => POLLIN | POLLRDNORM
   no data => 0