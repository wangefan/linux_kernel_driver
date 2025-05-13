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