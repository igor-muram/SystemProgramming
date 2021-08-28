#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

void count(DIR* dirPtr, int* filesCount, int* dirCount)
{
	*filesCount = 0;
	*dirCount = 0;

	struct dirent* dir;

	while (dir = readdir(dirPtr))
	{
		struct stat S;
		stat(dir->d_name, &S);

		if (S_ISDIR(S.st_mode))
			(*dirCount)++;

		if (S_ISREG(S.st_mode))
			(*filesCount)++;

	}

	rewinddir(dirPtr);
}	

void fill(DIR* dirPtr, char** files, char** dirs)
{
	int fileNo = 0;
	int dirNo = 0;

	struct dirent* dir;

	while (dir = readdir(dirPtr))
	{
		struct stat S;
		stat(dir->d_name, &S);

		if (S_ISDIR(S.st_mode))
		{	
			dirs[dirNo] = strdup(dir->d_name);
			dirNo++;
		}

		if (S_ISREG(S.st_mode))
		{
			files[fileNo] = strdup(dir->d_name);
			fileNo++;
		}
	}	

	rewinddir(dirPtr);
}

int compare(char* file1, char* file2)
{
	struct stat s1, s2;
	stat(file1, &s1);
	stat(file2, &s2);

	if (s1.st_mtime > s2.st_mtime)
		return 1;
	else if (s1.st_mtime == s2.st_mtime)
		return 0;
	else
		return -1;
}

void swap(char** file1, char** file2)
{
	char* temp = *file1;
	*file1 = *file2;
	*file2 = temp;
}

void sort(char** files, int count)
{
	for (int i = 0; i < count; i++)
	for (int j = i + 1; j < count; j++)
		if (compare(files[i], files[j]) == -1)
			swap(&files[i], &files[j]);
}

void print(char** files, int count)
{
	for (int i = 0; i < count; i++)
		printf("%s\n", files[i]);
}

int main(int argc, char** argv)
{
	DIR* dirPtr = opendir(".");

	int filesCount, dirsCount;
	count(dirPtr, &filesCount, &dirsCount);

	char* files[512];
	char* dirs[512];

	fill(dirPtr, files, dirs);

	sort(files, filesCount);

	print(files, filesCount);

	print(dirs, dirsCount);

	closedir(dirPtr);

	return 0;
}
