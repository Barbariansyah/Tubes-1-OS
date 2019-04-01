#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define DIRS_ENTRY_LENGTH 16
#define MAX_DIRNAME 15
#define MAX_DIRS 32
#define MAX_FILES 32
#define MAX_SECTORS 16
#define FALSE 0
#define TRUE 1

void clear(char *buffer, int length);
void copyString(char *dest, char *src, int len);

void main(){
  char argv[16][SECTOR_SIZE];
  char readbuffer[SECTOR_SIZE*MAX_SECTORS];
  char parentIndex, argc;
  char filename[SECTOR_SIZE];
  int i, j, result;

  i = 0;
  j = 0;
  interrupt(0x21,0x21, &parentIndex, 0, 0);
  interrupt(0x21, 0x22, &argc, 0, 0);
  interrupt(0x21, 0x23, 0, argv[0], 0);
  interrupt(0x21, 0x23, 1, argv[1], 0);
  copyString(filename,argv[0],SECTOR_SIZE);
  if (argc == 1){
    //Read file
    interrupt(0x21, (parentIndex << 8) | 0x04, readbuffer, argv[0], &result);
    if (result != 0){
      interrupt(0x21, 0x0, "Failed to read file\n\r", 0, 0);
    }else{
      interrupt(0x21, 0x0, readbuffer, 0, 0);
      interrupt(0x21, 0x0, "\n\r", 0, 0);
    }
  }else if (argc == 2 && argv[1][1] == 'w'){
    //Write file
    result = 1;
    interrupt(0x21, 0x0, "Enter your text:", 0, 0);
    interrupt(0x21, 0x1, readbuffer, 0, 0);
    interrupt(0x21, (parentIndex << 8) | 0x09, filename, 0, 0);
    interrupt(0x21, (parentIndex << 8) | 0x05, readbuffer, filename, &result);
    if (result <= 0){
      interrupt(0x21, 0x0, "Failed to write file\n\r", 0, 0);
    }else{
      interrupt(0x21, 0x0, "Success write file\n\r", 0, 0);
    }
  }

  interrupt(0x21, 0x07, 0, 0, 0);
}

void clear(char *buffer, int length) {
   int i;
   for(i = 0; i < length; ++i) {
      buffer[i] = '\0';
   }
}

void copyString(char *dest, char *src, int len){
   int i;
   clear(dest,len);
   for(i = 0; i < len; i++){
      if (src[i] == '-') break;
      else dest[i] = src[i];
   }
}
