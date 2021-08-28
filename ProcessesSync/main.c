#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{
	// Arguments checking
	if (argc != 2)
	{
		errno = EINVAL;
		perror("Wrong number of arguments");
		return errno;
	}

	int max_level = atoi(argv[1]);
	if (max_level > 5)
	{
		errno = EINVAL;
		perror("Level > 5 too dangerous");
		return errno;
	}
	else if (max_level < 0)
	{
		errno = EINVAL;
		perror("Level can't be less then 0");
		return errno;
	}

	int curr_level = max_level;

	printf("Process on level %d has been created\n", max_level - curr_level);

	if (max_level > 0)
	{
		printf("Two children on level %d have been created\n", max_level - (curr_level - 1));

		int p[2];					// Pipe for child processes
		char s_write[100];			// String for write-fd storage
		char s_max_level[10];		// String for max_level
		char s_curr_level[10];		// String for curr_level
		int child_pid1, child_pid2; // Pids of child processes

		// Create pipe
		if (pipe(p) < 0)
		{
			perror("pipe");
			return errno;
		}

		sprintf(s_write, "%d", p[1]);				 // write-fd to string
		sprintf(s_max_level, "%d", max_level);		 // max_level to string
		sprintf(s_curr_level, "%d", curr_level - 1); // curr_level to string

		// Creating first child-process
		int pid1 = fork();
		if (pid1 < 0)
		{
			perror("fork");
			return errno;
		}

		// Calling process_node for first child-process
		if (pid1 == 0)
		{
			if (execl("./process_node", s_write, s_max_level, s_curr_level, NULL) == -1)
			{
				perror("execl");
				return errno;
			}
			exit(0);
		}

		// Reading first child-process's pid
		if (read(p[0], &child_pid1, sizeof(int)) < 0)
		{
			perror("read");
			return errno;
		}

		// Creating second child-process
		int pid2 = fork();
		if (pid2 < 0)
		{
			perror("fork");
			return errno;
		}

		// Calling process_node for second child-process
		if (pid2 == 0)
		{
			if (execl("./process_node", s_write, s_max_level, s_curr_level, NULL) == -1)
			{
				perror("execl");
				return errno;
			}
			exit(0);
		}

		// Reading second child-process's pid
		if (read(p[0], &child_pid2, sizeof(int)) < 0)
		{
			perror("read");
			return errno;
		}

		// Waiting for two child-processes to finish
		waitpid(child_pid1, NULL, 0);
		waitpid(child_pid2, NULL, 0);
	}

	printf("Process on level %d finished work \n", max_level - curr_level);

	return 0;
}
