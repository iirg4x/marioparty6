/* Entry points for loading and unloading the M632 minigame. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_3934(void);

/* Module-load entry point: runs registered constructors, then initializes the M632 scene. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_3934();
    return 0;
}

/* Module-unload entry point: runs the registered shutdown routines. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
