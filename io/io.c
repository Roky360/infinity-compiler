#include "io.h"
#include "../logging/logging.h"
#include "../config/console_colors.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Chunk size for reading files
#define CHUNK_SIZE 128

unsigned long file_size(const char *file_name) {
    // opening the file in read mode
    unsigned long len;
    FILE *fp = fopen(file_name, "r");

    // checking if the file exist
    if (!fp)
        return -1;

    fseek(fp, 0, SEEK_END);
    // calculate the size of the file in bytes
    len = ftell(fp);

    fclose(fp);
    return len;
}

char *read_file(const char *filename) {
    FILE *fp;
    char *content;
    unsigned long flen;
    char chunk[CHUNK_SIZE];
    chunk[CHUNK_SIZE - 1] = '\0';

    // open file for reading
    fp = fopen(filename, "r");
    if (!fp) {
#ifdef INF_SHOW_COLORS
        log_error(IO, "Failed to open file " UNDERLINE "%s" RESET RED_B ". It may not exist.", filename);
#else
        log_error(IO, "Failed to open file %s. It may not exist.", filename);
#endif
        exit(1);
    }

    // get file length and allocate memory for content buffer
    flen = file_size(filename);
    content = malloc(flen + 1);
    if (!content) {
        fclose(fp);
#ifdef INF_SHOW_COLORS
        log_error(IO, "Failed to allocate memory for file buffer "
                      "(file path: " UNDERLINE "%s" RESET RED_B ")", filename);
#else
        log_error(IO, "Failed to allocate memory for file buffer (file path: %s)", filename);
#endif
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

char *get_file_extension(const char *filename) {
    char *p = strrchr(filename, '.');
    return p ? p + 1 : "";
}

const char *get_file_name(const char *path) {
    const char *p = path + strlen(path) - 1;
    while (*--p != '/' && *p != '\\')
        if (p == path) // if file name does not contain slashes
            return path;
    return p + 1;
}

char *alsprintf(char **stream, const char *format, ...) {
    va_list args;
    va_start(args, format);

    *stream = malloc(vsnprintf(NULL, 0, format, args) + 1);
    vsprintf(*stream, format, args);

    va_end(args);
    return *stream;
}

char *change_file_extension(char *filename, const char *new_ext) {
    char *p, *ext_p;
    size_t filename_new_len, new_ext_len;
    ext_p = get_file_extension(filename);
    new_ext_len = strlen(new_ext);
    filename_new_len = strlen(filename) - strlen(ext_p) + new_ext_len;

    filename = realloc(filename, filename_new_len + 1);
    p = filename + filename_new_len - new_ext_len;
    strcpy(p, new_ext);

    return filename;
}

void write_to_file(FILE *fp, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(fp, format, args);
    va_end(args);
}
