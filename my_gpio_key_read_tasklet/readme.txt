Usage:
./my_gpio_key_application /dev/my_gpio_key_dev

note:
1.decalre struct_tasklet in probe
2.tasklet_schedule in ISR
3.tasklet_kill in remove

note:
1.task_let function is processed in the softIRQ context
2.Can`t sleep, so can`t use mutex
3.Can use spinlock