// Module initialization and shutdown.
typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_A0(void);

// Runs module constructors before the board code initializes.
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;

    while (*constructor != 0) {
        (**constructor)();
        constructor++;
    }
    fn_1_A0();
    return 0;
}

// Runs module destructors when the board module shuts down.
void _epilog(void)
{
    const VoidFunc *destructor = _dtors;

    while (*destructor != 0) {
        (**destructor)();
        destructor++;
    }
}
