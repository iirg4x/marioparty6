/* Entry and exit points the REL loader calls: the module's static constructors and destructors,
   and its setup. */
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_1F98(void);

/* Called when the module is linked: runs every static constructor, then the module's setup. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_1F98();
    return 0;
}

/* Called when the module is unlinked: runs every static destructor. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}
