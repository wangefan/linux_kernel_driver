#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>


static int g_fd;
// prepare signal handler
static void signal_handler(int sig_num) {
  int b_key_pressed = 0;
  read(g_fd, &b_key_pressed, 4);
  printf("Signal received pressed:%d\n", b_key_pressed);
}

int main(int argc, char **argv) {
  int ret, sleep_time = 2000; // ms

  if (argc != 2) {
    printf("Usage: %s <dev>\n", argv[0]);
    return -1;
  }

  // open
  g_fd = open(argv[1], O_RDWR);
  if (g_fd < 0) {
    printf("Failed to open %s\n", argv[1]);
    return -1;
  }

  // register signal
  signal(SIGIO, signal_handler);

  // set app pid to fd
  ret = fcntl(g_fd, F_SETOWN, getpid());
  if (ret < 0) {
    printf("Failed to set owner\n");
    return -1;
  }

  // set to fasync
  int of_flags = fcntl(g_fd, F_GETFL);
  if (of_flags < 0) {
    printf("Failed to get flags\n");
    return -1;
  }

  // This cause drv_fasync in driver to be called
  ret = fcntl(g_fd, F_SETFL, of_flags | O_ASYNC);
  if (ret < 0) {
    printf("Failed to set async\n");
    return -1;
  }

  while (1) {
    printf("Now sleeping for %d ms\n", sleep_time);
    usleep(sleep_time * 1000);
  }
}