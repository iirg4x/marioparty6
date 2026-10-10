/* Runs Odd Card Out startup constructors and its module initialization hook. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* Called by module startup to run static initializers and initialize Odd Card Out. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* Runs Odd Card Out shutdown destructors when the module is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
