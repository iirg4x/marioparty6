/* Starts and stops the tutorial board module's static initialization. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
void fn_1_A0(void);

/* The module loader calls this to run constructors and start tutorial board setup. */
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

extern const VoidFunc _dtors[];

/* The module loader calls this to run destructors when the tutorial module unloads. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
