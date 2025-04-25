#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
  int flag = 0, val = 0, read_val = 0, val_size = 4;
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

  // Get flag
  flag = fcntl(fd, F_GETFL);
  if (flag < 0) {
    printf("Failed to get file status flags\n");
    close(fd);
    return -1;
  }
  // Set Non-blocking
  // flag |= O_NONBLOCK;
  // Comment out set blocking
  // flag &= ~O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flag) < 0) {
    printf("Failed to set file status flags\n");
    close(fd);
    return -1;
  }

  while (1) {
    read_val = read(fd, &val, val_size);
    if (read_val == val_size) {
      printf("read_val = %d\n", read_val);
    } else {
      printf("read none, read_val = %d\n", read_val);
      sleep(1);
    }
  }
}