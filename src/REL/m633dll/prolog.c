typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern void fn_1_128C(void);

int _prolog(void)
{
    const VoidFunc *ctor = _ctors;

    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }

    fn_1_128C();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;

    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
