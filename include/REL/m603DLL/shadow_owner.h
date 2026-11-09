#ifndef M603_SHADOW_OWNER_H
#define M603_SHADOW_OWNER_H
#include "REL/m603DLL/callback_owner.h"
#include "REL/framework/minigame_singleton.h"
#define M603_OWNED_MODEL_DECLARATION
#include "REL/m603DLL/model.h"
class ShadowModelOwner : public CallbackOwner,
                             public MinigameSingleton<ShadowModelOwner> {
public:
 virtual ~ShadowModelOwner(); virtual void update(unsigned long); virtual void extra();
 OwnedModel firstModel,shadowModel,animatedModel; s16 cameraModels[4]; s16 motion;
 ShadowModelOwner(bool);
 void fn_1_5914(int modelIndex);
};
#endif
