#include <sys/select.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h> // read()

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


/*
 File descriptor sets
       The principal arguments of select() are three "sets" of file
       descriptors (declared with the type fd_set), which allow the
       caller to wait for three classes of events on the specified set
       of file descriptors.  Each of the fd_set arguments may be
       specified as NULL if no file descriptors are to be watched for
       the corresponding class of events.

       Note well: Upon return, each of the file descriptor sets is
       modified in place to indicate which file descriptors are
       currently "ready".  Thus, if using select() within a loop, the
       sets must be reinitialized before each call.  The implementation
       of the fd_set arguments as value-result arguments is a design
       error that is avoided in poll(2) and epoll(7).

       The contents of a file descriptor set can be manipulated using
       the following macros:

       FD_ZERO()
              This macro clears (removes all file descriptors from) set.
              It should be employed as the first step in initializing a
              file descriptor set.

       FD_SET()
              This macro adds the file descriptor fd to set.  Adding a
              file descriptor that is already present in the set is a
              no-op, and does not produce an error.

       FD_CLR()
              This macro removes the file descriptor fd from set.
              Removing a file descriptor that is not present in the set
              is a no-op, and does not produce an error.

       FD_ISSET()
              select() modifies the contents of the sets according to
              the rules described below.  After calling select(), the
              FD_ISSET() macro can be used to test if a file descriptor
              is still present in a set.  FD_ISSET() returns nonzero if
              the file descriptor fd is present in set, and zero if it
              is not.
*/ 
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
    char waitchar[] = "|/-\\";
    printf(" Test_single_key_nb\n");
    printf("single key input in non-blocking mode until 'q' key\n");
    while (1) {
        //nb
        while (!key_hit()){ //print wait char until a key is hit
            i = ++i % 4; //waitchar has length 5
            printf("%c",waitchar[i]);
            fflush(stdout);
            usleep(250000); // 0.25s
        }
        c = getch();
        printf("%c", c);
        //printf("%c %2x ", c, c);
        fflush(stdout);
        if (c == 'q') break;
    }
    printf(" Quit!\n");
    fflush(stdout);
    // Reset termios
    reset_termios();
    return 0;
}
