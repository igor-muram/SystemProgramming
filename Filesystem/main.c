#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>

// Подсчет количества файлов и папок в данной директории
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

// Заполнение массива названий файлов и массива названий папок именами файлов и папок
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

// Функция сравнения двух файлов по дате последней модификации. Нужна для сортировки
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

// Функция обмена двух названий файлов местами. Нужна для сортировки
void swap(char** file1, char** file2)
{
	char* temp = *file1;
	*file1 = *file2;
	*file2 = temp;
}

// Функция сортировки для массива названий файлов. Пузырьковая сортировка
void sort(char** files, int count)
{
	for (int i = 0; i < count; i++)
	for (int j = i + 1; j < count; j++)
		if (compare(files[i], files[j]) == -1)
			swap(&files[i], &files[j]);
}

// Функция вывода массивов названий файлов
void print(char** files, int count)
{
	for (int i = 0; i < count; i++)
		printf("%s\n", files[i]);
}

int main(int argc, char** argv)
{
	// Открытие директории, в которой мы сейчас находимся
	DIR* dirPtr = opendir(".");

	// Подсчет файлов и папок в данной директории
	int filesCount, dirsCount;
	count(dirPtr, &filesCount, &dirsCount);

	// Массивы названий файлов и названий папок
	char* files[512];
	char* dirs[512];

	// Заполение массивов названий файлов и папок
	fill(dirPtr, files, dirs);

	// Сортировка массива названий файлов по дате последней модификации
	sort(files, filesCount);
	
	// Вывод отсортированного массива названий файлов или сообщение об отсутствии файлов в данной директории
	if (filesCount != 0)
	{	
		printf("Files:\n");
		print(files, filesCount);
	}
	else
	{
		printf("There are no files\n");
	}

	printf("\n\n");

	// Вывод массива названий папок или сообщение об отсутствии папок в данной директории
	if (dirsCount != 0)
	{
		printf("Directories:\n");
		print(dirs, dirsCount);
	}
	else
	{
		printf("There are no directories\n");
	}
	closedir(dirPtr);

	return 0;
}
