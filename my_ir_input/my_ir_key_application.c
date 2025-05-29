#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * 簡易 keycode -> 名稱 對照表
 */
const char *keycode_to_string(unsigned int code) {
  switch (code) {
  case KEY_POWER:
    return "KEY_POWER";
  case KEY_MENU:
    return "KEY_MENU";
  case KEY_BACK:
    return "KEY_BACK";
  case KEY_VOLUMEUP:
    return "KEY_VOLUMEUP";
  case KEY_VOLUMEDOWN:
    return "KEY_VOLUMEDOWN";
  case KEY_PLAYPAUSE:
    return "KEY_PLAYPAUSE";
  case KEY_LAST:
    return "KEY_LAST";
  case KEY_NEXT:
    return "KEY_NEXT";
  case KEY_0:
    return "KEY_0";
  case KEY_1:
    return "KEY_1";
  case KEY_2:
    return "KEY_2";
  case KEY_3:
    return "KEY_3";
  case KEY_4:
    return "KEY_4";
  case KEY_5:
    return "KEY_5";
  case KEY_6:
    return "KEY_6";
  case KEY_7:
    return "KEY_7";
  case KEY_8:
    return "KEY_8";
  case KEY_9:
    return "KEY_9";
  case KEY_C:
    return "KEY_C";
  default:
    return "UNKNOWN_KEY";
  }
}

int main(int argc, char **argv) {
  int fd;
  struct input_event data;

  if (argc != 2) {
    printf("Usage: %s <dev>\n", argv[0]);
    return -1;
  }

  fd = open(argv[1], O_RDWR);
  if (fd == -1) {
    printf("Cannot open file %s\n", argv[1]);
    return -1;
  }

  while (1) {
    if (read(fd, &data, sizeof(data)) == sizeof(data)) {
      if (data.type == EV_KEY) {
        printf("IR Key Event:\n");
        printf(" Code: 0x%x (%s)\n", data.code, keycode_to_string(data.code));
        printf(" Value: %d (%s)\n", data.value,
               data.value ? (data.value == 2 ? "HOLD" : "PRESS") : "RELEASE");
      }
    }
    // 若想避免過多輸出，可加入 sleep(1);
  }

  close(fd);
  return 0;
}
