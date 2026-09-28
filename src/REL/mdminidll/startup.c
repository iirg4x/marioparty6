typedef void (*MDMinidllVoidFunc)(void);
extern const MDMinidllVoidFunc _ctors[];
extern const MDMinidllVoidFunc _dtors[];
void fn_1_16F28(void);

int _prolog(void)
{
    const MDMinidllVoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_16F28();
    return 0;
}

void _epilog(void)
{
    const MDMinidllVoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
