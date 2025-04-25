Usage:
./my_gpio_key_application /dev/my_gpio_key_dev

note:
1.declare struct_work using INIT_WORK in probe
2.schedule_work in ISR

note:
1.struct_work function is processed in the process context
2.Can sleep, so can use mutex
3.Can use spinlock
4.Compare:
