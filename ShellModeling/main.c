#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

// ls -lisa | sort | wc -l > a.txt

int main()
{
	int fd1[2], fd2[2];

	pipe(fd1);
	// Check errors

	pipe(fd2);
	// Check errors

	int ls_pid = fork();
	// Check errors

	if (ls_pid == 0)
	{
		fprintf(stderr, "LS: created\n");

		close(fd2[0]);
		close(fd2[1]);
		close(fd1[0]);

		fprintf(stderr, "LS: stdout redirected\n");
		close(1);
		fcntl(fd1[1], F_DUPFD, 1);

		fprintf(stderr, "LS: finished\n");
		if (execl("/bin/ls", "ls", "-lisa", NULL) == -1)
		{
			perror("execl");
			exit(errno);
		}

		exit(0);
	}

	int sort_pid = fork();
	// Check errors

	if (sort_pid == 0)
	{
		fprintf(stderr, "SORT: created\n");

		close(fd1[1]);
		close(0);
		fcntl(fd1[0], F_DUPFD, 0);
		fprintf(stderr, "SORT: stdin redirected\n");

		close(fd2[0]);
		close(1);
		fcntl(fd2[1], F_DUPFD, 1);
		fprintf(stderr, "SORT: stdout redirected\n");

		fprintf(stderr, "SORT: finished\n");
		if (execl("/usr/bin/sort", "sort", NULL) == -1)
		{
			perror("execl");
			exit(errno);
		}

		exit(0);
	}

	int wc_pid = fork();
	// Check errors

	if (wc_pid == 0)
	{
		fprintf(stderr, "WC: created\n");
		close(fd1[0]);
		close(fd1[1]);
		close(fd2[1]);

		close(0);
		fcntl(fd2[0], F_DUPFD, 0);
		fprintf(stderr, "WC: stdin redirected\n");

		int fd = open("a.txt", O_CREAT | O_WRONLY);
		close(1);
		fcntl(fd, F_DUPFD, 1);
		fprintf(stderr, "WC: stdout redirected\n");

		fprintf(stderr, "WC: finished\n");
		if (execl("/usr/bin/wc", "wc", "-l", NULL) == -1)
		{
			perror("execl");
			exit(errno);
		}

		exit(0);
	}

	wait(NULL);
	close(fd1[0]);
	close(fd1[1]);
	close(fd2[0]);
	close(fd2[1]);

	return 0;
}