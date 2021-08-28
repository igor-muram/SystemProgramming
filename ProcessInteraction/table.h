#ifndef TABLE_H
#define TABLE_H

#include <stdio.h>
#include <string.h>

#define TABLE_SIZE 100
#define STRING_SIZE 1024

struct table
{
    char data[TABLE_SIZE][STRING_SIZE];
    int max_size;
    int size;

    int (*contains)(struct table *t, char *string);
    int (*put)(struct table *t, char *string);
    void (*print)(struct table *t);
};

int contains(struct table *t, char *string)
{
    if (strlen(string) >= STRING_SIZE)
        return -1;

    for (int i = 0; i < t->size; i++)
        if (!strcmp(t->data[i], string))
            return -2;

    return 0;
}

int put(struct table *t, char *string)
{
    if (strlen(string) >= STRING_SIZE)
        return -1;

    if (t->size == t->max_size)
        return -2;

    strcpy(t->data[t->size], string);
    (t->size)++;

    return 0;
}

void print(struct table *t)
{
    printf("table:\n");
    for (int i = 0; i < t->size; i++)
        printf("%s\n", t->data[i]);
}
void init(struct table *t)
{
    t->size = 0;
    t->max_size = TABLE_SIZE;
    t->contains = contains;
    t->put = put;
    t->print = print;
}

#endif