#ifndef INFINITY_COMPILER_COMPILER_H
#define INFINITY_COMPILER_COMPILER_H

/// Compiles a source code as a string in the `src` parameter, into the output_path
/// \param src The source code as a string
/// \param output_path Output asm path
void compiler_compile(char *src, char *output_path);

/// Reads file contents from the `input_path` and compiles it. The generated ASM file will be at the `output_path`.
/// Uses the compiler_compile function.
/// Also measures the time taken to compile the whole file and logs it.
/// \param input_path Target file path to be compiled
/// \param output_path Output ASM path
void compiler_compile_file(const char *input_path, char *output_path);

#endif //INFINITY_COMPILER_COMPILER_H
