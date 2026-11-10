// Module initialization and shutdown.
typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void ObjectSetup(void);

// Runs module constructors before the board code initializes.
int _prolog(void)
{
    const VoidFunc *constructor = _ctors;

    while (*constructor != 0) {
        (**constructor)();
        constructor++;
    }
    ObjectSetup();
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
