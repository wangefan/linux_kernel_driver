sudo scp dyn_chr_dev.ko /lib/modules/5.15.0-125-generic/

sudo rm -rf /lib/modules/5.15.0-125-generic/dyn_chr_dev.ko

sudo depmod
sudo modprobe dyn_chr_dev
sudo rmmod dyn_chr_dev

$ ls -al /sys/class
..
drwxr-xr-x  2 root root 0 十二  9 08:31 drm_dp_aux_dev
drwxr-xr-x  2 root root 0 十二 10 15:36 dyn_chr_dev_class

$ ls -al /dev/ | grep dyn_
crw-------   1 root root    509,     0 十二 10 15:36 dyn_chr_dev_device

cat /proc/sys/kernel/printk
echo 7 > /proc/sys/kernel/printk