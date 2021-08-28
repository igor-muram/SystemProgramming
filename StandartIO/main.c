#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

// Структура для хранения элемента таблицы открытых файлов
struct opened_file 
{
	char* filename;			// Имя файла
	int fd;					// Описатель файла
	int file_table_ptr;		// Указатель на место в таблице файлов
};

// Структура для хранения элемента таблицы файлов
struct file 
{
	char* filename;			// Имя файла
	char* mode;				// Режим открытого файла
	int counter;			// Счетчик ссылок на файл из таблицы открытых файлов
	int ino;				// Inode файла
	int fd_table_ptr;		// Указатель на место в таблице описателей файлов
};

// Структура для хранения элемента таблицы описателей файлов
struct file_descriptor 
{	
	char* filename;			// Имя файла
	struct stat info;		// Структура stat с информацией о файле
	int counter;			// Счетчик ссылок на файл из таблицы файлов
};

// Функция для печати таблицы открытых файлов
void print_opened_file_table(struct opened_file* table, int size)
{
	printf("Opened file table\n");
	printf("------------------------------\n");
	printf("name\tfd\tfile_table_pos\n");
	printf("------------------------------\n");
	for (int i = 0; i < size; i++)
	{
		struct opened_file f = table[i];
		printf("%s\t%d\t%d\n", f.filename, f.fd, f.file_table_ptr);
	}
	printf("------------------------------\n");
}

// Функция для печати таблицы файлов
void print_file_table(struct file* table, int size)
{
	printf("File table\n");
	printf("--------------------------------------------\n");
	printf("name\tinode\tmode\tcounter\tfd_table_pos\n");
	printf("--------------------------------------------\n");
	for (int i = 0; i < size; i++)
	{
		struct file f = table[i];
		printf("%s\t%d\t%s\t%d\t%d\n", f.filename, f.ino, f.mode, f.counter, f.fd_table_ptr);
	}
	printf("--------------------------------------------\n");
}

// Функция для печати таблицы описателей файлов
void print_fd_table(struct file_descriptor* table, int size)
{
	printf("File descriptor table\n");
	printf("---------------------------------------------------------------\n");
	printf("name\tinode\tmode\tlinks\tUID\tGID\tsize\tcounter\n");
	printf("---------------------------------------------------------------\n");
	for (int i = 0; i < size; i++)
	{
		struct file_descriptor fd = table[i];
		printf("%s\t%d\t%d\t%ld\t%d\t%d\t%ld\t%d\n", 
				fd.filename,
				fd.info.st_ino,
				fd.info.st_mode, 
				fd.info.st_nlink, 
				fd.info.st_uid,
				fd.info.st_gid,
				fd.info.st_size,
				fd.counter);
	}
	printf("---------------------------------------------------------------\n");
}

// Поиск файла в таблице описателей файлов по имени файла
int find_fd_table(struct file_descriptor *table, int size, char *filename)
{
	for (int i = 0; i < size; i++)
		if (strcmp(table[i].filename, filename) == 0) 
			return i;

	return -1;
}

// Добавление файла в таблицу открытых файлов
void add_opened_file(struct opened_file *table, int pos, char* filename, int fd, int ptr)
{
	table[pos].filename = filename;
	table[pos].fd = fd;
	table[pos].file_table_ptr = ptr;
}

// Добавление файла в таблицу файлов
void add_file(struct file *table, int pos, char* filename, int ino, char* mode, int counter, int ptr)
{
	table[pos].filename = filename;
	table[pos].ino = ino;
	table[pos].mode = mode;
	table[pos].counter = counter;
	table[pos].fd_table_ptr = ptr;
}

// Добавление файла в таблицу описателей файлов
int add_fd(struct file_descriptor *table, int pos, char* filename)
{
	struct stat info;
	int status = stat(filename, &info);
	if (status != -1)
	{
		int index = find_fd_table(table, pos, filename);
		if (index == -1)
		{
			table[pos].filename = filename;
			table[pos].info = info;
			table[pos].counter = 1;
			return 0;
		}
		else
		{
			table[index].counter++;
			return 1;
		}
	}
	else return -2;

}


int main()
{
	struct opened_file table1[10];
	struct file table2[10];
	struct file_descriptor table3[10];
	int size1 = 0, size2 = 0, size3 = 0;

	// Добавление файлов стандартных потоков ввода и вывода в таблицу открытых файлов
	printf("***Implicit opening of stdin***\n");
	add_opened_file(table1, size1++, "stdin", 0, -1);
	
	printf("***Implicit opening of stdout***\n");
	add_opened_file(table1, size1++, "stdout", 1, -1);
	
	printf("***Implicit opening of stderr***\n\n");
	add_opened_file(table1, size1++, "strerr", 2, -1);

	// Печать таблиц
	print_opened_file_table(table1, size1);
	printf("\n");
	print_file_table(table2, size2);
	printf("\n");
	print_fd_table(table3, size3);
	printf("\n\n\n");

	// Открытие первого пользовательского файла
	printf("***Open first user file***\n\n");
	struct stat info;
	int fd1 = open("file1", O_RDONLY);
	if (fd1 == -1)
	{
		perror("file1");
		return errno;

	}
	int status = stat("file1", &info);
	if (status == -1)
	{
		perror("file1");
		return -1;
	}

	// Добавление файла в таблицы
	add_opened_file(table1, size1++, "file1", fd1, size2);
	add_file(table2, size2++, "file1", info.st_ino, "read", 1, size3);
	if(add_fd(table3, size3++, "file1") == -1)
	{
		perror("file1");
		return -1;
	}

	// Печать таблиц
	print_opened_file_table(table1, size1);
	printf("\n");
	print_file_table(table2, size2);
	printf("\n");
	print_fd_table(table3, size3);
	printf("\n\n\n");

	// Открытие второго пользовательского файла
	printf("***Open second user file***\n\n");
	int fd2 = open("file2", O_WRONLY);
	if (fd2 == -1)
	{
		perror("file2");
		return errno;
	}
       	status = stat("file2", &info);
	if (status == -1)
	{
		perror("file2");
		return errno;
	}

	// Добавление файла в таблицы
	add_opened_file(table1, size1++, "file2", fd2, size2);
	add_file(table2, size2++, "file2", info.st_ino, "write", 1, size3);
	if(add_fd(table3, size3++, "file2") == -1)
	{
		perror("file2");
		return errno;
	}

	// Печать таблиц
	print_opened_file_table(table1, size1);
	printf("\n");
	print_file_table(table2, size2);
	printf("\n");
	print_fd_table(table3, size3);
	printf("\n\n\n");

	// Открытие первого пользовательского файла
	printf("***Open third user file***\n\n");
	int fd3 = open("file1", O_RDWR);
	if (fd3 == -1)
	{
		perror("file1");
		return errno;
	}
	status = stat("file1", &info);
	if (status == -1)
	{
		perror("file1");
		return errno;
	}

	// Добавление файла в таблицы
	add_opened_file(table1, size1++, "file1", fd3, size2);
	int index = find_fd_table(table3, size3,  "file1");
	add_file(table2, size2++, "file1", info.st_ino, "rd/wr", 1, index);
	if(add_fd(table3, size3, "file1") == -1)
	{
		perror("file1");
		return errno;
	}

	// Печать таблиц
	print_opened_file_table(table1, size1);
	printf("\n");
	print_file_table(table2, size2);
	printf("\n");
	print_fd_table(table3, size3);
	printf("\n\n\n");
	return 0;
}
