#define MAX_BYTE 256
#define SECTOR_SIZE 512

#define MAX_DIRS 32
#define MAX_FILES 32
#define MAX_DIRNAME 15
#define MAX_FILENAME 15
#define MAX_SECTORS 16
#define MAX_PATH_LENGTH 255
#define DIRS_ENTRY_LENGTH 16

#define MAP_SECTOR 256
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define SECTORS_SECTOR 259
#define ARGS_SECTOR 512

#define TRUE 1
#define FALSE 0

#define INSUFFICIENT_SECTORS 0
#define SUCCESS 0
#define NOT_FOUND -1
#define ALREADY_EXIST -2
#define INSUFFICIENT_SEGMENTS -2
#define INSUFFICIENT_ENTRIES -3

#define EMPTY 0x00
#define USED 0xFF

#define SEGMENT0 0x2000
#define SEGMENT1 0x3000
#define SEGMENT2 0x4000
#define SEGMENT3 0x5000
#define SEGMENT4 0x6000
#define SEGMENT5 0x7000
#define SEGMENT6 0x8000
#define SEGMENT7 0x9000

#define MAIN
#include "proc.h" 

void handleInterrupt21 (int AX, int BX, int CX, int DX);
void handleTimerInterrupt(int segment, int stackPointer);
void printString(char *string);
void readString(char *string, int disableProcessControls);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram (char *path, int *result, char parentIndex);
void terminateProgram (int *result);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);
void putArgs (char curdir, char argc, char **argv);
void getCurdir (char *curdir);
void getArgc (char *argc);
void getArgv (char index, char *argv);
void yieldControl ();
void sleep ();
void pauseProcess (int segment, int *result);
void resumeProcess (int segment, int *result);
void killProcess (int segment, int *result);

void clear(char *buffer, int length);
int isEqualPathName(char *path1, char *path2);
void relPathToAbsPath(char *path, char *parentIndex, int *success);
void deleteFileByParentId(char parentIndex);
void deleteDirectoryByParentId(char parentIndex);
void printPCB(int segment);

int main() {
   //interrupt(0x21, (AH << 8) | AL, BX, CX, DX);
   char shell[6];
   shell[0] = 's';
   shell[1] = 'h';
   shell[2] = 'e';
   shell[3] = 'l';
   shell[4] = 'l';
   shell[5] = '\0';

   initializeProcStructures();
   makeInterrupt21();
   makeTimerInterrupt();
   interrupt(0x21, 0x00, "ElPrimOS v0.3\r\n", 0, 0);
   interrupt(0x21, (0xFF << 8) | 0x06, shell, 0, 0);
   while (1);
}

void handleInterrupt21 (int AX, int BX, int CX, int DX) {
   char AL, AH;
   AL = (char) (AX);
   AH = (char) (AX >> 8);
   switch (AL) {
      case 0x00:
         printString(BX);
         break;
      case 0x01:
         readString(BX, CX);
         break;
      case 0x02:
         readSector(BX, CX);
         break;
      case 0x03:
         writeSector(BX, CX);
         break;
      case 0x04:
         readFile(BX, CX, DX, AH);
         break;
      case 0x05:
         writeFile(BX, CX, DX, AH);
         break;
      case 0x06:
         executeProgram(BX, CX, AH);
         break;
      case 0x07:
         terminateProgram(BX);
         break;
      case 0x08:
         makeDirectory(BX, CX, AH);
         break;
      case 0x09:
         deleteFile(BX, CX, AH);
         break;
      case 0x0A:
         deleteDirectory(BX, CX, AH);
         break;
      case 0x20:
         putArgs(BX, CX, DX);
         break;
      case 0x21:
         getCurdir(BX);
         break;
      case 0x22:
         getArgc(BX);
         break;
      case 0x23:
         getArgv(BX, CX);
         break;
      case 0x30:
         yieldControl ();
         break;
      case 0x31:
         sleep ();
         break;
      case 0x32:
         pauseProcess (BX, CX);
         break;
      case 0x33:
         resumeProcess (BX, CX);
         break;
      case 0x34:
         killProcess (BX, CX);
         break;
      default:
         printString("Invalid interrupt");
   }
}

void handleTimerInterrupt(int segment, int stackPointer) {
  struct PCB *currPCB;
  struct PCB *nextPCB;

  setKernelDataSegment();

  currPCB = getPCBOfSegment(segment);
  currPCB->stackPointer = stackPointer;
  if (currPCB->state != PAUSED) {
    currPCB->state = READY;
    addToReady(currPCB);
  }

  do {
    nextPCB = removeFromReady();
  }
  while (nextPCB != NULL && (nextPCB->state == DEFUNCT || nextPCB->state == PAUSED));

  if (nextPCB != NULL) {
    nextPCB->state = RUNNING;
    segment = nextPCB->segment;
    stackPointer = nextPCB->stackPointer;
    running = nextPCB;
  } else {
    running = &idleProc;
  }

  restoreDataSegment();
  returnFromTimer(segment, stackPointer);
}

void printString(char *string){
   int i = 0;
   while (string[i] != '\0'){
      interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
      i++;
   }
}

void readString(char *string, int disableProcessControls){
   char c = interrupt(0x16, 0, 0, 0, 0);
   int i = 0;
   while (c != '\r'){
      if (c == '\n'){
         string[i] = c;
         i++;
         interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
      }else if (c == '\b' && i > 0){
         string[i] = '\0';
         i--;
         interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
         interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
         interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
      }else if (c == 0x03 && !disableProcessControls){ //Ctrl + C
         terminateProgram(0);
      }else if (c == 0x1A && !disableProcessControls){ //Ctrl + Z
         sleep();
         resumeProcess(SEGMENT0, 0);
      }else if (c != '\b'){ //Karakter lain
         string[i] = c;
         i++;
         interrupt(0x10, 0xE00 + c, 0, 0, 0);
      }
      c = interrupt(0x16, 0, 0, 0, 0);
   }
   string[i] = '\0';
   interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
   interrupt(0x10, 0xE00 + '\r', 0, 0, 0);
}

int mod(int a, int b) {
   while(a >= b) {
      a = a - b;
   }
   return a;
}

int div(int a, int b) {
   int q = 0;
   while(q*b <= a) {
      q = q+1;
   }
   return q-1;
}

void clear(char *buffer, int length) {
   int i;
   for(i = 0; i < length; ++i) {
      buffer[i] = EMPTY;
   }
}

void readSector(char *buffer, int sector){
   interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector){
   interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void relPathToAbsPath(char *path, char *parentIndex, int *success){
   char dirs[SECTOR_SIZE];
   char currpath[MAX_FILENAME];
   int i, j, k, isEqual, isLastPathDone, isDirnameDone;
   j = 0;   //Variabel untuk menghitung panjang path yang sudah dibaca
   isLastPathDone = FALSE;
   
   readSector(dirs, DIRS_SECTOR);
   //Traversal setiap folder di path 
   while (!isLastPathDone){
      //Menyimpan nama folder saat ini di currpath
      clear(currpath,MAX_FILENAME);
      k = 0;
      isDirnameDone = FALSE;
      do{
         if (path[j+k] == '/'){
            isDirnameDone = TRUE;
         }else if (path[j+k] == '\0'){
            isDirnameDone = TRUE;
            isLastPathDone = TRUE;
         }else{  
            currpath[k] = path[j+k];
         }
         k++;
      }while(k < MAX_DIRNAME && !isDirnameDone);
      j += k;

      if (!isLastPathDone){
         //Traversal dirs untuk mencari folder currpath
         i = 0;   //Variabel untuk traversal sektor dirs
         while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
            isEqual = isEqualPathName(currpath,dirs+i*DIRS_ENTRY_LENGTH+1);
            if (isEqual && dirs[i*DIRS_ENTRY_LENGTH] == *parentIndex){
               *parentIndex = i;
               break;
            }else{
               i++;
            }
         }
         //Folder currpath tidak ditemukan
         if (i == MAX_DIRS){
            *success = NOT_FOUND;
            return;
         }   
      }
   }
   //Menandakan success
   *success = 0;
   for(i = 0; i < MAX_FILENAME; i++){
      path[i] = currpath[i];
   }
}

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

void copyString(char *dest, char *src, int len){
   int i;
   clear(dest,len);
   for(i = 0; i < len; i++){
      if (src[i] == '\0') break;
      else dest[i] = src[i];
   }
}

void readFile(char *buffer, char *path, int *result, char parentIndex){
   char files[SECTOR_SIZE];
   char sectors[SECTOR_SIZE];
   char temppath[SECTOR_SIZE];
   int i,isEqual,success;

   //Mengubah relative path menjadi path absolute dengan parent index yang sesuai
   copyString(temppath,path,SECTOR_SIZE);
   relPathToAbsPath(temppath, &parentIndex, &success);
   if (success != 0){
      *result = success;
      return;
   }
   
   
   readSector(files, FILES_SECTOR);
   i = 0;   //Variabel untuk traversal sektor files
   while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
      isEqual = isEqualPathName(temppath,files+i*DIRS_ENTRY_LENGTH+1);
      if (isEqual && files[i*DIRS_ENTRY_LENGTH] == parentIndex){
         parentIndex = i;
         break; 
      }else{
         i++;
      }
   }
   //File currpath tidak ditemukan
   if (i == MAX_FILES){
      *result = NOT_FOUND;
      return;
   }
   
   //Membaca isi sector file ke buffer
   //Parent index berisi index file
   readSector(sectors, SECTORS_SECTOR);
   i = 0;
   do{
      readSector(buffer + i*SECTOR_SIZE, sectors[i+parentIndex*MAX_SECTORS]);
      i++;
   }while (i < MAX_SECTORS && sectors[i+parentIndex*MAX_SECTORS] != '\0');
   *result = parentIndex;
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex){
   char map[SECTOR_SIZE];
   char dirs[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char filesect[SECTOR_SIZE];
   char sectorBuffer[SECTOR_SIZE];
   char inpath[MAX_PATH_LENGTH];
   int i,j,k,isEqual,sectorCount,fileIndex,success;

   copyString(inpath,path,MAX_PATH_LENGTH);
   //Menentukan apakah sisa sector masih tersedia untuk menampung file baru
   readSector(map, MAP_SECTOR);
   for (i = 0, sectorCount = 0; i < SECTOR_SIZE && sectorCount < *sectors; ++i) {
      if (map[i] == EMPTY) {
         ++sectorCount;
      }
   }

   if (sectorCount < *sectors) {
      *sectors = INSUFFICIENT_SECTORS;
      return;
   }
   
   //Mengubah relative path menjadi path absolute dengan parent index yang sesuai
   relPathToAbsPath(inpath, &parentIndex, &success);
   if (success != 0){
      *sectors = success;
      return;
   }
   
   //Traversal sector files untuk mencari apakah file currpath sudah ada
   //Jika belum, maka kembalikan index kosong pertama pada sektor files
   readSector(files, FILES_SECTOR);
   fileIndex = 0;   //Variabel untuk index pada sector files
   while (fileIndex*DIRS_ENTRY_LENGTH < SECTOR_SIZE && files[fileIndex*DIRS_ENTRY_LENGTH+1] != '\0'){
      isEqual = isEqualPathName(inpath,files+fileIndex*DIRS_ENTRY_LENGTH+1);
      if (isEqual && files[fileIndex*DIRS_ENTRY_LENGTH] == parentIndex){
         *sectors = ALREADY_EXIST;
         return;
      }else{
         fileIndex++;
      }
   }
   if (fileIndex == MAX_FILES){
      *sectors = INSUFFICIENT_ENTRIES;
      return;
   }
   
   //Menulis nama file ke sektor files
   clear(files+fileIndex*DIRS_ENTRY_LENGTH, DIRS_ENTRY_LENGTH);
   files[fileIndex*DIRS_ENTRY_LENGTH] = parentIndex;
   i = 0;
   while (i < MAX_FILENAME && inpath[i] != '\0'){
      files[fileIndex*DIRS_ENTRY_LENGTH+i+1] = inpath[i];
      i++;
   }
   
   //Menulis file ke sector tertentu
   readSector(filesect, SECTORS_SECTOR);
   for (i = 0, sectorCount = 0; i < SECTOR_SIZE && sectorCount < *sectors; ++i) {
      if (map[i] == EMPTY) {
         map[i] = USED;
         filesect[fileIndex * DIRS_ENTRY_LENGTH + sectorCount] = i;
         clear(sectorBuffer, SECTOR_SIZE);
         for (j = 0; j < SECTOR_SIZE; ++j) {
            sectorBuffer[j] = buffer[sectorCount * SECTOR_SIZE + j];
         }
         writeSector(sectorBuffer, i);
         ++sectorCount;
      }
   }
   
   writeSector(map, MAP_SECTOR);
   writeSector(files, FILES_SECTOR);
   writeSector(filesect, SECTORS_SECTOR);
}

void executeProgram (char *path, int *result, char parentIndex) {
   struct PCB* pcb;
   int segment;
   int i, fileIndex;
   char buffer[MAX_SECTORS * SECTOR_SIZE];
   readFile(buffer, path, result, parentIndex);
   
   if (*result != NOT_FOUND) {
      setKernelDataSegment();
      segment = getFreeMemorySegment();
      restoreDataSegment();

      fileIndex = *result;
      if (segment != NO_FREE_SEGMENTS) {
         setKernelDataSegment();

         pcb = getFreePCB();
         pcb->index = fileIndex;
         pcb->state = STARTING;
         pcb->segment = segment;
         pcb->stackPointer = 0xFF00;
         pcb->parentSegment = running->segment;
         addToReady(pcb);

         restoreDataSegment();
         for (i = 0; i < SECTOR_SIZE * MAX_SECTORS; i++) {
            putInMemory(segment, i, buffer[i]);
         }
         initializeProgram(segment);
         sleep();
      } else {
         *result = INSUFFICIENT_SEGMENTS;
      }
   }
} 

void terminateProgram (int *result) {
  int parentSegment;
  setKernelDataSegment();

  parentSegment = running->parentSegment;
  releaseMemorySegment(running->segment);
  releasePCB(running);

  restoreDataSegment();
  if (parentSegment != NO_PARENT) {
    resumeProcess(parentSegment, result);
  }
  yieldControl();
} 

void makeDirectory(char *path, int *result, char parentIndex){
   char dirs[SECTOR_SIZE];
   char inpath[SECTOR_SIZE];
   int i,isEqual,sectorCount,dirIndex,success;
   
   copyString(inpath,path,SECTOR_SIZE);
   readSector(dirs, DIRS_SECTOR);
   //Mengubah relative path menjadi path absolute dengan parent index yang sesuai
   relPathToAbsPath(inpath, &parentIndex, &success);
   if (success != 0){
      *result = success;
      return;
   }
   
   //Traversal sector dirs untuk mencari apakah dir currpath sudah ada
   //Jika belum, maka kembalikan index kosong pertama pada sektor dirs
   dirIndex = 0;   //Variabel untuk index pada sector dirs
   while (dirIndex*DIRS_ENTRY_LENGTH < SECTOR_SIZE && dirs[dirIndex*DIRS_ENTRY_LENGTH+1] != '\0'){
      isEqual = isEqualPathName(inpath,dirs+dirIndex*DIRS_ENTRY_LENGTH+1);
      if (isEqual && dirs[dirIndex*DIRS_ENTRY_LENGTH] == parentIndex){
         *result = ALREADY_EXIST;
         return;
      }else{
         dirIndex++;
      }
   }
   if (dirIndex == MAX_FILES){
      *result = INSUFFICIENT_ENTRIES;
      return;
   }
   
   //Menulis nama file ke sektor files
   clear(dirs+dirIndex*DIRS_ENTRY_LENGTH, DIRS_ENTRY_LENGTH);
   dirs[dirIndex*DIRS_ENTRY_LENGTH] = parentIndex;
   i = 0;
   while (i < MAX_DIRNAME && inpath[i] != '\0'){
      dirs[dirIndex*DIRS_ENTRY_LENGTH+i+1] = inpath[i];
      i++;
   }
   *result = 0;
   writeSector(dirs, DIRS_SECTOR);
}

void deleteFile(char *path, int *result, char parentIndex){
   char dirs[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char sectors[SECTOR_SIZE];
   char map[SECTOR_SIZE];
   char inpath[MAX_PATH_LENGTH];
   int i,j,k,isEqual,isDirnameDone,isFilenameDone,success;

   copyString(inpath,path,MAX_PATH_LENGTH);
   readSector(dirs, DIRS_SECTOR);
   //Mengubah relative path menjadi path absolute dengan parent index yang sesuai
   relPathToAbsPath(inpath, &parentIndex, &success);
   if (success != 0){
      *result = success;
      return;
   }
   
   readSector(files, FILES_SECTOR);
   i = 0;   //Variabel untuk traversal sektor files
   while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
      isEqual = isEqualPathName(inpath,files+i*DIRS_ENTRY_LENGTH+1);
      if (isEqual && files[i*DIRS_ENTRY_LENGTH] == parentIndex){
         parentIndex = i;
         break; 
      }else{
         i++;
      }
   }

   //File currpath tidak ditemukan
   if (i == MAX_FILES){
      *result = NOT_FOUND;
      return;
   }

   //Mengubah file name menjadi null
   clear(files+parentIndex*DIRS_ENTRY_LENGTH,DIRS_ENTRY_LENGTH);
   
   //Membaca isi sector file ke buffer
   readSector(sectors, SECTORS_SECTOR);
   readSector(map, MAP_SECTOR);
   i = 0;
   do{
      map[sectors[i+parentIndex*MAX_SECTORS]] = '\0';
      i++;
   }while (i < MAX_SECTORS && sectors[i+parentIndex*MAX_SECTORS] != '\0');
   
   //Mengubah sectors menjadi null
   clear(sectors+parentIndex*DIRS_ENTRY_LENGTH,DIRS_ENTRY_LENGTH);
   
   *result = 0;

   writeSector(map,MAP_SECTOR);
   writeSector(files,FILES_SECTOR);
   writeSector(dirs, DIRS_SECTOR);
   writeSector(sectors,SECTORS_SECTOR);
}

void deleteDirectoryByParentId(char parentIndex){
   char dirs[SECTOR_SIZE];
   int i;

   readSector(dirs,DIRS_SECTOR);
   for (i = 0; i*DIRS_ENTRY_LENGTH<SECTOR_SIZE; i++){
      if (dirs[i*DIRS_ENTRY_LENGTH] == parentIndex && dirs[i*DIRS_ENTRY_LENGTH+1] != '\0'){
         //Mengubah file name menjadi null
         clear(dirs+i*DIRS_ENTRY_LENGTH,DIRS_ENTRY_LENGTH);
         writeSector(dirs,DIRS_SECTOR);
         deleteFileByParentId(i);
         deleteDirectoryByParentId(i);
      }
   }
}

void deleteFileByParentId(char parentIndex){
   char files[SECTOR_SIZE];
   char sectors[SECTOR_SIZE];
   char map[SECTOR_SIZE];
   int i,j;

   readSector(files, FILES_SECTOR);
   readSector(sectors, SECTORS_SECTOR);
   readSector(map, MAP_SECTOR);
   for (i = 0; i*DIRS_ENTRY_LENGTH<SECTOR_SIZE; i++){
      if (files[i*DIRS_ENTRY_LENGTH] == parentIndex){
         //Mengubah file name menjadi null
         clear(files+i*DIRS_ENTRY_LENGTH,DIRS_ENTRY_LENGTH);
         //Mengubah map sehingga sector yang digunakan file ke-i ditandai empty
         j = 0;
         while (j < MAX_SECTORS && sectors[j+i*MAX_SECTORS] != '\0'){
            map[sectors[j+i*MAX_SECTORS]] = '\0';
            j++;
         }
         
         //Mengubah sectors menjadi null
         clear(sectors+i*DIRS_ENTRY_LENGTH,DIRS_ENTRY_LENGTH);
      }
   }

   writeSector(map,MAP_SECTOR);
   writeSector(files,FILES_SECTOR);
   writeSector(sectors,SECTORS_SECTOR);
}

void deleteDirectory(char *path, int *success, char parentIndex){
   char dirs[SECTOR_SIZE];
   char inpath[SECTOR_SIZE];
   int i,j,k,isEqual,isDirnameDone,isLastDirNameDone;
   
   //Mengubah relative path menjadi path absolute dengan parent index yang sesuai
   copyString(inpath,path,SECTOR_SIZE);
   relPathToAbsPath(inpath, &parentIndex, success);
   readSector(dirs,DIRS_SECTOR);
   if (*success != 0){
      return;
   }else{
      i = 0;   //Variabel untuk traversal sektor dirs
      while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
         isEqual = isEqualPathName(inpath,dirs+i*DIRS_ENTRY_LENGTH+1);
         if (isEqual && dirs[i*DIRS_ENTRY_LENGTH] == parentIndex){
            parentIndex = i;
            break;
         }else{
            i++;
         }
      }
      //Folder currpath tidak ditemukan
      if (i == MAX_DIRS){
         *success = NOT_FOUND;
         return;
      }
   }

   //Mengubah dir name menjadi null
   clear(dirs+parentIndex*DIRS_ENTRY_LENGTH,DIRS_ENTRY_LENGTH);
   writeSector(dirs, DIRS_SECTOR);

   //Secara rekursif menghapus semua anak
   deleteFileByParentId(parentIndex);
   deleteDirectoryByParentId(parentIndex);
}

void putArgs (char curdir, char argc, char **argv) {
   char args[SECTOR_SIZE];
   int i, j, p;
   clear(args, SECTOR_SIZE);

   args[0] = curdir;
   args[1] = argc;
   i = 0;
   j = 0;
   for (p = 2; p < ARGS_SECTOR && i < argc; ++p) {
      args[p] = argv[i][j];
      if (argv[i][j] == '\0') {
         ++i;
         j = 0;
      }
      else {
         ++j;
      }
   }

   writeSector(args, ARGS_SECTOR);
}

void getCurdir (char *curdir) {
   char args[SECTOR_SIZE];
   readSector(args, ARGS_SECTOR);
   *curdir = args[0];
}

void getArgc (char *argc) {
   char args[SECTOR_SIZE];
   readSector(args, ARGS_SECTOR);
   *argc = args[1];
}

void getArgv (char index, char *argv) {
   char args[SECTOR_SIZE];
   int i, j, p;
   readSector(args, ARGS_SECTOR);

   i = 0;
   j = 0;
   for (p = 2; p < ARGS_SECTOR; ++p) {
      if (i == index) {
         argv[j] = args[p];
         ++j;
      }

      if (args[p] == '\0') {
         if (i == index) {
            break;
         }
         else {
         ++i;
         }
      }
   }
} 

void yieldControl () {
  interrupt(0x08, 0, 0, 0, 0);
}

void sleep () {
  setKernelDataSegment();

  running->state = PAUSED;

  restoreDataSegment();
  yieldControl();
} 

void pauseProcess (int segment, int *result) {
  struct PCB *pcb;
  int res;

  setKernelDataSegment();

  pcb = getPCBOfSegment(segment);
  if (pcb != NULL && pcb->state != PAUSED) {
    pcb->state = PAUSED; res = SUCCESS;
  } else {
    res = NOT_FOUND;
  }

  restoreDataSegment();
  *result = res;
}

void resumeProcess (int segment, int *result) {
  struct PCB *pcb;
  int res;
  setKernelDataSegment();

  pcb = getPCBOfSegment(segment);
  if (pcb != NULL && pcb->state == PAUSED) {
    pcb->state = READY;
    addToReady(pcb);
    running->state = PAUSED;
    res = SUCCESS;
  } else {
    res = NOT_FOUND;
  }

  restoreDataSegment();
  *result = res;
}

void killProcess (int segment, int *result) {
  struct PCB *pcb;
  int res;
  setKernelDataSegment();

  pcb = getPCBOfSegment(segment);
  if (pcb != NULL) {
    releaseMemorySegment(pcb->segment);
    releasePCB(pcb);
    res = SUCCESS;
  } else {
    res = NOT_FOUND;
  }

  restoreDataSegment();
  *result = res;
}

void printPCB(int segment){
   getPCBOfSegment(segment);
}





