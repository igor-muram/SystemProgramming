#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

struct data
{
	int pid;
	int value;
};

int main(int argc, char *argv[])
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
	int *ni = malloc((r + 1) * sizeof(int));
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

	// Open temporary file
	int fd = open("tempfile.tmp", O_CREAT | O_RDWR);
	if (fd == -1)
	{
		perror("open");
		return errno;
	}

	// Creating r + 1 processes for calculating factorials
	for (int i = 0; i < r + 1; i++)
	{
		int pid = fork();
		// Check for errors
		if (pid == -1)
		{
			perror("fork");
			return errno;
		}

		if (pid == 0)
		{
			// Calculate factorial
			struct data d;
			d.pid = getpid();
			d.value = 1;
			int n = ni[i];

			for (int i = 1; i <= n; i++)
				d.value *= i;

			// Write data to temporary file
			write(fd, &d, sizeof(struct data));
			close(fd);
			exit(0);
		}
	}

	// Check temporary file
	struct stat s;
	while (s.st_size != (r + 1) * sizeof(struct data))
		fstat(fd, &s);

	// Reading values from temporary file
	lseek(fd, 0, SEEK_SET);
	for (int i = 0; i < r + 1; i++)
	{
		struct data res;
		read(fd, &res, sizeof(struct data));
		ni[i] = res.value;
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

	// Print results
	printf("%d", result);

	// Close and delete temporary file
	close(fd);
	remove("tempfile.tmp");
	return 0;
}