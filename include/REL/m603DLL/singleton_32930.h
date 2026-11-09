#ifndef REL_M603DLL_SINGLETON_32930_H
#define REL_M603DLL_SINGLETON_32930_H

extern "C" {
#include "game/object.h"
#include "REL/framework/assertion.h"
}

class Singleton32930Base;
extern "C" {
extern Singleton32930Base *lbl_1_bss_28;
extern char lbl_1_data_D20[];
extern char lbl_1_data_D2A[];
extern char lbl_1_data_D36[];
void fn_1_3A764(void);
}

class Singleton32930Base {
public:
    Singleton32930Base()
    {
        !lbl_1_bss_28
            ? (void)0
            : __msl_assertion_failed(lbl_1_data_D20, lbl_1_data_D2A, 50);
        lbl_1_bss_28 = this;
    }

    ~Singleton32930Base() { lbl_1_bss_28 = 0; }

    static Singleton32930Base &getUnchecked() { return *lbl_1_bss_28; }
    static Singleton32930Base &getChecked()
    {
        (lbl_1_bss_28 != 0)
            ? (void)0
            : __msl_assertion_failed(lbl_1_data_D36, lbl_1_data_D2A, 38);
        return *lbl_1_bss_28;
    }
};

class Singleton32930View : public Singleton32930Base {
public:
    virtual ~Singleton32930View();
    virtual void virtualAt0C(OMOBJMAN *) = 0;
    virtual void fn_1_32CAC();
    virtual void virtualAt14() = 0;

    int values[4];
    int messageId;
    int counter;

    Singleton32930View();
    void fn_1_32C40();

    int decrementCounter()
    {
        int expired = 0;
        if (counter > 0) {
            if ((counter -= 1) == 0)
                expired = 1;
        }
        return expired;
    }
};

#endif
