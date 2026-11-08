#ifndef M648DLL_SINGLETON_32930_H
#define M648DLL_SINGLETON_32930_H

extern "C" {
#include "game/object.h"
#include "REL/framework/assertion.h"
}

class Singleton32930Base;
extern "C" {
extern Singleton32930Base *lbl_1_bss_30;
extern char lbl_1_data_946[];
extern char lbl_1_data_93A[];
}

class Singleton32930Base {
public:
#ifdef SINGLETON32930_INLINE_BASE_DESTRUCTOR
    ~Singleton32930Base() { lbl_1_bss_30 = 0; }
#else
    ~Singleton32930Base();
#endif

    static Singleton32930Base &getChecked()
    {
        (lbl_1_bss_30 != 0)
            ? (void)0
            : __msl_assertion_failed(lbl_1_data_946, lbl_1_data_93A, 38);
        return *lbl_1_bss_30;
    }
};

class Singleton32930View : public Singleton32930Base {
public:
    virtual ~Singleton32930View();
    virtual void virtualAt0C(OMOBJMAN *) = 0;
    virtual void fn_1_32CAC();
    virtual void virtualAt14() = 0;

    int signedValues[4];
    int messageId_14;
    int counter_18;
};

#endif
