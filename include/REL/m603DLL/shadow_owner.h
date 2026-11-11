#ifndef M603_SHADOW_OWNER_H
#define M603_SHADOW_OWNER_H
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}
class ShadowModelOwner : public RequestObject,
                             public MinigameSingleton<ShadowModelOwner> {
public:
 virtual ~ShadowModelOwner(); virtual void flagEnabled(int); virtual void update();
 OwnedModel firstModel,shadowModel,animatedModel; s16 cameraModels[4]; s16 motion;
 ShadowModelOwner(bool);
 void fn_1_5914(int modelIndex);
};
#endif
