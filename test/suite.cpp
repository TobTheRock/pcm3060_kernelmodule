#include <stdio.h>
#include "fs_dev.hpp"

int main()
{
   /* Our first simple C basic program */
   printf("Hello World! ");
   return openDevice_write_and_read();
}