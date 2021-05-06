/*
    Program test_single_key.c
    Get a key input without hitting Enter key
    using termios
    Modified from
    http://stackoverflow.com/questions/7469139/what-is-equivalent-to-getch-getche-inlinux
    Global old_termios and new_termios for efficient key inputs.
*/
#include <termios.h>
#include <stdio.h>
#include <unistd.h> // read()
#include <sys/select.h> 
// GLobal termios structs
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
    echo = 1; // Disable echo
    init_termios(echo);
    // Test loop
    int i = 0;
    char waitchar[] = "|/-\\";
    printf(" Test_single_key\n");
    printf("single key input until 'q' key\n");
    while (!key_hit()){ //print wait char until a key is hit
        i = ++i % 4; //waitchar has length 5
        printf("%c",waitchar[i]);
        fflush(stdout);
        usleep(250000); // 0.25s
    }                                                           
    while (1) {
        c = getch();
        printf("%c", c);
        fflush(stdout);
        if (c == 'q') break;
    }
    printf(" Quit!\n");
    fflush(stdout);
    // Reset termios
    reset_termios();
    return 0;
}