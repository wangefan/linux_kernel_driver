#include <fcntl.h>
#include <poll.h>
#include <stdio.h>

int main(int argc, char **argv) {
  int ret, timeout;
  int b_key_pressed = 0;
  struct pollfd fds[1];

  if (argc != 2) {
    printf("Usage: %s <dev>\n", argv[0]);
    return -1;
  }

  // open
  int fd = open(argv[1], O_RDWR);
  if (fd < 0) {
    printf("Failed to open %s\n", argv[1]);
    return -1;
  }

  // now we have fd to poll
  fds[0].fd = fd;
  fds[0].events = POLLIN; // There is data to read.
  timeout = 2000;         // ms

  while (1) {
    ret = poll(fds, 1, timeout);
    if (ret == 0) {
      printf("Timeout\n");
      continue;
    } else if (ret < 0) {
      printf("poll error\n");
      break;
    }

    // now we have data to read
    if (ret > 0 && fds[0].revents & POLLIN) {
      read(fd, &b_key_pressed, 4);
      printf("Data available to read, count: %d, b_key_pressed:%d\n", ret, b_key_pressed);
    } else {
      printf("Unexpected event\n");
      continue;
    }
  }
}