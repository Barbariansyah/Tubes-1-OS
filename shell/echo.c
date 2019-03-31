#include "kernel.c"

void main(){
  int i;
  char argc;
  char argv[512];

  interrupt(0x21, 0x22, &argc, 0, 0);
  for(i = 0; i <argc; ++i){
    interrupt(0x21, 0x23, i, argv, 0);
    interrupt(0x21, 0x00, argv, 0, 0);
  }

  interrupt(0x21,0x00, "\n\r", 0, 0);
}
