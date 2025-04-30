/* Usage:
 * ./my_eeprom_app <w/r> <char node> <address offset> [<count>] [<characters>]
 * ex:
 * ./my_eeprom_app w /dev/my_eeprom 0x00 'abcde'
 * ./my_eeprom_app r /dev/my_eeprom 0x00 5
 * abcde
 */

#include "my_eeprom_ioctl.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAX_EEPROM_SIZE 256

int main(int argc, char *argv[]) {
  if (argc < 5) {
    fprintf(stderr,
            "Usage: %s <w/r> <char node> <address offset> [<count>] "
            "[<characters>]\n",
            argv[0]);
    return 1;
  }
  printf("argc = %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }

  char *operation = argv[1];
  char *char_node = argv[2];
  unsigned int address_offset = strtoul(argv[3], NULL, 0);
  char sz_input[MAX_EEPROM_SIZE] = {0};
  int fd = open(char_node, O_RDWR);
  if (fd < 0) {
    perror("open");
    return 1;
  }

  struct my_eeprom_data eeprom_data;
  eeprom_data.offset = address_offset;
  eeprom_data.count = 0;
  eeprom_data.buf = NULL;

  // write operation
  if (strcmp(operation, "w") == 0) {
    eeprom_data.count = strlen(argv[4]);
    if (eeprom_data.count > MAX_EEPROM_SIZE) {
      fprintf(stderr, "Count exceeds maximum EEPROM size\n");
      close(fd);
      return 1;
    }
    strncpy(sz_input, argv[4], eeprom_data.count);
    eeprom_data.buf = (uint8_t *)sz_input;

    if (ioctl(fd, MY_EEPROM_WRITE, &eeprom_data) < 0) {
      perror("ioctl write");
      close(fd);
      return 1;
    }
  } else if (strcmp(operation, "r") == 0) { // read operation
    eeprom_data.count = strtoul(argv[4], NULL, 0);
    if (eeprom_data.count > MAX_EEPROM_SIZE) {
      fprintf(stderr, "Count exceeds maximum EEPROM size\n");
      close(fd);
      return 1;
    }
    eeprom_data.buf = (uint8_t *)malloc(eeprom_data.count);
    if (!eeprom_data.buf) {
      perror("malloc");
      close(fd);
      return 1;
    }

    if (ioctl(fd, MY_EEPROM_READ, &eeprom_data) < 0) {
      perror("ioctl read");
      free(eeprom_data.buf);
      close(fd);
      return 1;
    }

    printf("Read data: ");
    for (unsigned int i = 0; i < eeprom_data.count; i++) {
      printf("%c", eeprom_data.buf[i]);
    }
    printf("\n");

    free(eeprom_data.buf);
  } else {
    fprintf(stderr, "Invalid operation. Use 'w' for write or 'r' for read.\n");
  }
  return 0;
}