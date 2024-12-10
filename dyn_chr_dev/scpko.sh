sudo scp dyn_chr_dev.ko /lib/modules/5.15.0-125-generic/

sudo rm -rf /lib/modules/5.15.0-125-generic/dyn_chr_dev.ko

sudo depmod
sudo modprobe dyn_chr_dev
sudo rmmod dyn_chr_dev

cat /proc/sys/kernel/printk
echo 7 > /proc/sys/kernel/printk