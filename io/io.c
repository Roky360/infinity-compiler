#include "io.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Chunk size for reading files
#define CHUNK_SIZE 128

/*
Gets the size of the file in bytes.
Sets the file pointer to the beginning of the file.
Returns the length of the file.
*/
unsigned long file_size(const char *file_name) {
    // opening the file in read mode
    unsigned long len;
    FILE *fp = fopen(file_name, "r");

    // checking if the file exist or not
    if (fp == NULL)
        return -1;

    fseek(fp, 0L, SEEK_END);

    // calculating the size of the file
    len = ftell(fp);

    fclose(fp);
    return len;
}

/*
Reads a file in text mode and return its contents as (char *).
Reads in chunks. CHUNK_SIZE is the size of each chunk.
*/
char *read_file(const char *filename) {
    FILE *fp;
    char *content;
    unsigned long flen;
    // unsigned long bytes_read = 0;
    // unsigned short reading_amount;
    char chunk[CHUNK_SIZE];
    chunk[CHUNK_SIZE - 1] = '\0';

    // open file for reading
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Error opening file \"%s\". It may does not exist.\n", filename);
        exit(1);
    }

    // get file length and allocate memory for content buffer
    flen = file_size(filename);
    content = malloc(flen + 1);
    if (!content) {
        printf("Error allocating memory\n");
        fclose(fp);
        return NULL;
    }
    content[0] = '\0';

    // read file into the buffer
    while (fgets(chunk, CHUNK_SIZE - 1, fp) != NULL) {
        strcat(content, chunk);
    }

    content[flen] = '\0';

    fclose(fp);
    return content;
}

char *get_file_extension(char *filename) {
    char *p = filename + strlen(filename) - 1;
    while (*--p != '.')
        if (p == filename) // if file name does not contain extension
            return "";
    return p + 1;
}

char *alsprintf(char **stream, char *format, ...) {
//    int res;
    va_list args;
    va_start(args, format);

    *stream = malloc(vsnprintf(NULL, 0, format, args) + 1);
    /*res =*/ vsprintf(*stream, format, args);

    va_end(args);
    return *stream;
}
