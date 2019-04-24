#define PROG_SIZE 1000
#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define DIRS_ENTRY_LENGTH 16

#define MAIN
#include "../proc.h"

void main(){
    struct PCB *pcb;
    int status;
    int i = 0;

    enableInterrupts();

    for(i;i<8;i++){
        pcb = getPCBOfSegment(i*1000+2000);
        if (pcb == NULL){
            status = 0;
        }
        interrupt(0x21, 0x00,"PID: ",0,0);
        interrupt(0x10,0xE00+i+'0',0,0,0);
        interrupt(0x21, 0x00,"  ",0,0);
        interrupt(0x21, 0x00,"STATUS: ",0,0);
        switch(pcb->state){
            case 0:
                interrupt(0x21, 0x00, "DEFUNCT",0,0);
                break;
            case 1:
                interrupt(0x21, 0x00, "RUNNING",0,0);
                break;
            case 2:
                interrupt(0x21, 0x00, "STARTING",0,0);
                break;
            case 3:
                interrupt(0x21, 0x00, "READY",0,0);
                break;
            case 4:
                interrupt(0x21, 0x00, "PAUSED",0,0);
                break;
        }
        interrupt(0x21, 0x00,"\n\r",0,0);
    }

    interrupt(0x21, 0x07, 0, 0, 0);
}