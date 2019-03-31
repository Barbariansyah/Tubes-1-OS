#define MAX_BYTE 256
#define SECTOR_SIZE 512
#define MAX_COMMAND 512
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

void changeCurDir(char *path, char *curdir, char parentIndex);
void splitCommand(char *command, char **commandList, char *num_command);
void strCopy(char *str1, char *str2);
void relPathToAbsPath(char *dir, char *parentIndex, int *success);
void clear(char *buffer, int length);

int main() {
	char command[MAX_COMMAND];
	char commandList[16][MAX_FILENAME];
	char path[512];
	char curdir;
	char argc;
	char *argv[MAX_BYTE];
	char parentIdx;
	int result;
	int found;
	int num_command;
	int i,j;
	
	curdir = 0xFF;
	while(1) {
		interrupt(0x21, 0x0, "3===D ", 0, 0);
		interrupt(0x21, 0x1, command, 0, 0);
		splitCommand(command, commandList, &num_command); 
		
		if(strCmp(commandList[0], "cd", 2)) {
			strCopy(commandList[1], path);
			parentIdx = curdir;
			// mengecek apakah directory sudah benar dan path sekarang berisi nama directory terakhir yang jika ditemukan
			relPathToAbsPath(path, &parentIdx, &found);
			if(found) {
				changeCurDir(path, &curdir, parentIdx);
			}
			else {
				interrupt(0x21, 0x0, "Directory not found \n");
			}
		}
		else if (strCmp(commandList[0], "./", 2)) {
			argc = num_command - 2;
			for(i = 2; i < num_command; i++) {
				j = i - 2;
				strCopy(commandList[i], argv[j]);
			}
			
			interrupt(0x21, 0x20, curdir, argc, argv);	
			interrupt(0x21, 0x06, commandList[1], 0x21000, &result, curdir);
		}
		else {
			argc = num_command - 2;
			for(i = 2; i < num_command; i++) {
				j = i - 2;
				strCopy(commandList[i], argv[j]);
			}
			
			interrupt(0x21, 0x20, curdir, argc, argv);
			interrupt(0x21, 0x06, commandList[1], 0x21000, &result, 0xFF);
		}
		
		if(!result) {
			interrupt(0x21, 0x0, "Command is invalid \n");
		}
	}
}

// Fungsi untuk membandingkan string jika true mengembalikan 1 jika false mengembalikan 0
int strCmp(char *str1, char *str2, int size) {
	int eq = TRUE;
	int i = 0;

	while((eq == 1) && (i < size)) {
		if(str1[i] != str2[i]) {
			eq = FALSE;
		}
		i++;
	}
	
	return eq;
}

// Fungsi untuk copy string
void strCopy(char *str1, char *str2) {
	int i = 0;
	while(str1[i] != '\0') {
		str2[i] = str1[i];
	}
}

// Fungsi untuk memecah command menjadi beberapa command inputan user dan menghasilkan jumlah command
void splitCommand(char *command, char **commandList, char *num_command) {
	int i = 0;
	int j;
	
	while(command[i] != '\0') {
		j = 0;
		while(command[j] != ' ') {
			commandList[i][j] = command[j];
			j++;
		}
		
		i++;
	}
	
	num_command = i;
}
		
void changeCurDir(char *path, char *curdir, char parentIndex) {
	//Traversal dirs untuk mencari folder currpath
	int i = 0;   //Variabel untuk traversal sektor dirs
	int isEqual;
	char dirs[SECTOR_SIZE];
	
	intterupt(0x21, 0x02, dirs, DIRS_SECTOR, 0);
	
	while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE) {
		isEqual = strCmp(path,dirs+i*DIRS_ENTRY_LENGTH+1, MAX_DIRNAME);
		if (isEqual && dirs[i*DIRS_ENTRY_LENGTH] == parentIndex){
		   *curdir = i;
		   break;
		}else{
		   i++;
		}
	}
}

// Fungsi untuk mengecek path dari root apakah sudah benar atau belum dan menyimpan directory terakhir serta parent index dari directory akhir
void relPathToAbsPath(char *dir, char *parentIndex, int *success) {
   char dirs[SECTOR_SIZE];
   char currpath[MAX_FILENAME];
   int i, j, k, isEqual, isLastPathDone, isDirnameDone;
   j = 0;   //Variabel untuk menghitung panjang path yang sudah dibaca
   isLastPathDone = FALSE;
   
   intterupt(0x21, 0x02, dirs, DIRS_SECTOR, 0);
   //Traversal setiap folder di path 
   while (!isLastPathDone){
      //Menyimpan nama folder saat ini di currpath
      clear(currpath,MAX_FILENAME);
      k = 0;
      isDirnameDone = FALSE;
      do{
         if (dir[j+k] == '/'){
            isDirnameDone = TRUE;
         }else if (dir[j+k] == '\0'){
            isDirnameDone = TRUE;
            isLastPathDone = TRUE;
         }else{  
            currpath[k] = dir[j+k];
         }
         k++;
      }while(k < MAX_DIRNAME && !isDirnameDone);
      j += k;

      if (!isLastPathDone){
         //Traversal dirs untuk mencari folder currpath
         i = 0;   //Variabel untuk traversal sektor dirs
         while (i*DIRS_ENTRY_LENGTH < SECTOR_SIZE){
            isEqual = strCmp(currpath,dirs+i*DIRS_ENTRY_LENGTH+1, MAX_DIRNAME);
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
   clear(dir,MAX_FILENAME);
   *success = 0;
   for(i = 0; i < MAX_FILENAME; i++){
      dir[i] = currpath[i];
      if (currpath[i] == '\0') break;
   }
}

// Fungsi untuk menghapus array
void clear(char *buffer, int length) {
   int i;
   for(i = 0; i < length; ++i) {
      buffer[i] = EMPTY;
   }
}



