#ifndef MP6_C_VA_LIST_H
#define MP6_C_VA_LIST_H
/* EABI variadic traversal tracks argument registers and stack storage. */
typedef struct mp6_argument_cursor {
    // Index of the next saved integer argument register.
    unsigned char integer_register;
    // Index of the next saved floating-point argument register.
    unsigned char floating_register;
    // Cursor for variadic arguments fetched from the caller's stack.
    char *stack_arguments;
    // Base of the register-save area used during variadic traversal.
    char *saved_registers;
} va_list[1];
#ifdef __cplusplus
namespace std { using ::va_list; }
#endif
#endif
