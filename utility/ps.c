#define PROG_SIZE 1000
#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define DIRS_ENTRY_LENGTH 16

#include "../proc.h"

char * stateOfPCB(int a){
    char * defunct = "DEFUNCT";
    char * running = "RUNNING";
    char * starting = "STARTING";
    char * ready = "READY";
    char * paused = "PAUSED";

    switch(a){
        case 0:
            return defunct;
            break;
        case 1:
            return running;
            break;
        case 2:
            return starting;
            break;
        case 3:
            return ready;
            break;
        case 4:
            return paused;
            break;
    }
}

void main(){
    struct PCB *pcb;
    int i = 0;

    enableInterrupts();

    // for(i;i<8;i++){
    //     int addr = i*0x1000 + 0x2000;
    //     interrupt(0x21, 0x35, addr, pcb, 0);
    //     interrupt(0x21, 0x00,"PID: ",0,0);
    //     interrupt(0x10,0xE00+i+'0',0,0,0);
    //     interrupt(0x21, 0x00,"  ",0,0);
    //     interrupt(0x21, 0x00,"STATUS: ",0,0);
    //     interrupt(0x21, 0x00,stateOfPCB(pcb->state),0,0);
    //     interrupt(0x21, 0x00,"\n\r",0,0);
    // }

    interrupt(0x21, 0x07, 0, 0, 0);
}