/* Runs C++ static constructors and the minigame startup hook on load. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[], _dtors[];
void fn_1_A0(void);
/* The runtime invokes this entry point to run constructors before setup. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) { (*ctor)(); ctor++; }
    fn_1_A0();
    return 0;
}
/* The runtime invokes this entry point to run registered destructors on unload. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) { (*dtor)(); dtor++; }
}
