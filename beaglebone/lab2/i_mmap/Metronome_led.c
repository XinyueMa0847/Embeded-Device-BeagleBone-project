// From: http://stackoverflow.com/questions/13124271/driving-beaglebone-gpio
// -through-dev-mem
// user contributions licensed under cc by-sa 3.0 with attribution required
// http://creativecommons.org/licenses/by-sa/3.0/
// http://blog.stackoverflow.com/2009/06/attribution-required/
// Author: madscientist159 
//(http://stackoverflow.com/users/3000377/madscientist159)
//
// Read one gpio pin and write it out to another using mmap.
// Be sure to set -O3 when compiling.

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
int tempo = 60; 
const int p[] = {7,1,1,3,1,1};    
int duty = 50;  
int delay = 250000;

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
    // delay = ((tempo/60)*(duty/100)/2)*10000000;

    printf("Playing 60bpm 6/8 metronome! \n");
    while(keepgoing==1){
        int i = 0;
        while(i<6&&keepgoing){
            b[0] = p[i] & 1 ? 1:0;
            b[1] = p[i] & (1<<1) ? 1:0; 
            b[2] = p[i] & (1<<2) ? 1:0; 
            b[3] = p[i] & (1<<3) ? 1:0;  
            printf("%d ",p[i]);
            fflush(stdout);
            *gpio_setdataout_addr = b[0]<<21 | b[1]<<22 | b[2]<<23 | b[3]<<24;
            usleep(delay);
            *gpio_cleardataout_addr = GPIO_22 | GPIO_23 | GPIO_24 | GPIO_21;
            usleep(delay);
            i++;
        }
        
    }
    munmap((void *)gpio1_addr,GPIO1_SIZE);
    close(fd);            
    return 0;
}
