#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_DIRS 32
#define MAX_FILES 32
#define MAX_DIRNAME 15
#define MAX_FILENAME 15
#define MAX_SECTORS 16
#define DIRS_ENTRY_LENGTH 16
#define MAP_SECTOR 256
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define SECTORS_SECTOR 259
#define ARGS_SECTOR 512
#define TRUE 1
#define FALSE 0
#define INSUFFICIENT_SECTORS 0
#define NOT_FOUND -1
#define ALREADY_EXIST -2
#define INSUFFICIENT_ENTRIES -3
#define EMPTY 0x00
#define USED 0xFF

void handleInterrupt21 (int AX, int BX, int CX, int DX);
void printString(char *string);
void readString(char *string);
int mod(int a, int b);
int div(int a, int b);
void readSector(char *buffer, int sector);
void writeSector(char *buffer, int sector);
void readFile(char *buffer, char *path, int *result, char parentIndex);
void writeFile(char *buffer, char *path, int *sectors, char parentIndex);
void executeProgram(char *filename, int segment, int *success);
void terminateProgram (int *result);
void makeDirectory(char *path, int *result, char parentIndex);
void deleteFile(char *path, int *result, char parentIndex);
void deleteDirectory(char *path, int *success, char parentIndex);
void putArgs (char curdir, char argc, char **argv);
void getCurdir (char *curdir);
void getArgc (char *argc);
void getArgv (char index, char *argv);

int isEqualPathName(char *path1, char *path2);
void drawLogo();
void clearScreen();

int main() {
   //interrupt(0x21, (AH << 8) | AL, BX, CX, DX);
   char buffer[6];
   char readbuffer[SECTOR_SIZE];
   char path[10];
   int sectors = 1;

   buffer[0] = 'H';
   buffer[2] = 'L';
   buffer[1] = 'E';
   buffer[3] = 'L';
   buffer[4] = 'O';
   buffer[5] = '\0';
   
   path[0] = 'h';
   path[1] = 'e';
   path[2] = 'l';
   path[3] = 'l';
   path[4] = 'o';
   path[5] = '.';
   path[6] = 't';
   path[7] = 'x';
   path[8] = 't';
   path[9] = '\0';
   
   drawLogo();
   clearScreen();
   makeInterrupt21();
   writeFile(buffer, path, &sectors, 0xFF);
   if (sectors == 1){
      readFile(readbuffer, path, &sectors, 0xFF);
      printString(readbuffer);
   }else if (sectors == -1){
      printString("notfound");
   }else if (sectors == 0){
      printString("inssector");
   }else if (sectors == -2){
      printString("alrdy");
   }else if (sectors == -3){
      printString("insentr");
   }
   
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
         readString(BX);
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
         executeProgram(BX, CX, DX, AH);
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
         putArgs(BX, CX);
         break;
      case 0x21:
         getCurdir(BX);
         break;
      case 0x22:
         getArgc(BX);
         break;
      case 0X23:
         getArgv(BX, CX);
         break;
      default:
         printString("Invalid interrupt");
   }
} 


void printString(char *string){
   int i = 0;
   while (string[i] != '\0'){
      interrupt(0x10, 0xE00 + string[i], 0, 0, 0);
      i++;
   }
}

void readString(char *string){
   char c = interrupt(0x16, 0, 0, 0, 0);
   int i = 0;
   while (c != '\r'){
      if (c == '\n'){
         string[i] = c;
         i++;
         interrupt(0x10, 0xE00 + '\n', 0, 0, 0);
      }else if (c == '\b'){
         string[i] = '\0';
         i--;
         interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
         interrupt(0x10, 0xE00 + '\0', 0, 0, 0);
         interrupt(0x10, 0xE00 + '\b', 0, 0, 0);
      }else{
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

void readSector(char *buffer, int sector){
   interrupt(0x13, 0x201, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
}

void writeSector(char *buffer, int sector){
   interrupt(0x13, 0x301, buffer, div(sector, 36) * 0x100 + mod(sector, 18) + 1, mod(div(sector, 18), 2) * 0x100);
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

void readFile(char *buffer, char *path, int *result, char parentIndex){
   char dirs[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char sectors[SECTOR_SIZE];
   char currpath[15];
   int i,j,k,isEqual,isDirnameDone,isFilenameDone;
   
   j = 0;   //Variabel untuk menghitung panjang path yang sudah dibaca
   isFilenameDone = FALSE;
   
   readSector(dirs, DIRS_SECTOR);
   //Traversal setiap folder di path 
   while (!isFilenameDone){
      //Menyimpan nama folder saat ini di currpath
      k = 0;
      isDirnameDone = FALSE;
      do{
         if (path[j+k] == '/'){
            isDirnameDone = TRUE;
         }else if (path[j+k] == '\0'){
            isDirnameDone = TRUE;
            isFilenameDone = TRUE;
         }else{  
            currpath[k] = path[j+k];
         }
         k++;
      }while(k < MAX_DIRNAME && !isDirnameDone);
      j += k;
      
      //Traversal dirs untuk mencari folder currpath
      if (!isFilenameDone){
         i = 0;   //Variabel untuk traversal sektor dirs
         while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
            isEqual = isEqualPathName(currpath,dirs+i*DIRS_ENTRY_LENGTH+1);
            if (isEqual && dirs[i*DIRS_ENTRY_LENGTH] == parentIndex){
               parentIndex = i;
            }else{
               i++;
            }
         }
         //Folder currpath tidak ditemukan
         if (i == MAX_DIRS){
            result = NOT_FOUND;
            return;
         }   
      }
   }
   
   readSector(files, FILES_SECTOR);
   i = 0;   //Variabel untuk traversal sektor files
   while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
      isEqual = isEqualPathName(currpath,files+i*DIRS_ENTRY_LENGTH+1);
      if (isEqual && files[i*DIRS_ENTRY_LENGTH] == parentIndex){
         parentIndex = i;
         break; 
      }else{
         i++;
      }
   }
   //File currpath tidak ditemukan
   if (i == MAX_FILES){
      result = NOT_FOUND;
      return;
   }
   
   //Membaca isi sector file ke buffer
   readSector(sectors, SECTORS_SECTOR);
   i = 0;
   do{
      readSector(buffer + i*SECTOR_SIZE, sectors[i+parentIndex*MAX_SECTORS]);
      i++;
   }while (i < MAX_SECTORS && sectors[i+parentIndex*MAX_SECTORS] != '\0');
   *result = 0;
}

void clear(char *buffer, int length) {
   int i;
   for(i = 0; i < length; ++i) {
      buffer[i] = EMPTY;
   }
}

void writeFile(char *buffer, char *path, int *sectors, char parentIndex){
   char map[SECTOR_SIZE];
   char dirs[SECTOR_SIZE];
   char files[SECTOR_SIZE];
   char filesect[SECTOR_SIZE];
   char sectorBuffer[SECTOR_SIZE];
   char currpath[15];
   int i,j,k,isEqual,isDirnameDone,isFilenameDone,sectorCount,fileIndex;
   
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
   
   j = 0;   //Variabel untuk menghitung panjang path yang sudah dibaca
   isFilenameDone = FALSE;
   
   //Traversal setiap folder di path 
   readSector(dirs, DIRS_SECTOR);
   while (!isFilenameDone){
      //Menyimpan nama folder saat ini di currpath
      k = 0;
      isDirnameDone = FALSE;
      do{
         if (path[j+k] == '/'){
            isDirnameDone = TRUE;
         }else if (path[j+k] == '\0'){
            isDirnameDone = TRUE;
            isFilenameDone = TRUE;
         }else{  
            currpath[k] = path[j+k];
         }
         k++;
      }while(k < MAX_DIRNAME && !isDirnameDone);
      j += k;
      
      //Traversal dirs untuk mencari folder currpath
      if (!isFilenameDone){
         i = 0;   //Variabel untuk traversal sektor dirs
         while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
            isEqual = isEqualPathName(currpath,dirs+i*DIRS_ENTRY_LENGTH+1);
            if (isEqual && dirs[i*DIRS_ENTRY_LENGTH] == parentIndex){
               parentIndex = i;
            }else{
               i++;
            }
         }
         //Folder currpath tidak ditemukan
         if (i == MAX_DIRS){
            *sectors = NOT_FOUND;
            return;
         }   
      }
   }
   
   //Traversal sector files untuk mencari apakah file currpath sudah ada
   //Jika belum, maka kembalikan index kosong pertama pada sektor files
   readSector(files, FILES_SECTOR);
   fileIndex = 0;   //Variabel untuk index pada sector files
   while (fileIndex*DIRS_ENTRY_LENGTH < SECTOR_SIZE && files[fileIndex*DIRS_ENTRY_LENGTH+1] != '\0'){
      isEqual = isEqualPathName(currpath,files+fileIndex*DIRS_ENTRY_LENGTH+1);
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
   while (i < MAX_FILENAME && currpath[i] != '\0'){
      files[fileIndex*DIRS_ENTRY_LENGTH+i+1] = currpath[i];
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
   writeSector(dirs, DIRS_SECTOR);
   writeSector(files, FILES_SECTOR);
   writeSector(filesect, SECTORS_SECTOR);
}

void executeProgram(char *path, int segment, int *result, char parentIndex){
   char buffer[MAX_SECTORS*SECTOR_SIZE];
   int successRead;
   int i;
   
   readFile(buffer, path, &successRead, parentIndex);
   if (successRead){
      for(i = 0; i < MAX_SECTORS*SECTOR_SIZE; i++){
         putInMemory(segment,i,buffer[i]);
      }
      launchProgram(segment);
      *result = TRUE;
   }else{
      printString("Failed to launch program");
      *result = FALSE;
   }
}

void terminateProgram (int *result) {
   char shell[6];
   shell[0] = 's';
   shell[1] = 'h';
   shell[2] = 'e';
   shell[3] = 'l';
   shell[4] = 'l';
   shell[5] = '\0';
   executeProgram(shell, 0x2000, result, 0xFF);
}

void makeDirectory(char *path, int *result, char parentIndex){
   //TODO
}
void deleteFile(char *path, int *result, char parentIndex){
   //TODO
}
void deleteDirectory(char *path, int *success, char parentIndex){
   //TODO
}

void putArgs (char curdir, char argc, char **argv) {
   char args[SECTOR_SIZE];
   int i, j, p;
   clear(args, SECTOR_SIZE);

   args[0] = curdir;
   args[1] = argc;
   i = 0;
   j = 0;
   for (p = 1; p < ARGS_SECTOR && i < argc; ++p) {
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
   for (p = 1; p < ARGS_SECTOR; ++p) {
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

void drawLogo(){
   int i = 0;
   int j = 0;
   char next;

   char *title = ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,(%%%*#%###/*,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,/&%%%%%%%,,*%%%%####(,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,*&&&%%%%%%*,,/%%%%%%%%####/*,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,*&&&%%%%%%%%%%%*///(%%%%%%%###**,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,*&,&%%*%%%%%%#%/////#%%%%%*(%#,#/,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,&/*....%%%%(//#%%/....,/%,,,,/,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,(,,,,&*.       %%%%%%%       ./%,,,*/*,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,##*,(%.        ,%%%%%         .%/,*##*,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,%##,/&         (%%%%%          %**###,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,%###/&&       ,%%%%%%%.       %%/###%,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,%%(#(/&&/#%%%%%%%%%%%%%/*%%%%((#(%%##,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,%((((((&&&%%%%%%%#/#%%%%%%%%%%(((((#%,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,#%%(((&&&&&&%%%%%%%%%%%%%%%%%%%(((%%(,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,/&&&%%#((####%########(((###%#%%%%/(,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,&&&&&%(#@*....,./#*....../@(#%%%%%%,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,#&&&##@.....            @(/#%%%%*%/,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,/&&&(##%/               ((///%%%/,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,&&((///*             (/////%%,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,*,,,/////////////////////////,*,*,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,/(///////////(**,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,ElPrimOS v0.1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,Press Any Key To Continue,,,,,,,,,,,,,,,,,,,,,,,,,,,,";

   for (i; i<160*25; i+=2){
      putInMemory(0xB000, 0x8000+i, title[i/2]);
      putInMemory(0xB000, 0x8001+i, 0xF);
   }
   next = interrupt(0x16, 0, 0, 0, 0);
}
void clearScreen(){
   int i = 0;
   for (i; i<160*25;i+=2){
      putInMemory(0xB000, 0x8000+i, '\0');
      putInMemory(0xB000, 0x8001+i, 0xF);
   }
   printString("Welcome to ElPrimOS v0.2\n\r");
}


