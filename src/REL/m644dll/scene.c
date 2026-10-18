/* Builds the course models and looks up their camera and collision transforms. */
#include "REL/m644dll/module_types.h"
#include "game/hu3d.h"

#define M644_COURSE_COLLISION_MASK 0x60
#define M644_ROCK_COLLISION_MASK 0x80
/* Player collision-contact parameters carry this group bit in addition to their slot. */
#define M644_PLAYER_CONTACT_BITS 0x100

/* Course-model animations for the first four route sections. */
s32 lbl_1_data_70[4] = {
    DATANUM(DATA_m644, 56), DATANUM(DATA_m644, 57), DATANUM(DATA_m644, 58), DATANUM(DATA_m644, 59)
};
/* Loaded motion IDs shared by the two team course models. */
s16 lbl_1_data_80[4] = {-1, -1, -1, -1};

extern OMOBJMAN *fn_1_A0(void);

/* Model and motion IDs shared by route, camera and collision setup. */
M644ModelScene lbl_1_bss_40;

typedef struct ModelObjectNames {
    /* Model hook names in course-section order. */
    char *name[5];
} ModelObjectNames;

void fn_1_21B8(M644PlayerRecord *);
void fn_1_24D4(M644PlayerRecord *);

typedef struct M644ResourceIds_s {
    /* Packed model or motion entries for the three scene pieces. */
    s32 resource_ids[3];
} M644ResourceIds;

static const M644ResourceIds seasonal_model_resources =
    {{DATANUM(DATA_m644, 77), DATANUM(DATA_m644, 79), DATANUM(DATA_m644, 81)}};
static const M644ResourceIds seasonal_motion_resources =
    {{DATANUM(DATA_m644, 78), DATANUM(DATA_m644, 80), DATANUM(DATA_m644, 82)}};

extern s16 GwMgNightF;
extern void fn_1_7D98(void);
extern void fn_1_4E84(void);
extern void fn_1_6838(void);
extern void fn_1_848(void);
extern s32 fn_1_9190(void);

void fn_1_1690(void);
void fn_1_1EA8(s32 entry_index, Point3d *position, Mtx matrix);
void fn_1_1F38(s32 entry_index, Mtx matrix);
void fn_1_1FB4(s32 team_index, s32 route_sequence);
void fn_1_205C(s32 team_index);
void fn_1_20A0(s32 team_index);
int fn_1_213C(COL_NARROW_PARAM *contact, COL_NARROW_PARAM *other_contact);
void fn_1_215C(M644PlayerRecord *player);
void fn_1_16F4(OMOBJ *object);
void fn_1_20E4(MGPLAYER *player);
void fn_1_2110(MGPLAYER *player);

/* Creates the one-shot scene setup object during module initialization. */
void fn_1_1690(void) {
    OMOBJ *object;

    object = omAddObjEx(fn_1_A0(), 256, 0, 0, 0, fn_1_16F4);
    object->data = &lbl_1_bss_40;
    memset(object->data, 0, sizeof(M644ModelScene));
}

/* Builds the day or night course, lighting and collision maps, then starts the gameplay objects. */
void fn_1_16F4(OMOBJ *object)
{
    M644ModelScene *data;
    s32 team_index;
    s32 model_index;
    s32 motion_index;
    s16 light_id;
    s16 night_mode;
    s16 collision_models[2];

    data = object->data;
    Hu3DLighInit();
    data->collision_model =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m644, 23), HU_MEMNUM_OVL, HEAP_MODEL));
    data->secondary_collision_model =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m644, 24), HU_MEMNUM_OVL, HEAP_MODEL));
    data->rock_hook_model =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m644, 41), HU_MEMNUM_OVL, HEAP_MODEL));

    night_mode = GwMgNightF;
    if (night_mode == 0) {
        M644ResourceIds variant_model_data;
        M644ResourceIds variant_motion_data;

        data->course_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 26), HU_MEMNUM_OVL, HEAP_MODEL));
        data->lighting_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 25), HU_MEMNUM_OVL, HEAP_MODEL));
        /* The day branch creates another hook model and replaces the shared model ID. */
        data->rock_hook_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 41), HU_MEMNUM_OVL, HEAP_MODEL));
        data->primary_animated_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 28), HU_MEMNUM_OVL, HEAP_MODEL));
        data->secondary_animated_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 29), HU_MEMNUM_OVL, HEAP_MODEL));
        data->looping_course_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 27), HU_MEMNUM_OVL, HEAP_MODEL));
        data->scenery_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 30), HU_MEMNUM_OVL, HEAP_MODEL));
        data->team_course_models[0] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 55), HU_MEMNUM_OVL, HEAP_MODEL));

        {
        for (team_index = 0; team_index < 2; team_index++) {
            for (model_index = 0; model_index < 3; model_index++) {
                variant_model_data = seasonal_model_resources;
                variant_motion_data = seasonal_motion_resources;
                if (team_index == 0) {
                    data->day_piece_models[team_index][model_index] = Hu3DModelCreate(
                        HuDataSelHeapReadNum(variant_model_data.resource_ids[model_index],
                                             HU_MEMNUM_OVL, HEAP_MODEL));
                } else {
                    data->day_piece_models[team_index][model_index] =
                        Hu3DModelLink(data->day_piece_models[0][model_index]);
                }
                data->day_piece_motions[team_index][model_index] = Hu3DJointMotion(
                    data->day_piece_models[team_index][model_index],
                    HuDataSelHeapReadNum(
                        variant_motion_data.resource_ids[model_index], HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DMotionSet(data->day_piece_models[team_index][model_index],
                              data->day_piece_motions[team_index][model_index]);
            }
            Hu3DModelAttrReset(data->day_piece_models[team_index][0], HU3D_MOTATTR_LOOP);
            Hu3DModelAttrSet(data->day_piece_models[team_index][1], HU3D_MOTATTR_LOOP);
            Hu3DModelAttrReset(data->day_piece_models[team_index][2], HU3D_MOTATTR_LOOP);
            Hu3DModelAttrSet(data->day_piece_models[team_index][2], HU3D_MOTATTR_PAUSE);
        }
        }
    } else {
        data->course_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 32), HU_MEMNUM_OVL, HEAP_MODEL));
        data->lighting_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 31), HU_MEMNUM_OVL, HEAP_MODEL));
        data->primary_animated_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 35), HU_MEMNUM_OVL, HEAP_MODEL));
        data->secondary_animated_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 34), HU_MEMNUM_OVL, HEAP_MODEL));
        data->looping_course_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 33), HU_MEMNUM_OVL, HEAP_MODEL));
        data->scenery_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 36), HU_MEMNUM_OVL, HEAP_MODEL));
        data->team_course_models[0] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 61), HU_MEMNUM_OVL, HEAP_MODEL));
        data->night_model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 37), HU_MEMNUM_OVL, HEAP_MODEL));
        data->night_motion = Hu3DJointMotion(
            data->night_model,
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 38), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionSet(data->night_model, data->night_motion);
        Hu3DModelAttrSet(data->night_model, HU3D_MOTATTR_LOOP);
    }

    for (motion_index = 0; motion_index < 4; motion_index++) {
        lbl_1_data_80[motion_index] = Hu3DJointMotion(
            data->team_course_models[0],
            HuDataSelHeapReadNum(lbl_1_data_70[motion_index], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    data->team_course_models[1] = Hu3DModelLink(data->team_course_models[0]);
    Hu3DMotionSet(data->team_course_models[0], lbl_1_data_80[0]);
    Hu3DMotionSet(data->team_course_models[1], lbl_1_data_80[0]);

    data->primary_model_motion =
        Hu3DJointMotion(data->primary_animated_model,
                        HuDataSelHeapReadNum(DATANUM(DATA_m644, 39), HU_MEMNUM_OVL, HEAP_MODEL));
    data->secondary_model_motion =
        Hu3DJointMotion(data->secondary_animated_model,
                        HuDataSelHeapReadNum(DATANUM(DATA_m644, 40), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(data->primary_animated_model, data->primary_model_motion);
    Hu3DMotionSet(data->secondary_animated_model, data->secondary_model_motion);
    Hu3DModelAttrSet(data->primary_animated_model, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(data->secondary_animated_model, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(data->looping_course_model, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(data->team_course_models[0], 1);
    Hu3DModelCameraSet(data->team_course_models[1], 2);

    collision_models[0] = data->collision_model;
    collision_models[1] = data->secondary_collision_model;
    MgActorColMapInit(collision_models, 2, 28);
    MgActorColMapMaskSet(0, M644_COURSE_COLLISION_MASK);
    MgActorColMapMaskSet(1, M644_ROCK_COLLISION_MASK);
    Hu3DModelAttrSet(data->collision_model, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(data->secondary_collision_model, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(data->team_course_models[0], HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(data->team_course_models[1], HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(data->team_course_models[0], 0);
    Hu3DModelLayerSet(data->team_course_models[1], 0);
    Hu3DModelLayerSet(data->course_model, 1);
    Hu3DModelLayerSet(data->lighting_model, 1);
    Hu3DModelLayerSet(data->looping_course_model, 2);
    Hu3DModelLayerSet(data->secondary_animated_model, 3);
    Hu3DModelLayerSet(data->primary_animated_model, 3);
    Hu3DModelLayerSet(data->scenery_model, 3);
    Hu3DModelAttrSet(data->secondary_animated_model, HU3D_MOTATTR_LOOP);
    Hu3DModelLightInfoSet(data->lighting_model, 1);
    light_id = Hu3DModelLightIdGet(data->lighting_model, "no644douro-inf_light1");
    Hu3DGLightInfinitytSet(light_id);
    Hu3DGLightStaticSet(light_id, 1);

    {
        s16 shadow_night_mode;
        /* These preset vectors are prepared but are not passed to shadow setup. */
        Vec shadow_position = {-100.0f, 3000.0f, 0.0f};
        Vec shadow_up = {0.0f, 0.0f, 1.0f};
        Vec shadow_target = {0.0f, 0.0f, 0.0f};

        Hu3DShadowMultiCreate(45.0f, 20.0f, 5000.0f, 3);
        Hu3DShadowMultiSizeSet(64, 3);
        shadow_night_mode = GwMgNightF;
        if (shadow_night_mode == 0) {
            Hu3DShadowMultiTPLvlSet(0.7f, 3);
            Hu3DShadowMultiColSet(79, 110, 128, 3);
        } else {
            Hu3DShadowMultiTPLvlSet(0.6f, 3);
            Hu3DShadowMultiColSet(15, 46, 64, 3);
        }
    }
    Hu3DModelShadowMapSet(data->course_model);
    fn_1_7D98();
    fn_1_4E84();
    fn_1_6838();
    fn_1_848();
    fn_1_9190();
    object->objFunc = NULL;
}

/* Leaves the scene unchanged when this empty hook is called. */
void fn_1_1EA4(void)
{
}

/* Returns a rock-section hook's world matrix and origin for route and CPU placement. */
void fn_1_1EA8(s32 entry_index, Point3d *position, Mtx matrix)
{
    ModelObjectNames object_names = { { "no644douro-rockPOS01", "no644douro-rockPOS02",
                                        "no644douro-rockPOS03", "no644douro-rockPOS04",
                                        "no644douro-rockPOS05" } };
    Hu3DModelObjMtxGet(lbl_1_bss_40.rock_hook_model, object_names.name[entry_index], matrix);
    Hu3DMtxTransGet(matrix, position);
}

/* Looks up the collision-section matrix for a paired route index. */
void fn_1_1F38(s32 entry_index, Mtx matrix)
{
    ModelObjectNames object_names = { { "no644douro-col1", "no644douro-col2", "no644douro-col3",
                                        "no644douro-col4", "no644douro-col5" } };
    Hu3DModelObjMtxGet(lbl_1_bss_40.collision_model, object_names.name[entry_index >> 1], matrix);
}

/* Selects a team's course-model animation when its route sequence enters one of the first four
 * sections. */
void fn_1_1FB4(s32 team_index, s32 route_sequence) {
    if ((s32) (route_sequence >> 1) <= 3) {
        OSReport("camera index = %d, seq = %d\n", team_index, route_sequence >> 1);
        Hu3DMotionSet(lbl_1_bss_40.team_course_models[team_index],
                      lbl_1_data_80[route_sequence >> 1]);
        Hu3DModelAttrReset(lbl_1_bss_40.team_course_models[team_index], HU3D_MOTATTR_LOOP);
    }
}

/* Pauses a team's course-model animation when the route controller slows to a stop. */
void fn_1_205C(s32 team_index) {
    s32 status;
    Hu3DModelAttrSet(lbl_1_bss_40.team_course_models[team_index], HU3D_MOTATTR_PAUSE);
}

/* Resumes the third linked scene piece for a team at the route controller's transition. */
void fn_1_20A0(s32 team_index) {
    s32 status;
    Hu3DModelAttrReset(lbl_1_bss_40.day_piece_models[team_index][2], HU3D_MOTATTR_PAUSE);
}

/* Enables the engine's COM stick override for the supplied player. */
void fn_1_20E4(MGPLAYER *player) {
    MgPlayerAttrSet(player, MGPLAYER_ATTR_COMSTK);
}

/* Restores normal stick input for the supplied player. */
void fn_1_2110(MGPLAYER *player)
{
    MgPlayerAttrReset(player, MGPLAYER_ATTR_COMSTK);
}

/* Rejects player-to-player contacts; installed on each player's collision body. */
int fn_1_213C(COL_NARROW_PARAM *contact, COL_NARROW_PARAM *other_contact) {
    if ((other_contact->paramB & M644_PLAYER_CONTACT_BITS) != 0) {
        return 0;
    } else {
        return 1;
    }
}
