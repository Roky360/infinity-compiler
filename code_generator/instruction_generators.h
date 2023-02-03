#ifndef INFINITY_COMPILER_INSTRUCTION_GENERATORS_H
#define INFINITY_COMPILER_INSTRUCTION_GENERATORS_H

#include "code_generator.h"

/** Identifier formatters */
#define LABEL_FORMAT "L_%d"
#define PROC_FORMAT "P_%s"
#define VAR_FORMAT "v_%s"
#define STRING_FORMAT "s_%d"
/** Reserve */
#define RESB " resb %d\n"
#define RESD " resd %d\n"
// label definition in the code
#define SECTION "section .%s\n"
#define LABEL_DEF "%s:\n"
#define GLOBAL "global %s\n"
#define COMMENT "; %s\n"

/** Instructions */
#define MOV "\tmov %s, %s\n"
#define MOVZX "\tmovzx %s, %s\n"
#define MOVSX "\tmovsx %s, %s\n"
#define LEA "\tlea %s, %s\n"
#define XCHG "\txchg %s, %s\n"

#define PUSH "\tpush %s\n"
#define POP "\tpop %s\n"
#define PUSHA "\tpusha\n"
#define POPA "\tpopa\n"
#define CALL "\tcall %s\n"
#define RET_NUM "\tret %d\n"
#define RET "\tret\n"

#define SYSCALL_80H "\tint 0x80\n"

#define ADD "\tadd %s, %s\n"
#define SUB "\tsub %s, %s\n"
#define IMUL "\timul %s\n"
#define IDIV "\tidiv %s\n"
#define INC "\tinc %s\n"

#define DEC "\tdec %s\n"
#define AND "\tand %s, %s\n"
#define OR "\tor %s, %s\n"
#define XOR "\txor %s, %s\n"
#define NEG "\tneg %s\n"
#define NOT "\tnot %s\n"
#define SHL "\tshl %s, %s\n"
#define SHR "\tshr %s, %s\n"

#define CMP "\tcmp %s, %s\n"
#define LOOP "\tloop %s\n"

#define JMP "\tjmp %s\n"
#define JE "\tje %s\n"
#define JNE "\tjne %s\n"
#define JA "\tja %s\n"
#define JAE "\tjae %s\n"
#define JB "\tjb %s\n"
#define JBE "\tjbe %s\n"
#define JNA "\tjna %s\n"
#define JNAE "\tjnae %s\n"
#define JNB "\tjnb %s\n"
#define JG "\tjg %s\n"
#define JGE "\tjge %s\n"
#define JL "\tjl %s\n"
#define JLE "\tjle %s\n"
#define JNL "\tjnl %s\n"
#define JNG "\tjng %s\n"
#define JNGE "\tjnge %s\n"
#define JNLE "\tjnle %s\n"

#define SETE "\tsete %s\n"
#define SETZ "\tsetz %s\n"
#define SETNZ "\tsetnz %s\n"
#define SETNE "\tsetne %s\n"
#define SETG "\tsetg %s\n"
#define SETGE "\tsetge %s\n"
#define SETL "\tsetl %s\n"
#define SETLE "\tsetle %s\n"

char *generate_label();

char *get_proc_name_formatted(char *proc_name);

char *get_var_name_formatted(char *var_name);

#endif //INFINITY_COMPILER_INSTRUCTION_GENERATORS_H
