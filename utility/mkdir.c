void main(){
  char argv[512];
  char curdir;
  char parent;
  int result;
  enableInterrupts();

  interrupt(0x21, 0x23, 0, argv, 0);
  interrupt(0x21, 0x21, &curdir, 0, 0);
  interrupt(0x21, (curdir << 8) | 0x08, argv, &result, 0);
  if (result != 0){
    interrupt(0x21, 0x0, "Failed to make directory",0,0);
  }
  
  interrupt(0x21, 0x07, 0, 0, 0);
}
