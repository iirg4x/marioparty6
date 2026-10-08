/* Starts and shuts down the instruction-screen module. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* Called by the module loader on screen load to run constructors and initialize it. */
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;
    while (*constructor != 0) {
        (*constructor)();
        constructor++;
    }
    fn_1_A0();
    return 0;
}

/* Called by the module loader on unload to run registered destructors. */
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;
    while (*destructor != 0) {
        (*destructor)();
        destructor++;
    }
}
