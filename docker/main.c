#include <stdio.h>
#include <threads.h>

int main()
{
  int i = 10;
  while (i--) {
    thrd_sleep(&(struct timespec) {.tv_sec = 1}, NULL);
    for (int j = 0; j < 8192; ++j) fputc('*', stdout);//eat some buffer
    const int rc = puts("hello stdout");
    if(EOF == rc)
      perror("puts()");
  }
}
