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

int process_args(char* arg)
{
	int length = strlen(arg);

	for (int i = 0; i < length; i++)
		if (!isdigit(arg[i]))
			return -1;

	return atoi(arg);
}

int main(int argc, char** argv)
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
			// Create message
			msg m;
			m.mtype = mtype;
			int data = i % 1;
			sprintf(m.mtext, "%d", data);
			printf("child process, data to queue: %s\n", m.mtext);

			// Send message to queue
			if (msgsnd(msgqid, (void*)&m, SIZE, IPC_NOWAIT))
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

		// Receive massage from queue
		if (msgrcv(msgqid, (void*)&m, SIZE, mtype, 0) < 0)
		{
			perror("msgrcv");
			return errno;
		}

		char* data = m.mtext;
		printf("parent process, data from queue: %s\n", data);

		// Put data to the table
		if (!t.contains(&t, data))
			t.put(&t, data);
	}

	// Print table	
	t.print(&t);

	// Close message queue
	msgctl(msgqid, IPC_RMID, NULL);

	return 0;
}
