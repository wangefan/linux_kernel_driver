Usage:
Tx:
   echo "uart tx data" > /dev/my_tty_dev0

simulate showing data by hw:
   cat /proc/my_tty
   uart tx data

simulate gen data by hw:
   echo "uart Rx data" > /proc/my_tty
Rx:
   cat /dev/my_tty_dev0
   uart tx data

dts:
=========
/ {
    my_uart: my_uart {
        compatible = "me,my-uart-driver-match";

        interrupt-parent = <&intc>; // use the specified GIC Interrupt Controller
        interrupts = <GIC_SPI 99 IRQ_TYPE_LEVEL_HIGH>; // intc(the Interrupt Controller) knows how to parse it
    };
};

console:
1.uboot set console=ttyMyConsole:
  setenv mmcargs setenv bootargs console=${console},${baudrate} root=${mmcroot} console=ttyMyConsole
  boot

2.now tty could be console:
  insmod my_uart_driver.ko
  cat /proc/consoles
  ttyMyConsole0        -W- (EC p  )  240:0
  ttymxc0              -W- (E  p  )  207:16

3.now could write to console(ttyMyConsole)
  echo "newone" > /dev/console
  echo "newtwo" > /dev/console

  simulate to read output from console:
  cat /proc/my_tty
  [  501.698606] [my_uart]: my_uart_console_device
  [  588.525940] [my_uart]: my_uart_console_device
  [  588.532310] [my_uart]: my_uart_start_tx
  newone[  588.536537] [my_uart]: my_uart_start_tx

  [  588.542393] [my_uart]: my_uart_start_tx
  [  598.525649] [my_uart]: my_uart_console_device
  [  598.539498] [my_uart]: my_uart_start_tx
  newtwo[  598.543399] [my_uart]: my_uart_start_tx

  [  598.547265] [my_uart]: my_uart_start_tx
  [root@100ask:~/testing]#
