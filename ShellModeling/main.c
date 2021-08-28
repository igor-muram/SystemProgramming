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

	// Create two channels
	if (pipe(fd1) < 0)
	{
		perror("pipe");
		return errno;
	}

	if (pipe(fd2) < 0)
	{
		perror("pipe");
		return errno;
	}

	// Create first process
	int ls_pid = fork();

	if (ls_pid == -1)
	{
		perror("LS: fork");
		return errno;
	}

	if (ls_pid == 0)
	{
		fprintf(stderr, "LS: created\n");
		// Close unnecessary channels
		close(fd2[0]);
		close(fd2[1]);
		close(fd1[0]);

		// Redirect stdout
		fprintf(stderr, "LS: stdout redirected\n");
		close(1);
		if (fcntl(fd1[1], F_DUPFD, 1) < 0)
		{
			perror("LS: fcntl");
			exit(errno);
		}

		// Call ls -lisa
		fprintf(stderr, "LS: finished\n");
		if (execl("/bin/ls", "ls", "-lisa", NULL) == -1)
		{
			perror("LS: execl");
			exit(errno);
		}

		exit(0);
	}

	// Create second process
	int sort_pid = fork();

	if (sort_pid == -1)
	{
		perror("SORT: fork");
		return errno;
	}

	if (sort_pid == 0)
	{
		fprintf(stderr, "SORT: created\n");

		// Close unnecessary channels
		close(fd1[1]);
		close(fd2[0]);

		// Redirect stdin
		close(0);
		if (fcntl(fd1[0], F_DUPFD, 0) < 0)
		{
			perror("SORT: fcntl");
			exit(errno);
		}
		fprintf(stderr, "SORT: stdin redirected\n");

		// Redirect stdout
		close(1);
		if (fcntl(fd2[1], F_DUPFD, 1) < 0)
		{
			perror("SORT: fcntl");
			exit(errno);
		}
		fprintf(stderr, "SORT: stdout redirected\n");

		// Call sort
		fprintf(stderr, "SORT: finished\n");
		if (execl("/usr/bin/sort", "sort", NULL) == -1)
		{
			perror("execl");
			exit(errno);
		}

		exit(0);
	}

	// Create third process
	int wc_pid = fork();

	if (wc_pid == -1)
	{
		perror("WC: fork");
		return errno;
	}
	if (wc_pid == 0)
	{
		// Close unnecessary channels
		fprintf(stderr, "WC: created\n");
		close(fd1[0]);
		close(fd1[1]);
		close(fd2[1]);

		// Redirect stdin
		close(0);
		if (fcntl(fd2[0], F_DUPFD, 0) < 0)
		{
			perror("WC: fcntl");
			exit(errno);
		}
		fprintf(stderr, "WC: stdin redirected\n");

		// Create file a.txt
		int fd = open("a.txt", O_CREAT | O_WRONLY);
		if (fd < 0)
		{
			perror("WC: open");
			exit(errno);
		}

		// Redirect stdout
		close(1);
		if (fcntl(fd, F_DUPFD, 1) < 0)
		{
			perror("WC: fcntl");
			exit(errno);
		}
		fprintf(stderr, "WC: stdout redirected\n");

		// Call wc -l
		fprintf(stderr, "WC: finished\n");
		if (execl("/usr/bin/wc", "wc", "-l", NULL) == -1)
		{
			perror("execl");
			exit(errno);
		}

		exit(0);
	}

	// Wait for processes
	wait(NULL);
	close(fd1[0]);
	close(fd1[1]);
	close(fd2[0]);
	close(fd2[1]);

	return 0;
}