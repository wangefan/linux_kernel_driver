Usage:
./my_gpio_key_application /dev/my_gpio_key_dev

note:
1.DECLARE_WAIT_QUEUE_HEAD(g_gpio_key_wait_queue)
2.wait_event_interruptible in read
3.wake_up_interruptible in irq