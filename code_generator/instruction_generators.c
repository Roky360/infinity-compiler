#include "instruction_generators.h"
#include "../io/io.h"

char *generate_label() {
    static int label_id = 0;
    char *label;
    return alsprintf(&label, LABEL_FORMAT, label_id++);
}

char *get_proc_name_formatted(char *proc_name) {
    char *proc_formatted;
    return alsprintf(&proc_formatted, PROC_FORMAT, proc_name);
}

char *get_var_name_formatted(char *var_name) {
    char *var_formatted;
    return alsprintf(&var_formatted, VAR_FORMAT, var_name);
}
