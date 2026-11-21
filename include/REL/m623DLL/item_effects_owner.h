#ifndef REL_M623DLL_ITEM_EFFECTS_OWNER_H
#define REL_M623DLL_ITEM_EFFECTS_OWNER_H
#include <memory>
#include "REL/framework/singleton.h"
#include "REL/framework/list_object.h"
#include "REL/framework/model.h"
#include "REL/m623DLL/item_effects_state.h"
#include "REL/m623DLL/scene_collision.h"

// Owns the coin, dollar and spiny models and their per-view item records.
class ItemEffectsOwner : public RequestObject, public MinigameSingleton<ItemEffectsOwner> {
  public:
    ItemEffectsOwner(int type);
    virtual ~ItemEffectsOwner();
    virtual void flagEnabled(int request);
    virtual void update();

    ItemEffectsState records[2][17];
    std::auto_ptr<TriangleCollision> collision;
    OwnedModel coinModel;
    OwnedModel dollarModel;
    OwnedModel spinyModel;
    OwnedModel coinLayout;
    OwnedModel dollarLayout;
    OwnedModel spinyLayout;
    short hookIds[2];

    // Handles contact with one item and starts its motion.
    void fn_1_715C(ItemEffectsState *state, int viewIndex);
    // Advances one item's hit motion and fade.
    void fn_1_788C(ItemEffectsState *state);
    // Places an item model and its ground circle in one view.
    void setupEffect(ItemEffectsState *state, int mode, short model, Point3d *position,
                     int viewIndex, f32 heightOffset);
};

// Handles the first active item in range and returns its kind.
int fn_1_7210(int viewIndex, const Vec *position, f32 radius, Vec *result);

// Finds the nearest active spiny within 5,000 units along X.
u8 fn_1_7534(int viewIndex, const Vec *position, Vec *result);

// Finds the nearest active coin or dollar within 5,000 units along X.
u8 fn_1_76E0(int viewIndex, const Vec *position, Vec *result);

#endif
