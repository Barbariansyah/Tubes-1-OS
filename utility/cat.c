#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define DIRS_ENTRY_LENGTH 16
#define MAX_DIRNAME 15
#define MAX_DIRS 32
#define MAX_FILES 32
#define FALSE 0
#define TRUE 1

int isEqualPathName(char *path1, char *path2){
   int i;
   for(i = 0; i < MAX_DIRNAME; i++){
      if (path1[i] != path2[i]){
         return FALSE;
      }else if(path1[i]=='\0' && path2[i]=='\0' && i!=0){
         return TRUE;
      }
   }
   return TRUE;
}

int main(){
  char argv[512];
  char dirs[SECTOR_SIZE];
  char files[SECTOR_SIZE];
  int i, j;
  int isEqual;
  char parentIndex;

  i = 0;
  j = 0;
  isEqual = 0;
  interrupt(0x21, 0x00, "Cat", 0, 0);
  interrupt(0x21,0x02, dirs, DIRS_SECTOR, 0);
  interrupt(0x21,0x02, files, FILES_SECTOR, 0);
  interrupt(0x21,0x21, &parentIndex, 0, 0);
  interrupt(0x21, 0x23, 0, argv, 0);
  interrupt(0x21, 0x00, argv, 0, 0);


  while(j*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
    isEqual = isEqualPathName(argv, dirs + j*DIRS_ENTRY_LENGTH +1);

    if (isEqual && dirs[j*DIRS_ENTRY_LENGTH] == parentIndex){
      interrupt(0x21, 0x00, "cat: ", 0, 0);
      interrupt(0x21, 0x00, argv, 0, 0);
      interrupt(0x21, 0x00, ": Is a directory", 0, 0);
      interrupt(0x21, 0x00, "\n\r", 0, 0);
      return;
    }
    else{
          j++;
    }
  }

  while(i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
    isEqual = isEqualPathName(argv, files + i*DIRS_ENTRY_LENGTH +1);

    if (isEqual && files[i*DIRS_ENTRY_LENGTH] == parentIndex){
         interrupt(0x21, 0x06, parentIndex, 0, 0);
         break;
    }
    else{
         i++;
    }
  }

  if (i == MAX_FILES && j == MAX_DIRS){
      interrupt(0x21, 0x00, "cat: ", 0, 0);
      interrupt(0x21, 0x00, argv, 0, 0);
      interrupt(0x21, 0x00, ": No such file or directory", 0, 0);
      interrupt(0x21, 0x00, "\n\r", 0, 0);
      return;
  }

}
