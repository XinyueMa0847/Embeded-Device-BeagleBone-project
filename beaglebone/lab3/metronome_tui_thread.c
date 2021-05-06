#include <termios.h>
#include <stdio.h>
#include <unistd.h> // read() close()
#include <sys/select.h> 
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <signal.h> // Defines signal-handling functions (i.e. trap Ctrl-C)
#include <pthread.h>
#include "userLEDmmap.h"


#define MIN  30 
#define MAX  200
#define STEP  5 

//global variables
int keepgoing = 1; // Set to 0 when Ctrl-c is pressed
const int p[4][6] = {{7,1},{7,1,1},
                        {7,1,3,1},{7,1,1,3,1,1}};   // Time signatures 
const char* name[4] = {"2/4","3/4","4/4","6/8"};
const int p_max[4] = {2,3,4,6};
int TimeSig = 2;                                    // Default is 4/4
float tempo = 90;                                     // Tempo defaults to 90
int Run = 0;                                        // Toggle the switch
int delay = 0;                                      // delay = (tempo/60)*250000;
                                                    // Assumes duty = 50
int quit = 0;
pthread_t tid; 
int loop_i = 0;
// input related functions 
static struct termios old_tio;
static struct termios new_tio;
// Initialize new terminal i/o settings 
void init_termios(int echo) 
{
    tcgetattr(0, &old_tio); // Grab old_tio terminal i/o setting 
    new_tio = old_tio; // Copy old_tio to new_tio
    new_tio.c_lflag &= ~ICANON; // disable buffered i/o 
    new_tio.c_lflag &= echo? ECHO : ~ECHO; // Set echo mode 
    if (tcsetattr(0, TCSANOW, &new_tio) < 0) perror("tcsetattr ~ICANON");
    // Set new_tio terminal i/o setting
}
// Restore old terminal i/o settings 
void reset_termios(void) 
{
    tcsetattr(0, TCSANOW, &old_tio);
}
// Read one character without Enter key: Blocking
char getch(void) 
{
    char ch = 0;
    if (read(0, &ch, 1) < 0) perror ("read()"); // Read one character
    return ch;
}
int key_hit()
{
    struct timeval tv = { 0L, 0L }; //timeout for select(), 0.0s => non-blocking
    fd_set fds;         // Initializaing a fd set
    FD_ZERO(&fds);      // Clears the set for initialization, needed in each iteration of the loop since FDS needs to be 
                        // re-initialized before each call. 
    FD_SET(0, &fds);    // Put STDIN(0) into FDS 
    return select(1, &fds, NULL, NULL, &tv);    //select read on fds, write and wait for exceptional conditions on no fd
                                                //NFDS is set to 1 so relect only reads from STDIN(0)
}
// Callback called when SIGINT is sent to the process (Ctrl-c)
void sigint_handler(int sig){
    printf("\nCtrl-C pressed, cleanning up and exiting...\n");
    keepgoing = 0; 
    raise(SIGQUIT);
}      
void sigquit_handler(int sig){
    pthread_join(tid,NULL);
    printf("Quit!\n");
    fflush(stdout);
    // Reset termios
    reset_termios();
    exit(0);
}
// metronome 
void* metronome(void* arg){
    // gpio1 for user LED connections
    void *gpio1_addr;
    // Other gpio connections 
    volatile unsigned int *gpio_datain;
    volatile unsigned int *gpio_setdataout_addr;
    volatile unsigned int *gpio_cleardataout_addr;                                      
    //Open memory devices 
    int fd = open("/dev/mem", O_RDWR); 

    // Setup user LEDs
    gpio1_addr = mmap(0,GPIO1_SIZE, PROT_READ|PROT_WRITE,MAP_SHARED,fd,GPIO1_START_ADDR);  
    gpio_setdataout_addr = gpio1_addr + GPIO_SETDATAOUT;
    gpio_cleardataout_addr = gpio1_addr + GPIO_CLEARDATAOUT; 
    int b[] = {0,0,0,0};     
    printf("Thread created successfully!\n"); 
    while (keepgoing==1){
        if(Run==0&&keepgoing){usleep(100000);continue;}
        loop_i = 0; 
        while(loop_i<p_max[TimeSig]&&keepgoing&&Run
        ){
            b[0] = p[TimeSig][loop_i] & 1 ? 1:0;
            b[1] = p[TimeSig][loop_i] & (1<<1) ? 1:0; 
            b[2] = p[TimeSig][loop_i] & (1<<2) ? 1:0; 
            b[3] = p[TimeSig][loop_i] & (1<<3) ? 1:0;  
            delay = (60/tempo)*500000;
            if(TimeSig==3){delay /= 2;}   
            printf("%d",p[TimeSig][loop_i]);
            fflush(stdout);
            *gpio_setdataout_addr = b[0]<<21 | b[1]<<22 | b[2]<<23 | b[3]<<24;
            usleep(delay);
            *gpio_cleardataout_addr = GPIO_22 | GPIO_23 | GPIO_24 | GPIO_21;
            usleep(delay);
            loop_i++;
        }
    }
    munmap((void *)gpio1_addr,GPIO1_SIZE);
    close(fd);
    return NULL; 
}
int main(void)
{
    char c;
    int echo;

    //1. Init GPIO LEDs 
    if(!pthread_create(&tid,NULL,&metronome,NULL)==0){
        perror("cannot create thread!");
        keepgoing = 0; 
    };
    //2. Init key processing 
    // Register intr handler for Ctrl-C
    signal(SIGINT, sigint_handler);   
    signal(SIGQUIT, sigquit_handler);  
    // Set terminos 
    echo = 0; // Disable echo
    init_termios(echo);  
    // Print the MENU 
    printf("Menu for algo_metronome_TUI:\n\
            z:     Time signature  2/4 > 3/4 > 4/4 > 6/8 > 2/4\n\
            c:     Dec tempo       Dec tempo by 5 (min tempo 30)\n\
            b:     Inc tempo       Inc tempo by 5 (max tempo 200)\n\
            m:     Start/Stop      Toggles start and stop\n\
            q:     Quit this program\n"  );
    //3. Print default values 
    printf("Key %c TimeSig %d, Tempo %d, Run = %d\n", c,TimeSig, (int)tempo, Run);

    while (1) {
        //nb
        while (!key_hit()){ //print wait char until a key is hit
           //manipulate leds here 
           usleep(250000);
        }
        c = getch();
        switch (c){
            case 'm':
                Run = (Run+1) %2;
                break; 
            case 'z':
                TimeSig = (++TimeSig) % 4;
                break;
            case 'b':
                tempo += tempo >= MAX ? 0 : STEP;
                break;
            case 'c':
                tempo -= tempo <= MIN ? 0 : STEP;
                break;
            case 'q':
                // printf("%c: Quit!\n", c);
                keepgoing = 0;
                // raise(SIGQUIT);
                // keepgoing = 0;
                break;
            default:
                break; //ignore other keys
        } 
        loop_i = -1;
        printf("Key %c TimeSig %d, Tempo %d, Run = %d\n", c,TimeSig, (int)tempo, Run);
        fflush(stdout);
        if (c == 'q') break;
    }
    raise(SIGQUIT);
}