#ifndef SINGLETON_32930_H
#define SINGLETON_32930_H

extern "C" {
#include "game/object.h"
}

class Singleton32930Base;
extern "C" {
extern Singleton32930Base *lbl_1_bss_164;
extern char lbl_1_data_2680[];
extern char lbl_1_data_268A[];
extern char lbl_1_data_2696[];
void __msl_assertion_failed(const char *, const char *, int);
void fn_1_3A764(void);
}

class Singleton32930Base {
public:
    Singleton32930Base()
    {
        !lbl_1_bss_164
            ? (void)0
            : __msl_assertion_failed(lbl_1_data_2680, lbl_1_data_268A, 50);
        lbl_1_bss_164 = this;
    }

#ifdef SINGLETON32930_INLINE_BASE_DESTRUCTOR
    ~Singleton32930Base() { lbl_1_bss_164 = 0; }
#else
    ~Singleton32930Base();
#endif

    static Singleton32930Base &getUnchecked() { return *lbl_1_bss_164; }
    static Singleton32930Base &getChecked()
    {
        (lbl_1_bss_164 != 0)
            ? (void)0
            : __msl_assertion_failed(lbl_1_data_2696, lbl_1_data_268A, 38);
        return *lbl_1_bss_164;
    }
};

class Singleton32930View : public Singleton32930Base {
public:
    virtual ~Singleton32930View();
    virtual void virtualAt0C(OMOBJMAN *) = 0;
    virtual void fn_1_32CAC();
    virtual void virtualAt14() = 0;

    int signedSlots_04_10[4];
    int messageId_14;
    int counter_18;

    Singleton32930View();
    void fn_1_32C40();

    int decrementCounter()
    {
        int expired = 0;
        if (counter_18 > 0) {
            if ((counter_18 -= 1) == 0)
                expired = 1;
        }
        return expired;
    }
};

#endif
