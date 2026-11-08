#ifndef M622_SCENE_MODELS_H
#define M622_SCENE_MODELS_H

#include "dolphin/types.h"
#include "REL/m622DLL/owned_model.h"

struct ReleasedModelA1C : ReleasedModel {
    ~ReleasedModelA1C() {}
};
#include "REL/m622DLL/framework/linknode.h"

struct SceneModelNode : Node {
    SceneModelNode() { }
};

static inline s16 scene_model_id(const s16 *model)
{
    return *model;
}

struct ModelVector {
    f32 values[3];
};

typedef ModelVector ModelVector1A7C;

typedef ModelVector ModelCameraVector;

struct ModelCameraPlacement;
struct ModelPulseOwner;
struct MovingPair;
struct FourModelOwner;

class ModelEntry : public SceneModelNode {
public:
    static void fn_1_1BD8();
    typedef void (ModelEntry::*Callback)();
    Callback initialData;
    u8 active;
    s32 requestId;
    u8 unknown_1C[4]; /* No code names this field. */
    s32 frame;
    ModelVector frameData;
    s16 characterModelId;
    s16 itemHookId;
    OwnedModel model;
    ModelCameraPlacement *model0;
    ModelPulseOwner *model1;
    MovingPair *model2;
    FourModelOwner *model3;

    ModelEntry(const void *, const ModelVector1A7C *, s32);
    ~ModelEntry() {}
    void fn_1_1CF4();
    bool fn_1_1F5C();
    s32 fn_1_1F84(s16 characterModelId, s16 itemId);
    s32 fn_1_20C0();
    void fn_1_217C();
    void fn_1_22C0();
    void fn_1_2368(ModelCameraPlacement *, ModelPulseOwner *, MovingPair *, FourModelOwner *);
};

struct ModelPairNode : Node {
    ModelPairNode() {}
    ~ModelPairNode();
};

struct ModelPairOwner : ModelPairNode {
    static void fn_1_24D8();
    s32 requestId;
    ModelVector vector;
    OwnedModel firstModel;
    OwnedModel secondModel;

    ModelPairOwner(const s16 *, const s16 *, const ModelVector *, u32);
    ~ModelPairOwner() {}
    void fn_1_2618();
};

struct ModelCameraNode : Node {
    ModelCameraNode() {}
    ~ModelCameraNode();
};

struct ModelCameraPlacement : ModelCameraNode {
    static void fn_1_2AFC();
    typedef void (ModelCameraPlacement::*Callback)();
    Callback initialValues;
    u8 active;
    s32 mode;
    u32 unknownPadding;
    ModelCameraVector cameraPosition;
    f32 modelScale;
    f32 modelOffset;
    OwnedModel model;

    ModelCameraPlacement(void *, const ModelCameraVector *, s32);
    ~ModelCameraPlacement() {}
    void fn_1_2C18();
    void fn_1_2E08();
    void fn_1_2EAC();
};

struct ModelPulseNode : Node {
    ModelPulseNode() {}
    ~ModelPulseNode();
};

struct ModelPulseOwner : ModelPulseNode {
    static void fn_1_3374();
    typedef void (ModelPulseOwner::*CallbackState)(void);
    CallbackState callback;
    u8 initialized;
    s32 requestId;
    u8 unknown_1C[4]; /* No code names this field. */
    ModelVector frameData;
    f32 scale;
    OwnedModel model;

    ModelPulseOwner(const void *, const ModelVector *, s32);
    ~ModelPulseOwner() {}
    void fn_1_3490();
    void fn_1_3678();
    void fn_1_36C4();
};

struct MovingPairNode : Node {
    MovingPairNode() {}
    ~MovingPairNode();
};

struct MovingPair : MovingPairNode {
    static void fn_1_458C();
    typedef void (MovingPair::*Callback)();
    Callback callback;
    u8 initialized;
    s32 requestId;
    u8 unknown_1C[4]; /* No code names this field. */
    ModelVector frameData;
    u8 unknown2C[4]; /* No code names this field. */
    OwnedModel frontModel;
    OwnedModel backModel;

    MovingPair(const void *, const void *, const ModelVector *, s32);
    ~MovingPair() {}
    void fn_1_46CC(void);
    void fn_1_49CC();
    void fn_1_4A70();
};

struct FourModelNode : Node {
    FourModelNode() {}
    ~FourModelNode();
};

struct FourModelOwner : FourModelNode {
    static void fn_1_3B08();
    typedef void (FourModelOwner::*Callback)();
    Callback callback;
    u8 initialized;
    s32 requestId;
    u8 unknown_1C[4]; /* No code names this field. */
    ModelVector frameData;
    f32 scale;
    s32 state;
    u32 unknown34;
    OwnedModel model0, model1, model2, model3;

    FourModelOwner(const void *, const void *, const void *, const void *,
                   const ModelVector *, s32);
    ~FourModelOwner() {}
    void fn_1_3C90();
    void fn_1_3FE0();
    void fn_1_40A0();
};

template <class T> struct SceneList : Node {
    bool empty() const { return link0 == this && link0 == link4; }
    Node *last_node() { return link4; }
    T *back() { return static_cast<T *>(last_node()); }
    Node *first_node() { return link0; }
    T *begin() { return static_cast<T *>(first_node()); }
    T *end() { return static_cast<T *>(static_cast<Node *>(this)); }
    static T *next(T *node) { return static_cast<T *>(node->link0); }
    void visit(void (*visitor)(T *));
};

extern "C" {
ModelEntry *fn_1_1DB8(s32 ownerId);
s32 fn_1_1E68(s32 ownerId);
}

#endif
