#include "kernel.c"

void main(){
  char argv[512];
  char dirs[SECTOR_SIZE];
  char files[SECTOR_SIZE];
  int i, j;
  int isEqual;
  char parentIndex;

  i = 0;
  j = 0;
  isEqual = 0;
  
  interrupt(0x21,0x02, dirs, DIRS_SECTOR, 0);
  interrupt(0x21,0x02, files, FILES_SECTOR, 0);
  interrupt(0x21,0x21, parentIndex, 0, 0);
  interrupt(0x21, 0x23, 0, argv, 0);

  while(i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
    isEqual = isEqualPathName(argv, files + i*DIRS_ENTRY_LENGTH +1);

    if (isEqual && files[i*DIRS_ENTRY_LENGTH] == parentIndex){
         interrupt(0x21, 0x09, argv, 0, parentIndex);
         break;
    }
    else{
         i++;
    }
  }

  while(j*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
    isEqual = isEqualPathName(argv, dirs + j*DIRS_ENTRY_LENGTH +1);

    if (isEqual && dirs[j*DIRS_ENTRY_LENGTH] == parentIndex){
         interrupt(0x21, 0x0A, argv, 0, parentIndex);
         break;
    }
    else{
         j++;
    }
  }

  if (i == MAX_FILES && j == MAX_DIRS){
      interrupt(0x21, 0x00, "rm: cannot remove \'", 0, 0);
      interrupt(0x21, 0x00, argv, 0, 0);
      interrupt(0x21, 0x00, "\': No such file or directory", 0, 0);
      interrupt(0x21, 0x00, "\n\r", 0, 0);
      return;
  }


}
