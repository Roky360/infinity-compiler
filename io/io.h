#ifndef INFINITY_COMPILER_IO_H
#define INFINITY_COMPILER_IO_H

#include <stdio.h>

/// Gets the size of the file in bytes.
/// Sets the file pointer to the beginning of the file.
/// \param file_name File path to work with
/// \return The length of the file, in bytes.
unsigned long file_size(const char *file_name);

/// Reads a file in text mode and return its contents as (char *).
/// Reads in chunks.
/// \param filename File path to read.
/// \return Contents of the file, as allocated string.
char *read_file(const char *filename);

/// Returns the extension of a file, based on its path
/// \param filename File path
/// \return File extension if present, or empty string ("") if the file has no extension.
char *get_file_extension(const char *filename);

/// Returns file name from a path.
/// For example: for this input "/files/test.txt", will return "test.txt"
/// \param path File path
/// \return File name.
const char *get_file_name(const char *path);

/// Inserts a formatted string to a memory address pointer.
/// Calculates the memory required for the string, allocates this memory to `stream`
/// and inserts the formatted string to it.
/// \param stream A pointer to a non-allocated string
/// \param format String format
/// \param ... More arguments
/// \return The formatted string
char *alsprintf(char **stream, const char *format, ...);

/// Changes file extension on a path. DOES NOT returns new string, but changes the original path.
/// \param filename File path to work with.
/// \param new_ext New extension to change to.
/// \return File path with updated extension
char *change_file_extension(char *filename, const char *new_ext);

/// Writes a formatted string to a pre-opened file for writing.
/// \param fp File instance to write to. The file has to be opened for writing.
/// \param format String format
/// \param ... Additional arguments
void write_to_file(FILE *fp, const char *format, ...);

#endif //INFINITY_COMPILER_IO_H
