#include <termios.h>
#include <stdio.h>
#include <unistd.h> // read()
#include <sys/select.h> 
#include <string.h>
#define MIN  30 
#define MAX  200
#define STEP  5 

//global variables
const int p[4][6] = {{7,1},{7,1,1},
                        {7,1,3,1},{7,1,1,3,1,1}};   // Time signatures 
int TimeSig = 2;                                    // Default is 4/4
int tempo = 90;                                     // Tempo defaults to 90
int Run = 0;                                        // Toggle the switch
int delay = 0;                                      // delay = (tempo/60)*250000;
                                                    // Assumes duty = 50
int quit = 0;

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
int main(void)
{
    char c;
    int echo;
    // Init termios: Disable buffered IO with arg 'echo'
    echo = 0; // Disable echo
    init_termios(echo);
    // Test loop
    int i = 0;

    // Print the MENU 
    printf("Menu for algo_metronome_TUI:\n\
            z:     Time signature  2/4 > 3/4 > 4/4 > 6/8 > 2/4\n\
            c:     Dec tempo       Dec tempo by 5 (min tempo 30)\n\
            b:     Inc tempo       Inc tempo by 5 (max tempo 200)\n\
            m:     Start/Stop      Toggles start and stop\n\
            q:     Quit this program\n"  );
    while (1) {
        //nb
        while (!key_hit()){ //print wait char until a key is hit
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
                quit = 1;
                break;
            default:
                break; 
        } 
        //ignore other keys
        if(strchr("mzbcq",c)==NULL){continue;}
        printf("Key %c TimeSig %d, Tempo %d, Run = %d, \n", c,TimeSig, tempo, Run);
        fflush(stdout);
        if (c == 'q') break;
    }
    printf(" Quit!\n");
    fflush(stdout);
    // Reset termios
    reset_termios();
    return 0;
}