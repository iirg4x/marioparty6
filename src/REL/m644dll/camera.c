/* Sets up the two camera views and updates their scene models and transitions. */
#include "REL/m644dll/module_types.h"
#include "game/hu3d.h"
#include "game/object.h"

/* Invisible models carrying the animated CAMgrid and CAMintgrid hooks. */
s32 lbl_1_data_40[7] = {
    DATANUM(DATA_m644, 0), DATANUM(DATA_m644, 1), DATANUM(DATA_m644, 2), DATANUM(DATA_m644, 3),
    DATANUM(DATA_m644, 4), DATANUM(DATA_m644, 5), DATANUM(DATA_m644, 6)
};

void fn_1_1124(s32);
s32 fn_1_913C(s32);

s32 fn_1_559C(s16);
s32 fn_1_56D4(s32);

/* Rail selection and split-screen transition state shared by camera callbacks. */
M644EffectMenuData lbl_1_bss_8;

typedef struct SceneModelIds {
    /* Camera rail selected by each route-motion index; -1 skips boarding segments. */
    s32 rail_selector[12];
} SceneModelIds;

extern void fn_1_11F0(void);
extern void fn_1_C44(OMOBJ *obj);

extern void fn_1_9664(void);
extern void fn_1_EE0(OMOBJ *object);
typedef struct M644CameraIdPair_s {
    /* One value per team view: mask or viewport dimension, as selected by the caller. */
    s32 per_team[2];
} M644CameraIdPair;

extern OMOBJMAN *fn_1_A0(void);
extern void fn_1_C0C(OMOBJ *object);
extern void fn_1_1180(s32 index);

extern void fn_1_11F0(void);
extern f32 fn_1_9178(s32 entry_index);

extern void fn_1_94A4(s32 index);
extern s16 fn_1_9114(void);
extern void fn_1_1060(OMOBJ *object);

typedef struct M644CameraIds_s {
    /* One value per team view: mask or viewport dimension, as selected by the caller. */
    s32 per_team[2];
} M644CameraIds;

extern void fn_1_80D4(s32 cameraIndex, Mtx modelMatrix);

extern s32 fn_1_224(void);
extern void fn_1_9614(void);

s32 fn_1_367CC(s32 *player);

void fn_1_1690(void);
void fn_1_590(void);

void fn_1_5A8(void);
void fn_1_690(s32 index, s32 selector);
BOOL fn_1_6F8(s32 team_index);
float fn_1_760(s32 index);
s32 fn_1_7C8(s32 entry_index);
void fn_1_848(void);
void fn_1_C0C(OMOBJ *object);
void fn_1_C44(OMOBJ *object);
void fn_1_EE0(OMOBJ *object);
void fn_1_1060(OMOBJ *object);
void fn_1_1124(s32 team_index);
void fn_1_1180(s32 index);
void fn_1_11F0(void);
void fn_1_134C(void);
void fn_1_1510(void);

/* Loads seven invisible animated camera rails for each team during camera setup. */
void fn_1_5A8(void)
{
    s32 group_index;
    s32 model_index;
    for (group_index = 0; group_index < 2; group_index++) {
        for (model_index = 0; model_index < 7; model_index++) {
            lbl_1_bss_8.camera_models[group_index][model_index] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_40[model_index], HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelAttrSet(lbl_1_bss_8.camera_models[group_index][model_index],
                             HU3D_MOTATTR_PAUSE);
            Hu3DModelTPLvlSet(lbl_1_bss_8.camera_models[group_index][model_index], 0.0f);
        }
    }
}

/* Selects and unpauses a team's camera rail when the route controller changes travel segments. */
void fn_1_690(s32 index, s32 selector)
{
    struct SelectorPair {
        /* First selector preset; this pair is not read by the selection helper. */
        u32 first_selector;
        /* Second selector preset. */
        u32 second_selector;
    } model_selectors = {1, 2};

    lbl_1_bss_8.selector[index] = selector;
    fn_1_1180(index);
}

/* Reports whether the selected camera rail has reached the end of its motion. */
BOOL fn_1_6F8(s32 team_index) {
    s16 selector = lbl_1_bss_8.selector[team_index];
    return Hu3DMotionEndCheck(lbl_1_bss_8.camera_models[team_index][selector]);
}

/* Returns the selected camera rail's motion frame for the requested team. */
float fn_1_760(s32 index)
{
    s16 selector;
    selector = lbl_1_bss_8.selector[index];
    return Hu3DMotionTimeGet(lbl_1_bss_8.camera_models[index][selector]);
}

/* Maps route travel and boarding motions to camera rails; boarding entries return -1. */
s32 fn_1_7C8(s32 entry_index)
{
    SceneModelIds model_ids = {{0, -1, 1, -1, 2, -1, 3, -1, 4, -1, 5, 6}};

    return model_ids.rail_selector[entry_index];
}

/* Creates both cameras and their opening viewports, then starts the split-screen transition
 * object. */
void fn_1_848(void)
{
    M644CameraIdPair camera_ids;
    M644CameraIdPair left;
    M644CameraIdPair top;
    M644CameraIdPair width;
    M644CameraIdPair height;
    OMOBJ *object;
    s32 camera_index;

    M644EffectMenuData *camera_state;

    /* The second view starts with zero width and height and opens during fade-in. */
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(3, 45.0f, 100.0f,
                             160000.0f, 1.2f);
    Hu3DCameraPosSet(3, 0.0f, 1000.0f,
                     2400.0f, 0.0f, 1.0f,
                     0.0f, 0.0f, 0.0f,
                     80000.0f);
    for (camera_index = 0; camera_index < 2; camera_index++) {
        M644CameraIdPair camera_ids = {{1, 2}};
        M644CameraIdPair left = {{0, 0}};
        M644CameraIdPair top = {{0, 0}};
        M644CameraIdPair width = {{640, 0}};
        M644CameraIdPair height = {{480, 0}};
        Hu3DCameraViewportSet(
            camera_ids.per_team[camera_index],
            0.0f, 0.0f,
            (f32)width.per_team[camera_index],
            (f32)height.per_team[camera_index],
            0.0f, 1.0f);
        Hu3DCameraScissorSet(
            camera_ids.per_team[camera_index], left.per_team[camera_index],
            top.per_team[camera_index], width.per_team[camera_index],
            height.per_team[camera_index]);
    }
    object = omAddObjEx(fn_1_A0(), 272, 0, 0, 0, fn_1_C0C);
    object->data = &lbl_1_bss_8;
    memset(object->data, 0, 52);
    camera_state = object->data;
    camera_state->split_transition = 0.0f;
    fn_1_5A8();
    fn_1_690(0, 0);
    fn_1_690(1, 0);
    fn_1_11F0();
}

/* Applies the initial rail camera positions once before switching to the opening transition. */
void fn_1_C0C(OMOBJ *object)
{
    fn_1_11F0();
    object->objFunc = fn_1_C44;
}

/* Opens the second team view over sixty frames, then begins progress tracking at the start
 * announcement. */
void fn_1_C44(OMOBJ *object)
{
    M644EffectMenuData *camera_data;
    M644CameraIdPair camera_ids;
    f32 transition;

    camera_data = object->data;
    fn_1_11F0();
    if ((u32)MgSeqFrameNoGet() > 139) {
        M644CameraIdPair camera_ids = {{1, 2}};
        camera_data->split_transition = (f32)(MgSeqFrameNoGet() - 139) / 60.0f;
        transition = camera_data->split_transition;

        Hu3DCameraPerspectiveSet(
            3, 45.0f, 100.0f, 160000.0f,
            (576.0f - (288.0f * transition)) /
                480.0f);
        Hu3DCameraViewportSet(
            camera_ids.per_team[0], 0.0f, 0.0f,
            640.0f - (320.0f * transition),
            480.0f, 0.0f, 1.0f);
        Hu3DCameraScissorSet(
            camera_ids.per_team[0], 0, 0,
            (u32)(640.0 -
                  (320.0 * (f64)transition)),
            480);
        Hu3DCameraViewportSet(
            camera_ids.per_team[1],
            640.0f - (320.0f * transition),
            0.0f, (f32)(320.0 * (f64)transition),
            480.0f, 0.0f, 1.0f);
        {
            u32 right_edge = (u32)(320.0 * (f64)transition);
            Hu3DCameraScissorSet(
                camera_ids.per_team[1],
                (u32)(640.0 -
                      (320.0 * (f64)transition)),
                0, right_edge, 480);
        }
    }
    if (MgSeqModeGet() == MGSEQ_MODE_START) {
        fn_1_9664();
        camera_data->split_transition = 0;
        object->objFunc = fn_1_EE0;
    }
}

/* Updates camera rails and team progress each frame until a team reaches the finish. */
void fn_1_EE0(OMOBJ *object)
{
    s16 selector;
    s32 entry_index;
    f32 motion_speed;
    void * object_data = object->data;

    fn_1_11F0();
    if (fn_1_6F8(0) == 0 && lbl_1_bss_8.slowdown_requested[0] == 0)
    {
        fn_1_94A4(0);
    }
    if (fn_1_6F8(1) == 0 && lbl_1_bss_8.slowdown_requested[1] == 0)
    {
        fn_1_94A4(1);
    }
    for (entry_index = 0; entry_index < 2; entry_index++)
    {
        if (lbl_1_bss_8.slowdown_requested[entry_index] == 1)
        {
            selector = lbl_1_bss_8.selector[entry_index];
            motion_speed = fn_1_9178(entry_index);
            Hu3DMotionSpeedSet(lbl_1_bss_8.camera_models[entry_index][selector], motion_speed);
        }
    }
    if (fn_1_9114() != 0)
    {
        object->objFunc = fn_1_1060;
    }
}

/* Updates both rail cameras after finish and applies each requested route slowdown. */
void fn_1_1060(OMOBJ *object)
{
    void *object_data = object->data;
    s32 entry_index;

    fn_1_11F0();
    for (entry_index = 0; entry_index < 2; entry_index++) {
        if (lbl_1_bss_8.slowdown_requested[entry_index] == 1) {
            s16 selector = lbl_1_bss_8.selector[entry_index];
            f32 motion_speed = fn_1_9178(entry_index);

            Hu3DMotionSpeedSet(lbl_1_bss_8.camera_models[entry_index][selector], motion_speed);
        }
    }
}

/* Makes a team's camera rail follow its route slowdown when result handling requests it. */
void fn_1_1124(s32 team_index) {
    s16 selector;

    selector = lbl_1_bss_8.selector[team_index];
    lbl_1_bss_8.slowdown_requested[team_index] = 1;
    lbl_1_bss_8.motion_speed[team_index] = (f32) 1.0f;
}

/* Unpauses the selected camera rail after a route-motion change. */
void fn_1_1180(s32 index)
{
    s16 selector;
    selector = lbl_1_bss_8.selector[index];
    Hu3DModelAttrReset(lbl_1_bss_8.camera_models[index][selector], HU3D_MOTATTR_PAUSE);
}

/* Reads the active rail's camera and target hooks each frame to position the team views. */
void fn_1_11F0(void)
{
    HuVecF position;
    HuVecF target;
    s16 model_id;
    s32 selector;
    s32 team_index;

    for (team_index = 0; team_index < 2; team_index++) {
        M644CameraIds camera_ids = {{1, 2}};
        selector = lbl_1_bss_8.selector[team_index];
        model_id = lbl_1_bss_8.camera_models[team_index][selector];
        if (Hu3DMotionEndCheck(
                lbl_1_bss_8.camera_models[team_index][selector]) != 1) {
            Hu3DModelObjPosGet(
                lbl_1_bss_8.camera_models[team_index][selector], "CAMgrid",
                &position);
            Hu3DModelObjPosGet(
                lbl_1_bss_8.camera_models[team_index][selector], "CAMintgrid",
                &target);
            Hu3DCameraPosSet(camera_ids.per_team[team_index], position.x, position.y,
                             position.z, 0.0f, 1.0f,
                             0.0f, target.x, target.y, target.z);
        }
    }
}

/* Places both cameras at an offset from their team's NPC01 hook on request. */
void fn_1_134C(void)
{
    Mtx npc_matrix;
    Vec target;
    Vec position;

    target.x = 0.0f;
    target.y = 0.0f;
    target.z = 0.0f;
    position.x = 100.0f;
    position.y = 500.0f;
    position.z = 1400.0f;

    fn_1_80D4(0, npc_matrix);
    PSMTXMultVec(npc_matrix, &target, &target);
    PSMTXMultVec(npc_matrix, &position, &position);
    Hu3DCameraPosSet(1,
                     position.x, position.y, position.z,
                     0.0f, 1.0f, 0.0f,
                     target.x, target.y, target.z);

    target.x = 0.0f;
    target.y = 0.0f;
    target.z = 0.0f;
    position.x = 100.0f;
    position.y = 500.0f;
    position.z = 1400.0f;

    fn_1_80D4(1, npc_matrix);
    PSMTXMultVec(npc_matrix, &target, &target);
    PSMTXMultVec(npc_matrix, &position, &position);
    Hu3DCameraPosSet(2,
                     position.x, position.y, position.z,
                     0.0f, 1.0f, 0.0f,
                     target.x, target.y, target.z);
}

/* Expands the winner's camera and clips the other view after the finish-scene wipe. */
void fn_1_1510(void)
{
    M644CameraIds camera_ids = {{1, 2}};
    s32 camera_index;

    camera_index = fn_1_224();
    Hu3DCameraPerspectiveSet(camera_ids.per_team[camera_index], 45.0f,
                             100.0f, 160000.0f,
                             1.2f);
    Hu3DCameraViewportSet(camera_ids.per_team[camera_index], 0.0f,
                          0.0f, 640.0f,
                          480.0f, 0.0f,
                          1.0f);
    Hu3DCameraScissorSet(camera_ids.per_team[camera_index], 0, 0, 640, 480);
    Hu3DCameraViewportSet(camera_ids.per_team[1 - camera_index], 0.0f,
                          0.0f, 640.0f,
                          480.0f, 0.0f,
                          1.0f);
    Hu3DCameraScissorSet(camera_ids.per_team[1 - camera_index], 0, 0, 0, 0);
    fn_1_9614();
}
