/* Start the Gondola Glide scene after module initialization and run module destructors on exit. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

/* Invoke registered constructors, then start the minigame scene. */
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

/* Invoke registered destructors as the module leaves the game. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
