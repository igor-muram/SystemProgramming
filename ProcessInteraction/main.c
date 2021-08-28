#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#include <errno.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "table.h"

#define SIZE 1024

typedef struct message
{
    long mtype;
    char mtext[SIZE];
} msg;

const int mtype = 1;

int process_args(char *arg)
{
    int length = strlen(arg);

    for (int i = 0; i < length; i++)
        if (!isdigit(arg[i]))
            return -1;

    return atoi(arg);
}

int main(int argc, char **argv)
{
    // Process argument
    if (argc != 2)
    {
        errno = EINVAL;
        perror("Wrong number of arguments");
        return errno;
    }

    int K = process_args(argv[1]);
    if (K == -1)
    {
        errno = EINVAL;
        perror("Argument must be a number");
        return errno;
    }

    printf("Argument equals %d\n", K);

    // Create message queue
    int msgqid = msgget(0, IPC_CREAT | 0666);
    if (msgqid < 0)
    {
        perror("msgget");
        return errno;
    }

    // Create K processes
    for (int i = 0; i < K; i++)
    {
        int pid = fork();
        if (pid == -1)
        {
            perror("fork");
            return errno;
        }

        if (pid == 0)
        {
            int child = getpid();
            msg m;
            m.mtype = mtype;
            int data = i % 7;
            sprintf(m.mtext, "%d", data);
            printf("child process: %d, data to queue: %s\n", child,  m.mtext);
            if(msgsnd(msgqid, (void *)&m, strlen(m.mtext) + 1, IPC_NOWAIT))
	    {
		perror("msgsnd");
                exit(errno);
            }
            exit(0);
        }
    }

    // Create table
    struct table t;
    init(&t);

    for (int i = 0; i < K; i++)
    {
        msg m;
        if(msgrcv(msgqid, (void *)&m, SIZE, mtype, 0) < 0)
	{
		perror("msgrcv");
		return errno;
	}

        char *data = m.mtext;
        printf("parent process, data from queue: %s\n", data);
        if (!t.contains(&t, data))
            t.put(&t, data);
    }

    t.print(&t);
    return 0;
}
