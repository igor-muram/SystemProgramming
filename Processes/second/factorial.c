#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#define START SIGCONT

void receive() {}

int main(int argc, char **argv)
{
    // Read arguments
    int n = atoi(argv[0]);
    int fd_write = atoi(argv[1]);

    // Send child's pid to parent process
    int pid = getpid();
    write(fd_write, &pid, sizeof(int));

    // Waiting for start signal from parent process
    signal(START, receive);
    pause();

    // Calculate factorial
    int factorial = 1;
    for (int i = 1; i <= n; i++)
        factorial *= i;

    // Send value to parent process
    write(fd_write, &factorial, sizeof(int));

    exit(0);
}
