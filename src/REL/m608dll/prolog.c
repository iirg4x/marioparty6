/* Supplies the module loader's constructor and scene-start entry points. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_43C8(void);

/* Called by the module loader on entry; runs constructors before scene setup. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_43C8();
    return 0;
}

/* Called by the module loader on unload to release module-owned state. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
