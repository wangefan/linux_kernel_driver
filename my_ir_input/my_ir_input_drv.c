#include <linux/gpio/consumer.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <media/rc-map.h>

#define my_ir_info(fmt, ...) pr_info("[MY_IR_INPUT]: " fmt, ##__VA_ARGS__)
#define my_ir_err(fmt, ...) pr_err("[MY_IR_INPUT]: " fmt, ##__VA_ARGS__)

#define DRIVER_NAME "my_ir_input_driver_name"
#define EDGE_MAX 80
#define TIMEOUT_NS 60000000       // 60ms
#define HEAD_DIFF_NS 9000000      // 9ms
#define TAIL_LEAD_DIFF_NS 4500000 // 4.5ms
#define TAIL_REP_DIFF_NS 2250000  // 2.25ms
#define TAIL_VAL_1_NS 1690000     // 2.25ms - 0.56ms = 1.69ms
#define TOR_NS 800000             // 0.8ms

struct gpio_desc *g_ir_gpio_pin;
static int g_irq;

static struct rc_map_table hs0038_nec_map[] = {
    {0x45, KEY_POWER},    {0x47, KEY_MENU},

    {0x44, KEY_T},                          // Test
    {0x40, KEY_VOLUMEUP}, {0x43, KEY_BACK}, // RETURN

    {0x07, KEY_LAST},     {0x15, KEY_PLAYPAUSE},  {0x09, KEY_NEXT},

    {0x16, KEY_0},        {0x19, KEY_VOLUMEDOWN}, {0x0d, KEY_C},

    {0x0c, KEY_1},        {0x18, KEY_2},          {0x5e, KEY_3},
    {0x08, KEY_4},        {0x1c, KEY_5},          {0x5a, KEY_6},
    {0x42, KEY_7},        {0x52, KEY_8},          {0x4a, KEY_9},
};

int find_keycode_from_scancode(u16 scancode) {
  int i;
  for (i = 0; i < ARRAY_SIZE(hs0038_nec_map); i++) {
    if (hs0038_nec_map[i].scancode == scancode) {
      return hs0038_nec_map[i].keycode;
    }
  }
  return -1; // Not found
}

static struct input_dev *g_ir_input_dev;

enum MY_IR_RAWDATA_RES {
  MY_IR_RAWDATA_DATA = 0,
  MY_IR_RAWDATA_REPEAT = 1,
  MY_IR_RAWDATA_NOTHING = 2,
  MY_IR_RAWDATA_ERR = -1,
};
struct my_ir_rawdata {
  unsigned int raw_data;
  int edge_cnt;
  u64 edges_timestamp[EDGE_MAX];
};

struct my_ir_rawdata g_my_ir_rawdata = {
    .raw_data = 0,
    .edge_cnt = 0,
    .edges_timestamp = {0},
};

int is_in_range(u64 ns_value, u64 ns_min, u64 ns_max) {
  return (ns_value >= ns_min && ns_value <= ns_max);
}

void my_ir_rawdata_save_edge(struct my_ir_rawdata *raw_data) {
  if (raw_data->edge_cnt + 1 >= EDGE_MAX) {
    my_ir_err("Edge count exceeded maximum limit of %d\n", EDGE_MAX);
    return;
  }

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0))
  raw_data->edges_timestamp[raw_data->edge_cnt++] = ktime_get_boottime_ns();
#else
  raw_data->edges_timestamp[raw_data->edge_cnt++] = ktime_get_boot_ns();
#endif

  if (raw_data->edge_cnt >= 2) {
    // timeout
    if (raw_data->edges_timestamp[raw_data->edge_cnt - 1] -
            raw_data->edges_timestamp[raw_data->edge_cnt - 2] >
        TIMEOUT_NS) {
      raw_data->edges_timestamp[0] =
          raw_data->edges_timestamp[raw_data->edge_cnt - 1];
      raw_data->edge_cnt = 1;
      return;
    }
  }
}

void my_ir_rawdata_reset(struct my_ir_rawdata *raw_data) {
  int i;
  for (i = 0; i < EDGE_MAX; i++) {
    raw_data->edges_timestamp[i] = 0;
  }
  // raw_data->raw_data = 0;
  raw_data->edge_cnt = 0;
}

enum MY_IR_RAWDATA_RES my_ir_rawdata_parse(struct my_ir_rawdata *raw_data) {
  u64 head_diff = 0, tail_diff = 0, edges_timestamp_idx1 = 0,
      edges_timestamp_idx2 = 0;
  const int data_cnt = 4;
  unsigned char data[data_cnt];
  int data_idx = 0, bit_idx = 0;
  memset(data, 0, data_cnt);
  if (raw_data->edge_cnt < 4) {
    return MY_IR_RAWDATA_NOTHING;
  }

  head_diff = raw_data->edges_timestamp[1] - raw_data->edges_timestamp[0];
  tail_diff = raw_data->edges_timestamp[2] - raw_data->edges_timestamp[1];

  // repeat
  if (is_in_range(head_diff, HEAD_DIFF_NS - TOR_NS, HEAD_DIFF_NS + TOR_NS) &&
      is_in_range(tail_diff, TAIL_REP_DIFF_NS - TOR_NS,
                  TAIL_REP_DIFF_NS + TOR_NS)) {
    return MY_IR_RAWDATA_REPEAT;
  }

  // data
  if (raw_data->edge_cnt >= 68) {

    for (data_idx = 0; data_idx < data_cnt; ++data_idx) {
      edges_timestamp_idx1 = 3 + data_idx * 16;
      edges_timestamp_idx2 = 4 + data_idx * 16;
      for (bit_idx = 0; bit_idx < 8; ++bit_idx) {
        tail_diff = raw_data->edges_timestamp[edges_timestamp_idx2] -
                    raw_data->edges_timestamp[edges_timestamp_idx1];
        if (is_in_range(tail_diff, TAIL_VAL_1_NS - TOR_NS,
                        TAIL_VAL_1_NS + TOR_NS)) {
          data[data_idx] |= (1 << bit_idx); // bit is 1
        }
        edges_timestamp_idx1 += 2;
        edges_timestamp_idx2 += 2;
      }
    }

    // check data integrity
    data[1] = ~data[1];
    data[3] = ~data[3];
    if (data[0] != data[1] || data[2] != data[3]) {
      my_ir_err("Data integrity check failed: %02x:%02x:%02x:%02x\n", data[0],
                data[1], data[2], data[3]);
      return MY_IR_RAWDATA_ERR;
    }

    raw_data->raw_data = (data[0] << 8) | data[2];
    return MY_IR_RAWDATA_DATA;
  }

  return MY_IR_RAWDATA_NOTHING;
}

static irqreturn_t my_ir_isr(int irq, void *dev_id) {
  int keycode = -1;
  enum MY_IR_RAWDATA_RES res;
  my_ir_rawdata_save_edge(&g_my_ir_rawdata);
  res = my_ir_rawdata_parse(&g_my_ir_rawdata);
  switch (res) {
  case MY_IR_RAWDATA_DATA:
    my_ir_info("Received data: %02x\n", g_my_ir_rawdata.raw_data);
    my_ir_rawdata_reset(&g_my_ir_rawdata);
    keycode = find_keycode_from_scancode(g_my_ir_rawdata.raw_data);
    if (keycode >= 0) {
      input_event(g_ir_input_dev, EV_KEY, keycode, 1);
      input_event(g_ir_input_dev, EV_KEY, keycode, 0);
      input_sync(g_ir_input_dev);
    }
    break;
  case MY_IR_RAWDATA_REPEAT:
    my_ir_info("Received repeat data: %02x\n", g_my_ir_rawdata.raw_data);
    my_ir_rawdata_reset(&g_my_ir_rawdata);
    keycode = find_keycode_from_scancode(g_my_ir_rawdata.raw_data);
    if (keycode >= 0) {
      input_event(g_ir_input_dev, EV_KEY, keycode, 1);
      input_event(g_ir_input_dev, EV_KEY, keycode, 0);
      input_sync(g_ir_input_dev);
    }
    break;
  case MY_IR_RAWDATA_ERR:
    my_ir_err("Error parsing raw data\n");
    my_ir_rawdata_reset(&g_my_ir_rawdata);
    break;
  case MY_IR_RAWDATA_NOTHING:
    break;
  }

  return IRQ_HANDLED;
}

static const struct of_device_id my_ir_input_dt_match[] = {
    {
        .compatible = "my_ir_input_driver",
    },
    {},
};

static int my_ir_input_probe(struct platform_device *pdev) {
  int ret = -1, i = 0;
  my_ir_info("probe...\n");

  g_ir_gpio_pin = gpiod_get(&pdev->dev, "my", 0);
  if (IS_ERR(g_ir_gpio_pin))
    my_ir_err("%s %s line %d\n", __FILE__, __FUNCTION__, __LINE__);

  g_irq = gpiod_to_irq(g_ir_gpio_pin);

  ret =
      request_irq(g_irq, my_ir_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                  "my_ir_isr_name", NULL);
  if (ret) {
    my_ir_err("Failed to request IRQ %d for GPIO %d\n", g_irq,
              desc_to_gpio(g_ir_gpio_pin));
    gpiod_put(g_ir_gpio_pin);
    return ret;
  }

  // allocate input device
  g_ir_input_dev = devm_input_allocate_device(&pdev->dev);

  // set input device properties
  g_ir_input_dev->name = "My IR Input Device Name";
  g_ir_input_dev->phys = "my_ir_input/input0";
  __set_bit(EV_KEY, g_ir_input_dev->evbit); // enable key events
  __set_bit(EV_REP, g_ir_input_dev->evbit); // enable repeat events

  // set key mappings
  for (i = 0; i < ARRAY_SIZE(hs0038_nec_map); i++) {
    __set_bit(hs0038_nec_map[i].keycode, g_ir_input_dev->keybit);
  }

  // register the input device
  ret = input_register_device(g_ir_input_dev);
  if (ret) {
    my_ir_err("Failed to register input device\n");
    free_irq(g_irq, NULL);
    gpiod_put(g_ir_gpio_pin);
    return ret;
  }

  return ret;
}

static int my_ir_input_remove(struct platform_device *pdev) {
  my_ir_info("remove...\n");
  free_irq(g_irq, NULL);
  gpiod_put(g_ir_gpio_pin);
  return 0;
}

static struct platform_driver g_my_ir_input_driver = {
    .driver =
        {
            .name = DRIVER_NAME,
            .of_match_table = my_ir_input_dt_match,
        },
    .probe = my_ir_input_probe,
    .remove = my_ir_input_remove,
};

static int __init my_ir_input_ini(void) {
  int ret = -1;

  my_ir_info("Initializing...\n");

  ret = platform_driver_register(&g_my_ir_input_driver);
  if (ret) {
    my_ir_err("platform_driver_register failed!\n");
    return -ENODEV;
  }

  my_ir_info("Initializing done\n");
  return ret;
}

static void __exit my_ir_input_exit(void) {
  my_ir_info("Exiting...\n");
  platform_driver_unregister(&g_my_ir_input_driver);
  my_ir_info("Exit done\n");
}

module_init(my_ir_input_ini);
module_exit(my_ir_input_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yifan");
MODULE_DESCRIPTION("My GPIO Driver");