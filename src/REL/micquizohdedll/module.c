/* Microphone quiz module initialization and shutdown. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern void ObjectSetup(void);

/* Run static constructors, then start the quiz. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

extern const VoidFunc _dtors[];

/* Run static destructors when the module exits. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
