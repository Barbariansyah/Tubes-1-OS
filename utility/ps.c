#define PROG_SIZE 1000
#define SECTOR_SIZE 512
#define DIRS_SECTOR 257
#define FILES_SECTOR 258
#define DIRS_ENTRY_LENGTH 16

    char * stateOfPCB(int a){
        char defunct[8] = "DEFUNCT";
        char running[8] = "RUNNING";
        char starting[8] = "STARTING";
        char ready[8] = "READY";
        char paused[8] = "PAUSED";

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
    setKernelDataSegment();

    for(i;i<8;i++){

        pcb = getPCBOfSegment(i*0x1000 + 0x2000);
        interrupt(0x21, 0x00,"PID: ",0,0);
        interrupt(0x10,0xE00+i+'0',0,0,0);
        interrupt(0x21, 0x00,"  ",0,0);
        interrupt(0x21, 0x00,"STATUS: ",0,0);
        interrupt(0x21, 0x00,stateOfPCB(pcb->state),0,0);
        interrupt(0x21, 0x00,"\n\r",0,0);

    }

    restoreDataSegment();

}