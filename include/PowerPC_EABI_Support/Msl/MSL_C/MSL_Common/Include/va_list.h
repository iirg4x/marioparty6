#ifndef MP6_C_VA_LIST_H
#define MP6_C_VA_LIST_H
/* EABI variadic traversal tracks argument registers and stack storage. */
typedef struct mp6_argument_cursor {
    unsigned char integer_register;
    unsigned char floating_register;
    char *stack_arguments;
    char *saved_registers;
} va_list[1];
#ifdef __cplusplus
namespace std { using ::va_list; }
#endif
#endif
