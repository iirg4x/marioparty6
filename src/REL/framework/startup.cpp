// REL startup hooks run static constructors and destructors around minigame setup.
static const char rcsid[] = "$Id: startup.cpp,v 1.16.2.6 2004/06/23 07:40:41 hanamasu Exp $";

typedef void (*Procedure)();

extern "C" {
extern Procedure _ctors[];
extern Procedure _dtors[];
void ObjectSetup(void);
void _prolog(void);
void _epilog(void);
}

static void invokeProcedures(Procedure *procedures);

// The REL manager calls this after linking an overlay; run its constructors before minigame setup.
extern "C" void _prolog(void)
{
    invokeProcedures(_ctors);
    ObjectSetup();
}

// The REL manager calls this while unloading an overlay; run its registered destructors.
extern "C" void _epilog(void)
{
    invokeProcedures(_dtors);
}

// Called by the REL entry points to run constructor or destructor table entries in order.
static void invokeProcedures(Procedure *procedures)
{
    Procedure procedure;
    while ((procedure = *procedures++) != 0) {
        procedure();
    }
}
