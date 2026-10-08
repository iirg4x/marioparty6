/* Runs the minigame constructors, starts its sequence, and runs destructors during teardown. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* Runs registered constructors before initializing the minigame sequence. */

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

/* Runs registered destructors when the minigame REL is unloaded. */

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
