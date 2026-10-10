/* Starts and stops the minigame REL around its constructor and destructor lists. */
typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern void fn_1_128C(void);

/* The REL loader calls this on entry; run registered constructors before initializing minigame
 * state. */
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

/* The REL loader calls this on unload; run registered destructors in their stored order. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;

    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
