#include "REL/m652DLL/framework/model.h"
extern "C" {
#include <math.h>
#include "game/hu3d.h"
}

extern unsigned char lbl_1_bss_108;
extern unsigned char lbl_1_bss_109;

class OwnedModel : public Model
{
public:
    short id() const
    {
        return modelId;
    }

    void fn_1_18DB8()
    {
        if (lbl_1_bss_108) fn_1_33950((1 << 14));
        if (lbl_1_bss_109) {
            fn_1_33950(((1 << 30) | (1 << 6)));
            fn_1_33950(((1 << 30) | (1 << 0)));
        }
    }

    void created(short value)
    {
        modelId = value;
        fn_1_18DB8();
    }

};
