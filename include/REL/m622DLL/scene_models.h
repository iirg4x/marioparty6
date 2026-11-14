#ifndef M622_SCENE_MODELS_H
#define M622_SCENE_MODELS_H

#include "dolphin/types.h"
#include "REL/framework/model.h"

#include "REL/framework/list.h"

struct ModelCameraPlacement;
struct ModelPulseOwner;
struct MovingPair;
struct FourModelOwner;

class ModelEntry : public ListNode<ModelEntry> {
public:
    static void fn_1_1BD8();
    typedef void (ModelEntry::*Callback)();
    Callback initialData;
    u8 active;
    s32 requestId;
    u8 unknown_1C[4]; /* No code names this field. */
    s32 frame;
    Vec frameData;
    s16 characterModelId;
    s16 itemHookId;
    OwnedModel model;
    ModelCameraPlacement *model0;
    ModelPulseOwner *model1;
    MovingPair *model2;
    FourModelOwner *model3;

    ModelEntry(const OwnedModel &, const Vec *, s32);
    /* Destroys the owned models and unlinks the list node. */
    ~ModelEntry() {}
    void fn_1_1CF4();
    bool fn_1_1F5C();
    s32 fn_1_1F84(s16 characterModelId, s16 itemId);
    s32 fn_1_20C0();
    void fn_1_217C();
    void fn_1_22C0();
    void fn_1_2368(ModelCameraPlacement *, ModelPulseOwner *, MovingPair *, FourModelOwner *);
};

struct ModelPairOwner : ListNode<ModelPairOwner> {
    static void fn_1_24D8();
    s32 requestId;
    Vec vector;
    OwnedModel firstModel;
    OwnedModel secondModel;

    ModelPairOwner(const OwnedModel &, const OwnedModel &, const Vec *, u32);
    /* Destroys the owned models and unlinks the list node. */
    ~ModelPairOwner() {}
    void fn_1_2618();
};

struct ModelCameraPlacement : ListNode<ModelCameraPlacement> {
    static void fn_1_2AFC();
    typedef void (ModelCameraPlacement::*Callback)();
    Callback initialValues;
    u8 active;
    s32 mode;
    u32 unknownPadding;
    Vec cameraPosition;
    f32 modelScale;
    f32 modelOffset;
    OwnedModel model;

    ModelCameraPlacement(const OwnedModel &, const Vec *, s32);
    /* Destroys the owned models and unlinks the list node. */
    ~ModelCameraPlacement() {}
    void fn_1_2C18();
    void fn_1_2E08();
    void fn_1_2EAC();
};

struct ModelPulseOwner : ListNode<ModelPulseOwner> {
    static void fn_1_3374();
    typedef void (ModelPulseOwner::*CallbackState)(void);
    CallbackState callback;
    u8 initialized;
    s32 requestId;
    u8 unknown_1C[4]; /* No code names this field. */
    Vec frameData;
    f32 scale;
    OwnedModel model;

    ModelPulseOwner(const OwnedModel &, const Vec *, s32);
    /* Destroys the owned models and unlinks the list node. */
    ~ModelPulseOwner() {}
    void fn_1_3490();
    void fn_1_3678();
    void fn_1_36C4();
};

struct FourModelOwner : ListNode<FourModelOwner> {
    static void fn_1_3B08();
    typedef void (FourModelOwner::*Callback)();
    Callback callback;
    u8 initialized;
    s32 requestId;
    u8 unknown_1C[4]; /* No code names this field. */
    Vec frameData;
    f32 scale;
    s32 state;
    u32 unusedValue;
    OwnedModel model0, model1, model2, model3;

    FourModelOwner(const OwnedModel &, const OwnedModel &, const OwnedModel &, const OwnedModel &,
                   const Vec *, s32);
    /* Destroys the owned models and unlinks the list node. */
    ~FourModelOwner() {}
    void fn_1_3C90();
    void fn_1_3FE0();
    void fn_1_40A0();
};

struct MovingPair : ListNode<MovingPair> {
    static void fn_1_458C();
    typedef void (MovingPair::*Callback)();
    Callback callback;
    u8 initialized;
    s32 requestId;
    u8 unknown_1C[4]; /* No code names this field. */
    Vec frameData;
    u8 unknown2C[4]; /* No code names this field. */
    OwnedModel frontModel;
    OwnedModel backModel;

    MovingPair(const OwnedModel &, const OwnedModel &, const Vec *, s32);
    /* Destroys the owned models and unlinks the list node. */
    ~MovingPair() {}
    void fn_1_46CC(void);
    void fn_1_49CC();
    void fn_1_4A70();
};

ModelEntry *fn_1_1DB8(s32 ownerId);
s32 fn_1_1E68(s32 ownerId);

#endif
