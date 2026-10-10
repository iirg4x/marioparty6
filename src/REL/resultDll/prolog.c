/* Runs result setup before the results overlay starts. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
void fn_1_A0(void);

/* Runs setup functions before the results overlay begins. */
int _prolog(void) {
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_A0();
    return 0;
}

extern const VoidFunc _dtors[];

/* Runs cleanup functions when the results overlay closes. */
void _epilog(void) {
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}
