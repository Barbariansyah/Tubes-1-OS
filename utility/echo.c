int main(){
  int i;
  char argc;
  char argv[16];

  interrupt(0x21, 0x22, &argc, 0, 0);
  for(i = 0; i < argc; ++i){
    interrupt(0x21, 0x23, i, argv, 0);
    interrupt(0x21, 0x00, argv, 0, 0);
    interrupt(0x21, 0x00, " ",0,0);
  }

  interrupt(0x21,0x00, "\n\r", 0, 0);
  interrupt(0x21, 0x07, 0, 0, 0);
}
