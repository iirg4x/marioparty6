/* Animates stage models, updates shadow cameras, and maintains the sprite display. */
#include "REL/m644dll/module_types.h"
#include "game/hu3d.h"

M644DisplayState lbl_1_bss_7C0;
M644StageModelTable lbl_1_bss_764;
/* Set once a team reaches its final travel segment's finish frame. */
s16 lbl_1_bss_760;

#define MSM_SE_M644_TEAM0_TRAVEL 1962
#define MSM_SE_M644_TEAM0_ARRIVE 1963
#define MSM_SE_M644_TEAM1_TRAVEL 1964
#define MSM_SE_M644_TEAM1_ARRIVE 1965
#include "game/esprite.h"

/* Alternating travel and boarding motions, followed by the final travel and finish motions. */
s32 lbl_1_data_3D8[12] = {
    DATANUM(DATA_m644, 7), DATANUM(DATA_m644, 12), DATANUM(DATA_m644, 8), DATANUM(DATA_m644, 13),
    DATANUM(DATA_m644, 9), DATANUM(DATA_m644, 14), DATANUM(DATA_m644, 10), DATANUM(DATA_m644, 15),
    DATANUM(DATA_m644, 11), DATANUM(DATA_m644, 16), DATANUM(DATA_m644, 17), DATANUM(DATA_m644, 18)
};
/* Loaded route motions, shared by both team route models. */
s16 lbl_1_data_408[12] = {-1};
void fn_1_1EA8(s32 route, Point3d *position, Mtx matrix);

typedef Vec M644CameraUp;

typedef struct M644CameraBitPair_s {
    /* Camera-selection bits for team zero and team one. */
    s32 masks[2];
} M644CameraBitPair;

typedef Mtx M644Fn1EA8Scratch;

extern OMOBJMAN *fn_1_A0(void);

typedef struct HookNamePair {
    /* Player hook names, indexed by the teammate's boarding slot. */
    char *name[2];
} HookNamePair;

extern void fn_1_5444(s32);
extern void fn_1_54F0(s32);

void fn_1_51B4(s32, s32);
f32 fn_1_65A4(s32, s32, s32);

void fn_1_7C24(s32 player, s32 difficulty);
void fn_1_51B4(s32 player, s32 state);
void fn_1_205C(s32 player);

extern s16 GwMgNightF;

extern s32 fn_1_6588(s32 route);

extern s32 abs(s32 absoluteInput);

void fn_1_51B4(s32, s32);
s32 fn_1_5210(s32);

extern void fn_1_690(s32 player_index, s32 motion_id);
extern s32 fn_1_7C8(s32 motion_step);
extern void fn_1_1FB4(s32 player_index, s32 motion_step);
extern void fn_1_51B4(s32 player_index, s32 mode);
extern void fn_1_7C24(s32 player_index, s32 motion_step);

void fn_1_205C(s32);
void fn_1_B0(s16);

extern void fn_1_1510(void);
extern void fn_1_20A0(s32);

OMOBJMAN *fn_1_A0(void);

void fn_1_7CB8(s32 *route_state);
s32 fn_1_7D98(void);
void fn_1_7F8C(s32 unused_context, s32 entry_index, s32 slot);
void fn_1_7FF4(s32 entry_index, s32 slot, Point3d *position);
void fn_1_8064(s32 entry_index, s32 slot, Mtx matrix);
void fn_1_80D4(s32 team_index, Mtx matrix);
void fn_1_8120(s32 entry_index, s32 slot);
void fn_1_8194(s32 team_index);
void fn_1_81B4(HU3D_MODELID model_id, HU3D_MOTIONID motion_id, BOOL timing_cue);
void fn_1_8290(HU3D_MODELID model_id, HU3D_MOTIONID motion_id, BOOL timing_cue);
void fn_1_836C(OMOBJ *object);
void fn_1_8760(OMOBJ *object);
void fn_1_8A70(OMOBJ *object);
void fn_1_8B40(OMOBJ *object);
void fn_1_8BDC(OMOBJ *object);
void fn_1_8E5C(OMOBJ *object);
void fn_1_8FFC(OMOBJ *object);
void fn_1_9190(void);
void fn_1_91F4(OMOBJ *object);
void fn_1_9380(OMOBJ *object);
void fn_1_94A4(s32 index);
void fn_1_9614(void);
void fn_1_9664(void);
s32 fn_1_96B4(s32 team_index);
void fn_1_7E5C(s16 model, s32 entry_index, s32 slot);
void fn_1_7F50(s16 team_index);
s32 fn_1_9124(s32 team_index);
void fn_1_913C(s32 team_index);
f32 fn_1_9178(s32 team_index);
s16 fn_1_9114(void);

static const Vec lbl_1_rodata_458 = {0.0f, 0.0f, 1.0f};
static const M644CameraBitPair lbl_1_rodata_464 = {{1, 2}};

/* Moves a team's shadow camera above the current rock section when its route advances. */
void fn_1_7CB8(s32 *route_state)
{
    M644CameraBitPair camera_masks;
    M644Fn1EA8Scratch section_matrix;
    M644CameraUp up;
    HuVecF target;
    HuVecF position;

    up = lbl_1_rodata_458;
    camera_masks = lbl_1_rodata_464;

    /* The caller supplies adjacent team and route-motion words. */
    fn_1_1EA8(route_state[1] >> 1, &position, section_matrix);
    target = position;
    position.y += 3000.0f;
    position.x -= 200.0f;
    Hu3DShadowMultiPosSet(&position, &up, &target,
                          (s16)camera_masks.masks[route_state[0]]);
}

/* Creates the two team route controllers during scene setup. */
s32 fn_1_7D98(void)
{
    s32 entry_index = 0;
    OMOBJ *object;
    M644StageModelEntry *entry;

    for (entry_index = 0; entry_index < 2; entry_index++) {
        object = omAddObjEx(fn_1_A0(), 352, 0, 0, 0, fn_1_836C);
        object->data = &lbl_1_bss_764.entry[entry_index];
        memset(object->data, 0, sizeof(M644StageModelEntry));
        entry = object->data;
        entry->team_index = entry_index;
        entry->route_motion = 0;
        entry->remaining_rocks = 0;
        entry->travel_speed = 1.0f;
        entry->travel_sound = -1;
    }
}

/* Attaches a character to its team's boarding hook while that character travels with the route
 * model. */
void fn_1_7E5C(s16 model, s32 entry_index, s32 slot)
{
    HookNamePair hook_names = {{"no644douro-P1", "no644douro-P2"}};

    lbl_1_bss_764.entry[entry_index].player_models[slot] = model;
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelHookSet(lbl_1_bss_764.entry[entry_index].route_model,
                     hook_names.name[slot], model);
}

/* Clears all attachments from the selected team's route model on request. */
void fn_1_7F50(s16 team_index)
{
    Hu3DModelHookReset(lbl_1_bss_764.entry[team_index].route_model);
}

/* Detaches a character's boarding hook when that player enters the course. */
void fn_1_7F8C(s32 unused_context, s32 entry_index, s32 slot)
{
    HookNamePair hook_names = {{"no644douro-P1", "no644douro-P2"}};

    Hu3DModelHookObjReset(lbl_1_bss_764.entry[entry_index].route_model, hook_names.name[slot]);
}

/* Returns a boarding hook's world position for player spawn and return movement. */
void fn_1_7FF4(s32 entry_index, s32 slot, Point3d *position)
{
    HookNamePair hook_names = {{"no644douro-P1", "no644douro-P2"}};

    Hu3DModelObjPosGet(lbl_1_bss_764.entry[entry_index].route_model, hook_names.name[slot],
                       position);
}

/* Returns a boarding hook's world matrix when placing a character on its route model. */
void fn_1_8064(s32 entry_index, s32 slot, Mtx matrix)
{
    HookNamePair hook_names = {{"no644douro-P1", "no644douro-P2"}};

    Hu3DModelObjMtxGet(lbl_1_bss_764.entry[entry_index].route_model, hook_names.name[slot], matrix);
}

/* Returns the NPC01 hook matrix for the selected team's route model. */
void fn_1_80D4(s32 team_index, Mtx matrix) {
    Hu3DModelObjMtxGet(lbl_1_bss_764.entry[team_index].route_model, "no644douro-NPC01", matrix);
}

/* Looks up one of the two boarding hooks on request; its returned object pointer is discarded. */
void fn_1_8120(s32 entry_index, s32 slot)
{
    HookNamePair hook_names = {{"no644douro-P1", "no644douro-P2"}};
    s32 name_index;

    name_index = slot & 1;
    Hu3DModelObjPtrGet(lbl_1_bss_764.entry[entry_index].route_model, hook_names.name[name_index]);
}

/* Decrements the selected route controller's remaining-rock count on request. */
void fn_1_8194(s32 team_index) {
    lbl_1_bss_764.entry[team_index].remaining_rocks--;
}

/* Plays the team's jump and landing poses at successive outward-travel motion timing cues. */
void fn_1_81B4(HU3D_MODELID model_id, HU3D_MOTIONID motion_id, BOOL timing_cue) {
    s32 team_index;

    team_index = 0;
    if (model_id == lbl_1_bss_764.entry[0].route_model) {
        team_index = 0;
    } else {
        team_index = 1;
    }
    if (timing_cue == 1) {
        switch (lbl_1_bss_764.entry[team_index].timing_phase) {
        case 0:
            lbl_1_bss_764.entry[team_index].timing_phase = 1;
            fn_1_5444(team_index);
            break;
        case 1:
            fn_1_54F0(team_index);
            lbl_1_bss_764.entry[team_index].timing_phase = 2;
            break;
        case 2:
        default:
            break;
        }
    }
}

/* Plays the team's jump and landing poses at successive return-travel motion timing cues. */
void fn_1_8290(HU3D_MODELID model_id, HU3D_MOTIONID motion_id, BOOL timing_cue) {
    s32 team_index;

    team_index = 0;
    if (timing_cue == 1) {
        if (model_id == lbl_1_bss_764.entry[0].route_model) {
            team_index = 0;
        } else {
            team_index = 1;
        }
        switch (lbl_1_bss_764.entry[team_index].timing_phase) {
        case 0:
            lbl_1_bss_764.entry[team_index].timing_phase = 1;
            fn_1_5444(team_index);
            break;
        case 1:
            lbl_1_bss_764.entry[team_index].timing_phase = 2;
            fn_1_54F0(team_index);
            break;
        case 2:
        default:
            break;
        }
    }
}

static const M644CameraBitPair lbl_1_rodata_4A4 = {{1, 2}};

/* Builds a team's route model and attached models, then begins its first travel segment. */
void fn_1_836C(OMOBJ *object)
{
    M644StageModelEntry *data;
    Mtx matrix;
    Vec position;
    Vec target;
    Vec camera_offset;
    M644CameraBitPair camera_masks;
    M644CameraBitPair shadow_camera_masks;
    HU3D_MODELID npc01_model;
    s32 model_index;
    s32 route_index;
    s32 team_index;
    HU3D_MODELID model_id;
    HU3D_MODELID npc02_model;

    data = object->data;
    team_index = data->team_index;
    camera_masks = lbl_1_rodata_4A4;
    if (team_index == 0) {
        npc01_model = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_m644, 76), HU_MEMNUM_OVL, HEAP_MODEL));
        npc02_model = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_m644, 75), HU_MEMNUM_OVL, HEAP_MODEL));
        {
            s16 night_mode = GwMgNightF;
            if (night_mode == 0) {
            model_id = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_m644, 19), HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                model_id = Hu3DModelCreate(HuDataSelHeapReadNum(
                    DATANUM(DATA_m644, 21), HU_MEMNUM_OVL, HEAP_MODEL));
            }
        }
        for (model_index = 0; model_index < 12; model_index++) {
            lbl_1_data_408[model_index] = Hu3DJointMotion(
                model_id, HuDataSelHeapReadNum(lbl_1_data_3D8[model_index],
                                               HU_MEMNUM_OVL, HEAP_MODEL));
        }
    } else {
        {
            s16 night_mode = GwMgNightF;
            if (night_mode == 0) {
            model_id = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_m644, 20), HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                model_id = Hu3DModelCreate(HuDataSelHeapReadNum(
                    DATANUM(DATA_m644, 22), HU_MEMNUM_OVL, HEAP_MODEL));
            }
        }
        npc01_model = Hu3DModelLink(lbl_1_bss_764.entry[0].npc01_model);
        npc02_model = Hu3DModelLink(lbl_1_bss_764.entry[0].npc02_model);
    }
    data->route_model = model_id;
    data->npc01_model = npc01_model;
    data->npc02_model = npc02_model;
    Hu3DMotionSet(model_id, lbl_1_data_408[0]);
    Hu3DMotionSpeedSet(model_id, 1.0f);
    Hu3DModelCameraSet(npc01_model, camera_masks.masks[team_index]);
    Hu3DModelCameraSet(npc02_model, camera_masks.masks[team_index]);
    Hu3DModelCameraSet(model_id, camera_masks.masks[team_index]);
    Hu3DModelLayerSet(model_id, 4);
    Hu3DModelAttrReset(model_id, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(npc01_model, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(npc02_model, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(npc02_model, 420.0f, -1645.0f,
                    -52500.0f);
    Hu3DModelShadowSet(npc02_model);
    /* The two timing cues animate the boarding players independently of the route model. */
    data->timing_phase = 0;
    Hu3DMotionTimingHookSet(model_id, fn_1_81B4);
    Hu3DModelHookSet(data->route_model, "no644douro-NPC01", npc01_model);
    data->remaining_rocks = fn_1_6588(data->route_motion);
    camera_offset = lbl_1_rodata_458;
    shadow_camera_masks = lbl_1_rodata_464;
    fn_1_1EA8(data->route_motion >> 1, &position, matrix);
    target = position;
    position.y += 3000.0f;
    position.x -= 200.0f;
    Hu3DShadowMultiPosSet(&position, &camera_offset, &target,
                          shadow_camera_masks.masks[data->team_index]);
    data->sound_pitch = 0;
    if (data->team_index == 0) {
        data->travel_sound = HuAudFXPlay(MSM_SE_M644_TEAM0_TRAVEL);
        HuAudFXPitchSet(data->travel_sound, data->sound_pitch);
    } else {
        data->travel_sound = HuAudFXPlay(MSM_SE_M644_TEAM1_TRAVEL);
        HuAudFXPitchSet(data->travel_sound, data->sound_pitch);
    }
    object->objFunc = fn_1_8760;
}

/* Updates travel sound pitch and rock visibility each frame, then releases players when travel
 * ends. */
void fn_1_8760(OMOBJ *object)
{
    M644StageModelEntry *data;
    s32 team_index;
    f32 opening_motion_time;
    f32 current_motion_time;
    f32 fade_motion_time;
    s16 pitch;
    s16 motion_id;

    data = object->data;
    team_index = data->team_index;
    /* The second section's rocks appear only as travel passes frame 130. */
    if (data->route_motion == 2) {
        opening_motion_time = Hu3DMotionTimeGet(data->route_model);
        if (129.0f < opening_motion_time &&
            opening_motion_time < 131.0f) {
            fn_1_7C24(data->team_index, data->route_motion >> 1);
        }
    }
    current_motion_time = Hu3DMotionTimeGet(data->route_model);
    pitch = 8191 - abs((s16)(((8191.0f *
                                  (current_motion_time - 130.0f)) /
                                 130.0f)));
    if (pitch < 0) {
        pitch = 0;
    }
    if (pitch > 8191) {
        pitch = 8191;
    }
    HuAudFXPitchSet(data->travel_sound, pitch);
    if (Hu3DMotionTimeGet(data->route_model) == 260.0f) {
        if (data->team_index == 0) {
            HuAudFXPlay(MSM_SE_M644_TEAM0_ARRIVE);
        } else {
            HuAudFXPlay(MSM_SE_M644_TEAM1_ARRIVE);
        }
    }
    if (Hu3DMotionEndCheck(data->route_model) != 0) {
        fn_1_51B4(data->team_index, 2);
        object->objFunc = fn_1_8A70;
    }
    /* Result handling gradually reduces both travel speed and its looping sound volume. */
    if (data->slowdown_requested == 1) {
        fade_motion_time = Hu3DMotionTimeGet(data->route_model);
        motion_id = Hu3DMotionIDGet(data->route_model);
        if (motion_id == lbl_1_data_408[0] ||
            motion_id == lbl_1_data_408[2] ||
            motion_id == lbl_1_data_408[4] ||
            motion_id == lbl_1_data_408[6] ||
            motion_id == lbl_1_data_408[8] ||
            motion_id == lbl_1_data_408[10]) {
            if (fade_motion_time < 249.0f) {
                Hu3DMotionSpeedSet(data->route_model, data->travel_speed);
                data->travel_speed -= 0.02f;
                /* The volume request precedes the negative-speed clamp and sound stop. */
                if (data->travel_sound != -1) {
                    HuAudFXVolSet(data->travel_sound,
                                  127.0f * data->travel_speed);
                }
                if (data->travel_speed < 0.0f) {
                    data->travel_speed = 0.0f;
                    HuAudFXStop(data->travel_sound);
                    data->travel_sound = -1;
                }
            }
        }
        fn_1_205C(data->team_index);
    }
}

/* Waits until all rocks in the current section are depleted, then recalls that team's players. */
void fn_1_8A70(OMOBJ *object) {
    M644StageModelEntry *data;

    data = object->data;
    if ((fn_1_65A4(data->team_index, data->route_motion, 0) <= 0.0f) &&
        (fn_1_65A4(data->team_index, data->route_motion, 1) <= 0.0f) &&
        (fn_1_65A4(data->team_index, data->route_motion, 2) <= 0.0f)) {
        data->route_motion = data->route_motion + 1;
        fn_1_51B4(data->team_index, 3);
        object->objFunc = fn_1_8B40;
    }
}

/* Waits for both players to return before starting the route model's boarding animation. */
void fn_1_8B40(OMOBJ *object) {
    M644StageModelEntry *data;

    data = object->data;
    if (fn_1_5210(data->team_index) == 0) {
        Hu3DMotionSet(data->route_model, lbl_1_data_408[data->route_motion]);
        data->timing_phase = 0;
        Hu3DMotionTimingHookSet(data->route_model, fn_1_8290);
        fn_1_51B4(data->team_index, 4);
        data->timing_phase = 0;
        object->objFunc = fn_1_8BDC;
    }
}

/* Starts the next travel segment after boarding, updating the course animation, shadows and player
 * phases. */
void fn_1_8BDC(OMOBJ *object)
{
    M644StageModelEntry *sequence;

    sequence = object->data;
    if (Hu3DMotionEndCheck(sequence->route_model) == 0) {
        return;
    }

    sequence->route_motion++;
    sequence->remaining_rocks = fn_1_6588(sequence->route_motion);
    Hu3DMotionSet(sequence->route_model,
                  lbl_1_data_408[sequence->route_motion]);
    Hu3DMotionTimeSet(sequence->route_model, 0.0f);
    fn_1_690(sequence->team_index, fn_1_7C8(sequence->route_motion));
    if (sequence->route_motion == 10) {
        fn_1_51B4(sequence->team_index, 5);
        sequence->sound_pitch = 0;
        if (sequence->team_index == 0) {
            sequence->travel_sound = HuAudFXPlay(MSM_SE_M644_TEAM0_TRAVEL);
            HuAudFXPitchSet(sequence->travel_sound, (s16)sequence->sound_pitch);
        } else {
            sequence->travel_sound = HuAudFXPlay(MSM_SE_M644_TEAM1_TRAVEL);
            HuAudFXPitchSet(sequence->travel_sound, (s16)sequence->sound_pitch);
        }
        object->objFunc = fn_1_8E5C;
        return;
    }

    fn_1_1FB4(sequence->team_index, sequence->route_motion);
    fn_1_51B4(sequence->team_index, 1);
    Hu3DMotionTimingHookSet(sequence->route_model, fn_1_81B4);
    sequence->timing_phase = 0;
    fn_1_7CB8(&sequence->team_index);
    sequence->sound_pitch = 0;
    if (sequence->team_index == 0) {
        sequence->travel_sound = HuAudFXPlay(MSM_SE_M644_TEAM0_TRAVEL);
        HuAudFXPitchSet(sequence->travel_sound, (s16)sequence->sound_pitch);
    } else {
        sequence->travel_sound = HuAudFXPlay(MSM_SE_M644_TEAM1_TRAVEL);
        HuAudFXPitchSet(sequence->travel_sound, (s16)sequence->sound_pitch);
    }
    if (sequence->route_motion != 2) {
        fn_1_7C24(sequence->team_index, sequence->route_motion >> 1);
    }
    object->objFunc = fn_1_8760;
}

/* Runs the final travel segment and declares its team the winner once it reaches frame 180 during
 * play. */
void fn_1_8E5C(OMOBJ *object)
{
    M644StageModelEntry *state;
    float motion_time;
    state = object->data;
    if (state->slowdown_requested == 1) {
        motion_time = Hu3DMotionTimeGet(state->route_model);
        if (motion_time < 249.0f) {
            Hu3DMotionSpeedSet(state->route_model, state->travel_speed);
            state->travel_speed -= 0.02f;
            /* The volume request precedes the negative-speed clamp and sound stop. */
            if (state->travel_sound != -1)
                HuAudFXVolSet(state->travel_sound, 127.0f * state->travel_speed);
            if (state->travel_speed < 0.0f) {
                state->travel_speed = 0.0f;
                HuAudFXStop(state->travel_sound);
                state->travel_sound = -1;
            }
        }
        fn_1_205C(state->team_index);
    } else {
        if (160.0f == Hu3DMotionTimeGet(state->route_model)) {
            if (state->team_index == 0) HuAudFXPlay(MSM_SE_M644_TEAM0_ARRIVE);
            else HuAudFXPlay(MSM_SE_M644_TEAM1_ARRIVE);
        }
    }
    if (MgSeqModeGet() == MGSEQ_MODE_MAIN && Hu3DMotionTimeGet(state->route_model) >= 180.0f) {
        lbl_1_bss_760 = 1;
        fn_1_B0(state->team_index);
        object->objFunc = fn_1_8FFC;
        MgSeqModeNext();
    }
}

/* Switches to the finish scene after a sixty-frame wipe in the pre-winner sequence. */
void fn_1_8FFC(OMOBJ *object)
{
    M644StageModelEntry *data = object->data;

    if (MgSeqModeGet() != MGSEQ_MODE_PREWIN) {
        return;
    }
    if (MgSeqFrameNoGet() == 0) {
        HuAudFXAllStop();
        WipeCreate(2, 0, 60);
    }
    if (MgSeqFrameNoGet() != 60) {
        return;
    }

    fn_1_1510();
    WipeCreate(1, 5, 60);
    OSReport("Set.\n");
    Hu3DModelPosSet(data->npc02_model, 0.0f,
                    0.0f, 0.0f);
    Hu3DModelHookSet(data->route_model, "no644douro-NPC02B", data->npc02_model);
    Hu3DMotionSet(data->route_model, lbl_1_data_408[11]);
    Hu3DMotionTimeSet(data->route_model, 0.0f);
    fn_1_690(data->team_index, fn_1_7C8(data->route_motion + 1));
    fn_1_20A0(data->team_index);
}

/* Reports whether a team has reached the final travel segment's finish point. */
s16 fn_1_9114(void)
{
    return lbl_1_bss_760;
}

/* Returns a team's current route-motion index for rock and CPU queries. */
s32 fn_1_9124(s32 team_index) {
    return lbl_1_bss_764.entry[team_index].route_motion;
}

/* Requests a team's travel slowdown when the minigame result is decided. */
void fn_1_913C(s32 team_index) {
    lbl_1_bss_764.entry[team_index].slowdown_requested = 1;
    lbl_1_bss_764.entry[team_index].travel_speed = 1.0f;
}

/* Returns the current travel speed multiplier for camera slowdown. */
f32 fn_1_9178(s32 team_index) {
    return lbl_1_bss_764.entry[team_index].travel_speed;
}

/* Creates the course progress display object during scene setup. */
void fn_1_9190(void)
{
    OMOBJ *object;
    object = omAddObjEx(fn_1_A0(), 528, 0, 0, 0, fn_1_91F4);
    object->data = &lbl_1_bss_7C0;
    memset(object->data, 0, sizeof(lbl_1_bss_7C0));
}

/* Progress-marker, endpoint and route-segment sprite resources. */
u32 lbl_1_data_468[19] = {
    DATANUM(DATA_m644, 73), DATANUM(DATA_m644, 74), DATANUM(DATA_m644, 70), DATANUM(DATA_m644, 72),
    DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71),
    DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71),
    DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71),
    DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71), DATANUM(DATA_m644, 71)
};
/* Sprite draw priorities: team markers, endpoints, then route segments. */
u32 lbl_1_data_4B4[19] = {
    29998, 29998, 29999, 29999, 30000, 30000, 30000, 30000,
    30000, 30000, 30000, 30000, 30000, 30000, 30000, 30000,
    30000, 30000, 30000
};

/* Creates and positions the course progress sprites, initially hidden until the sequence shows
 * them. */
void fn_1_91F4(OMOBJ *object)
{
    M644DisplayState *display_state;
    s32 index;

    display_state = object->data;
    for (index = 0; index < 19; index++) {
        display_state->display_id[index] =
            espEntry(lbl_1_data_468[index], (s16)lbl_1_data_4B4[index], 0);
    }
    espPosSet(display_state->display_id[2], 288.0f,
              416.0f);
    espPosSet(display_state->display_id[3], 288.0f,
              98.0f);
    for (index = 0; index < 15; index++) {
        espPosSet(display_state->display_id[index + 4], 288.0f,
                  16.0f + (32.0f * (f32)index));
    }
    espPosSet(display_state->display_id[0], 274.0f,
              432.0f);
    espPosSet(display_state->display_id[1], 302.0f,
              432.0f);
    fn_1_9614();
    object->objFunc = 0;
}

/* Updates both team markers from their progress counters when invoked as a display callback. */
void fn_1_9380(OMOBJ *object)
{
    M644DisplayState *data;
    f32 vertical_position;

    data = object->data;
    vertical_position = 432.0f -
                        (328.0f * ((f32)data->progress[0] /
                                            1500.0f));
    espPosSet(data->display_id[0], 274.0f, vertical_position);
    vertical_position = 432.0f -
                        (328.0f * ((f32)data->progress[1] /
                                            1500.0f));
    espPosSet(data->display_id[1], 302.0f, vertical_position);
}

/* Advances one team's progress counter and immediately moves its display marker during camera
 * travel. */
void fn_1_94A4(s32 index)
{
    f32 first_position;
    f32 second_position;

    lbl_1_bss_7C0.progress[index]++;
    if (index == 0) {
        first_position = 432.0f -
                         (328.0f *
                          ((f32)lbl_1_bss_7C0.progress[0] /
                           1500.0f));
        espPosSet(lbl_1_bss_7C0.display_id[0], 274.0f,
                  first_position);
        return;
    }
    second_position = 432.0f -
                      (328.0f *
                       ((f32)lbl_1_bss_7C0.progress[1] /
                        1500.0f));
    espPosSet(lbl_1_bss_7C0.display_id[1], 302.0f,
              second_position);
}

/* Hides every course progress sprite on request. */
void fn_1_9614(void) {
    int sprite_index;
    for (sprite_index = 0; sprite_index < 19; sprite_index++) {
        espDispOff(lbl_1_bss_7C0.display_id[sprite_index]);
    }
}

/* Shows every course progress sprite when the start announcement begins. */
void fn_1_9664(void) {
    s32 sprite_index = 0;
    while (sprite_index < 19) {
        espDispOn(lbl_1_bss_7C0.display_id[sprite_index]);
        sprite_index++;
    }
}

/* Returns a team's course progress counter. */
s32 fn_1_96B4(s32 team_index) {
    return lbl_1_bss_7C0.progress[team_index];
}
