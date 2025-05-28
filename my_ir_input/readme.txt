Usage:
./my_ir_key_application /dev/input/eventx

note:
dts
===
my-input-ir {
  compatible = "my_ir_input_driver";
  my-gpios = <&gpio4 19 GPIO_ACTIVE_HIGH>;
};
