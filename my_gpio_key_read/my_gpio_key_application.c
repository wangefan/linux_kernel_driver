#include <stdio.h>
#include <fcntl.h>

int main(int argc, char **argv) {
  int fg, val;
  if(argc != 2) {
    printf("Usage: %s <dev>\n", argv[0]);
    return -1;
  }

  // open
  int fd = open(argv[1], O_RDWR);
  if (fd < 0) {
    printf("Failed to open %s\n", argv[1]);
    return -1;
  }
  
  while(1) {
    read(fd, &val, 4);
    printf("val = %d\n", val);
  } 
}