#ifndef REL_M634DLL_ACTMODELS_H
#define REL_M634DLL_ACTMODELS_H
#include <memory>
#include "REL/framework/model.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/hu3d.h"
extern char *lbl_1_data_34[4];
}
struct ActModels : MinigameSingleton<ActModels> {
    std::auto_ptr<OwnedModel> models[4];
    ActModels();
    ~ActModels();
    u8 fn_1_2C0(s32 index, Vec *position, Vec *rotation);
};
#endif
