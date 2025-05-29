Usage:
./my_ir_key_application /dev/input/event3

note:
dts
===
my-input-ir {
  compatible = "my_ir_input_driver";
  my-gpios = <&gpio4 19 GPIO_ACTIVE_HIGH>;
};

key_map
====
{
    { 0x45, KEY_POWER },
    { 0x47, KEY_MENU },

    { 0x44, KEY_T },           // Test
    { 0x40, KEY_VOLUMEUP },
    { 0x43, KEY_BACK },        // RETURN

    { 0x07, KEY_LAST },
    { 0x15, KEY_PLAYPAUSE },
    { 0x09, KEY_NEXT },

    { 0x16, KEY_0 },
    { 0x19, KEY_VOLUMEDOWN },
    { 0x0d, KEY_C },

    { 0x0c, KEY_1 },
    { 0x18, KEY_2 },
    { 0x5e, KEY_3 },
    { 0x08, KEY_4 },
    { 0x1c, KEY_5 },
    { 0x5a, KEY_6 },
    { 0x42, KEY_7 },
    { 0x52, KEY_8 },
    { 0x4a, KEY_9 },
};