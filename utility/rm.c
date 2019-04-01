#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define DIRS_ENTRY_LENGTH 16
#define MAX_DIRNAME 15
#define MAX_DIRS 32
#define MAX_FILES 32
#define FALSE 0
#define TRUE 1

void copyString(char *dest, char *src, int len);

void main(){
  char argv[SECTOR_SIZE];
  char parentIndex;
  char fileName[SECTOR_SIZE];
  char dirName[SECTOR_SIZE];
  int resultDir;
  int resultFile;

  interrupt(0x21, 0x21, &parentIndex, 0, 0);
  interrupt(0x21, 0x23, 0, argv, 0);
  copyString(fileName,argv,SECTOR_SIZE);
  copyString(dirName,argv,SECTOR_SIZE);

  interrupt(0x21, (parentIndex << 8) | 0x09, fileName, &resultFile, 0);
  interrupt(0x21, (parentIndex << 8) | 0x0A, dirName, &resultDir, 0);

  if(resultDir == -1 && resultFile == -1){
    interrupt(0x21, 0x00, "rm: cannot remove \'", 0, 0);
    interrupt(0x21, 0x00, fileName, 0, 0);
    interrupt(0x21, 0x00, "\': No such file or directory", 0, 0);
    interrupt(0x21, 0x00, "\n\r", 0, 0);
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
