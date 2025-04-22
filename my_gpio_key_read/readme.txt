Usage:
./my_gpio_key_application /dev/my_gpio_key_dev

note:
1.DECLARE_WAIT_QUEUE_HEAD(g_gpio_key_wait_queue)
2.wait_event_interruptible in read
3.wake_up_interruptible in irq

support nonblock:
1.int fd = open(argv[1], O_RDWR | O_NONBLOCK);
  or
  flag = fcntl(fd, F_GETFL);
  flag |= O_NONBLOCK;
2.driver side:
  dvr_read:
  if(!g_key_status_updated && file->f_flags & O_NONBLOCK)
    return -EAGAIN;