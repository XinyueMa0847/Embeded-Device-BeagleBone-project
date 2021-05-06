// *************************************************************** //
// userLEDmmap.c, controls the user LED0 to play SOS in moss code. //
// *************************************************************** //

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <signal.h> // Defines signal-handling functions (i.e. trap Ctrl-C)
#include <unistd.h> // close()
#include "userLEDmmap.h"

// Global variable
int keepgoing = 1; // Set to 0 when Ctrl-c is pressed
int _long = 450000;  // Long blink
int _short = 150000;  // Short blink
int sos[] = {4,4,4,2,2,2,1,1,1};
// Callback called when SIGINT is sent to the process (Ctrl-c)
void signal_handler(int sig){
  printf("\nCtrl-C pressed, cleanning up and exiting...\n");
  keepgoing = 0; 
}

int main(int argc, char* argv[]){
    // gpio1 for user LED connections
    void *gpio1_addr;
    // Other gpio connections 
    volatile unsigned int *gpio_datain;
    volatile unsigned int *gpio_setdataout_addr;
    volatile unsigned int *gpio_cleardataout_addr;                                      
    // Register intr handler for Ctrl-C
    signal(SIGINT, signal_handler);
    //Open memory devices 
    int fd = open("/dev/mem", O_RDWR); 

    // Setup user LEDs
    gpio1_addr = mmap(0,GPIO1_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,GPIO1_START_ADDR);  
    gpio_setdataout_addr = gpio1_addr + GPIO_SETDATAOUT;
    gpio_cleardataout_addr = gpio1_addr + GPIO_CLEARDATAOUT; 

    int b[] = {0,0,0,0}; 

    while(keepgoing==1){
        printf("SOS! \n");
        int i = 0;
        while(i<9&&keepgoing){
            b[0] = sos[i] & 1 ? 1:0;
            b[1] = sos[i] & (1<<1) ? 1:0; 
            b[2] = sos[i] & (1<<2) ? 1:0; 
            b[3] = sos[i] & (1<<3) ? 1:0;  
            *gpio_setdataout_addr = b[0]<<21 | b[1]<<22 | b[2]<<23 | b[3]<<24;
            if ( i<3 || i>5){usleep(_short);}else{usleep(_long);}
            *gpio_cleardataout_addr = GPIO_22 | GPIO_23 | GPIO_24 | GPIO_21;
            if ( i>2 && i<6){1000000-usleep(_long);}else{1000000-usleep(_long);}
            i++;
        }
        *gpio_setdataout_addr = GPIO_22 | GPIO_23 | GPIO_24 | GPIO_21;
        sleep(1);
        *gpio_cleardataout_addr = GPIO_22 | GPIO_23 | GPIO_24 | GPIO_21;
        sleep(1);
        
    }
    munmap((void *)gpio1_addr,GPIO1_SIZE);
    close(fd);            
    return 0;
}
