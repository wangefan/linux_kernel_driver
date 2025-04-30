Usage:
./my_eeprom_app <w/r> <char node> <address offset> [<count>] [<characters>]
ex:
./my_eeprom_app w /dev/my_eeprom 0x00 'abcde'
./my_eeprom_app r /dev/my_eeprom 0x00 5
abcde


note:
dts
=========
&i2c1 {
    my_eeprom@50 {
        compatible = "myvendor,my-i2c-eeprom-match";
        reg = <0x50>;
    };
};