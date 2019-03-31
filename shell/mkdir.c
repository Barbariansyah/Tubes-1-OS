#include "kernel.c"

void main(){
  char argv[512];
  char curdir;
  char parent;
  interrupt(0x21, 0x23, 1, argv, 0);
  interrupt(0x21, 0x21, curdir, 0, 0);
  interrupt(0x21, 0x08, argv, 0, curdir);
}
