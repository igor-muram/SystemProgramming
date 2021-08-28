#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>

#define START SIGCONT

int main(int argc, char **argv)
{
	// Check arguments
	if (argc < 3)
	{
		errno = EINVAL;
		perror("Not enough arguments");
		return errno;
	}

	int n = atoi(argv[1]);
	int r = atoi(argv[2]);

	if (argc != r + 3)
	{
		errno = EINVAL;
		perror("Wrong number of arguments");
		return -2;
	}

	// Read arguments
	int ni[100];
	for (int i = 0; i < r; i++)
		ni[i] = atoi(argv[3 + i]);

	ni[r] = n;

	// Check sum of ni
	int check_sum = 0;
	for (int i = 0; i < r; i++)
		check_sum += ni[i];

	if (check_sum != n)
	{
		errno = EINVAL;
		perror("Sum of (ni) is not equal to (n)");
		return -3;
	}

	int p[2];		   // Pipe for child processes
	char s_write[100]; // String for write-fd storage
	char s_value[100]; // String for value storage
	int pids[100];	   // Array of child pids

	if (pipe(p) < 0)
	{
		perror("pipe");
		return errno;
	}

	sprintf(s_write, "%d", p[1]); // Write-fd to string

	for (int i = 0; i < r + 1; i++)
	{
		sprintf(s_value, "%d", ni[i]); // Value to string

		// Create child process
		int pid = fork();
		if (pid < 0)
		{
			perror("fork");
			return errno;
		}

		// Call factorial program
		if (pid == 0)
		{
			execl("./factorial", s_value, s_write, NULL);
			exit(0);
		}

		// Read child pid
		if (read(p[0], &pids[i], sizeof(int)) < 0)
		{
			perror("read");
			return errno;
		}
	}

	int gpid = getpgrp(); // Group pid

	// Send start signal for all processes in group
	int tries = 0;
	while (killpg(gpid, START) < 0 && tries < 15)
		tries++;

	if (tries == 15)
	{
		perror("killpg");
		return errno;
	}

	// Waiting for all child processes to finish
	for (int i = 0; i < r + 1; i++)
		waitpid(pids[i], NULL, 0);

	// Reading values from child processes
	for (int i = 0; i < r + 1; i++)
	{
		if (read(p[0], &ni[i], sizeof(int)) < 0)
		{
			perror("read");
			return errno;
		}
	}

	// Calculate result
	int max = ni[0];
	for (int i = 0; i < r + 1; i++)
		if (ni[i] > max)
			max = ni[i];

	int max_i;
	for (int i = 0; i < r + 1; i++)
		if (ni[i] == max)
			max_i = i;

	int result = max;
	for (int i = 0; i < r + 1; i++)
		if (i != max_i)
			result /= ni[i];

	// Print result
	printf("%d\n", result);
	return 0;
}