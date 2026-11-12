#ifndef REL_M604DLL_STAGE_H
#define REL_M604DLL_STAGE_H
#include <memory>
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "humath.h"
#include "game/hu3d.h"
struct Stage : RequestObject, MinigameSingleton<Stage> {
    s16 hookId;
    s32 lightId;
    std::auto_ptr<OwnedModel> floorModel;
    std::auto_ptr<OwnedModel> backModel;
    std::auto_ptr<OwnedModel> heightModels[13];
    std::auto_ptr<OwnedModel> movingModels[3];
    u8 heightStates[13];
    std::auto_ptr<OwnedModel> shadowModel;
    std::auto_ptr<OwnedModel> shadowCopies[4];
    Vec targetPositions[4];
    Vec movingPositions[3];
    f32 angles[3];

    Stage();
    virtual ~Stage();
    virtual void flagEnabled(int bit);
    virtual void update();
    void draw(Mtx *matrix);
    static void drawCallback(HU3D_MODEL *model, Mtx *matrix);
};
#endif
