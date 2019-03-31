#include "kernel.c"
#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define DIRS_ENTRY_LENGTH 16

void main(){
  char name[512];
  char dirs[SECTOR_SIZE];
  char files[SECTOR_SIZE];
  int i;
  int parentIndex;

  interrupt(0x21,0x02, dirs, DIRS_SECTOR, 0);
  interrupt(0x21,0x02, files, FILES_SECTOR, 0);
  interrupt(0x21,0x21, parentIndex, 0, 0);

  i = 0;
  while(i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
    if(dirs[i*DIRS_ENTRY_LENGTH] == parentIndex){
      interrupt(0x21, 0x00, dirs+i*DIRS_ENTRY_LENGTH+1, 0, 0);
      interrupt(0x21, 0x00, "\n\r", 0 , 0);
    }
    i++;
  }

  i = 0;
  while(i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
    if(files[i*DIRS_ENTRY_LENGTH] == parentIndex){
      interrupt(0x21, 0x00, files+i*DIRS_ENTRY_LENGTH+1, 0, 0);
      interrupt(0x21, 0x00, "\n\r", 0 , 0);
    }
    i++;
  }

}
