/* Builds the course models and looks up their camera and collision transforms. */
#include "REL/m644dll/module_types.h"
#include "game/hu3d.h"

#define M644_COURSE_COLLISION_MASK 0x60
#define M644_ROCK_COLLISION_MASK 0x80
/* Player collision-contact parameters carry this group bit in addition to their slot. */
#define M644_PLAYER_CONTACT_BITS 0x100

/* Course-model animations for the first four route sections. */
s32 lbl_1_data_70[4] = {
    DATANUM(0x670000, 56), DATANUM(0x670000, 57), DATANUM(0x670000, 58), DATANUM(0x670000, 59)
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
    {{DATANUM(0x670000, 77), DATANUM(0x670000, 79), DATANUM(0x670000, 81)}};
static const M644ResourceIds seasonal_motion_resources =
    {{DATANUM(0x670000, 78), DATANUM(0x670000, 80), DATANUM(0x670000, 82)}};

extern s16 lbl_802C0278;
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

    object = fn_8003AB38(fn_1_A0(), 256, 0, 0, 0, fn_1_16F4);
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
    fn_800285BC();
    data->collision_model =
        fn_80025A04(fn_80008DD0(DATANUM(0x670000, 23), 0x10000000, 2));
    data->secondary_collision_model =
        fn_80025A04(fn_80008DD0(DATANUM(0x670000, 24), 0x10000000, 2));
    data->rock_hook_model =
        fn_80025A04(fn_80008DD0(DATANUM(0x670000, 41), 0x10000000, 2));

    night_mode = lbl_802C0278;
    if (night_mode == 0) {
        M644ResourceIds variant_model_data;
        M644ResourceIds variant_motion_data;

        data->course_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 26), 0x10000000, 2));
        data->lighting_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 25), 0x10000000, 2));
        /* The day branch creates another hook model and replaces the shared model ID. */
        data->rock_hook_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 41), 0x10000000, 2));
        data->primary_animated_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 28), 0x10000000, 2));
        data->secondary_animated_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 29), 0x10000000, 2));
        data->looping_course_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 27), 0x10000000, 2));
        data->scenery_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 30), 0x10000000, 2));
        data->team_course_models[0] = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 55), 0x10000000, 2));

        {
        for (team_index = 0; team_index < 2; team_index++) {
            for (model_index = 0; model_index < 3; model_index++) {
                variant_model_data = seasonal_model_resources;
                variant_motion_data = seasonal_motion_resources;
                if (team_index == 0) {
                    data->day_piece_models[team_index][model_index] = fn_80025A04(
                        fn_80008DD0(variant_model_data.resource_ids[model_index],
                                             0x10000000, 2));
                } else {
                    data->day_piece_models[team_index][model_index] =
                        fn_80025D4C(data->day_piece_models[0][model_index]);
                }
                data->day_piece_motions[team_index][model_index] = fn_80030458(
                    data->day_piece_models[team_index][model_index],
                    fn_80008DD0(
                        variant_motion_data.resource_ids[model_index], 0x10000000, 2));
                fn_8002C95C(data->day_piece_models[team_index][model_index],
                              data->day_piece_motions[team_index][model_index]);
            }
            fn_80026A44(data->day_piece_models[team_index][0], ((1 << 30) | (1 << 0)));
            fn_800269E8(data->day_piece_models[team_index][1], ((1 << 30) | (1 << 0)));
            fn_80026A44(data->day_piece_models[team_index][2], ((1 << 30) | (1 << 0)));
            fn_800269E8(data->day_piece_models[team_index][2], ((1 << 30) | (1 << 1)));
        }
        }
    } else {
        data->course_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 32), 0x10000000, 2));
        data->lighting_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 31), 0x10000000, 2));
        data->primary_animated_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 35), 0x10000000, 2));
        data->secondary_animated_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 34), 0x10000000, 2));
        data->looping_course_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 33), 0x10000000, 2));
        data->scenery_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 36), 0x10000000, 2));
        data->team_course_models[0] = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 61), 0x10000000, 2));
        data->night_model = fn_80025A04(
            fn_80008DD0(DATANUM(0x670000, 37), 0x10000000, 2));
        data->night_motion = fn_80030458(
            data->night_model,
            fn_80008DD0(DATANUM(0x670000, 38), 0x10000000, 2));
        fn_8002C95C(data->night_model, data->night_motion);
        fn_800269E8(data->night_model, ((1 << 30) | (1 << 0)));
    }

    for (motion_index = 0; motion_index < 4; motion_index++) {
        lbl_1_data_80[motion_index] = fn_80030458(
            data->team_course_models[0],
            fn_80008DD0(lbl_1_data_70[motion_index], 0x10000000, 2));
    }
    data->team_course_models[1] = fn_80025D4C(data->team_course_models[0]);
    fn_8002C95C(data->team_course_models[0], lbl_1_data_80[0]);
    fn_8002C95C(data->team_course_models[1], lbl_1_data_80[0]);

    data->primary_model_motion =
        fn_80030458(data->primary_animated_model,
                        fn_80008DD0(DATANUM(0x670000, 39), 0x10000000, 2));
    data->secondary_model_motion =
        fn_80030458(data->secondary_animated_model,
                        fn_80008DD0(DATANUM(0x670000, 40), 0x10000000, 2));
    fn_8002C95C(data->primary_animated_model, data->primary_model_motion);
    fn_8002C95C(data->secondary_animated_model, data->secondary_model_motion);
    fn_800269E8(data->primary_animated_model, ((1 << 30) | (1 << 0)));
    fn_800269E8(data->secondary_animated_model, ((1 << 30) | (1 << 0)));
    fn_800269E8(data->looping_course_model, ((1 << 30) | (1 << 0)));
    fn_80026B64(data->team_course_models[0], 1);
    fn_80026B64(data->team_course_models[1], 2);

    collision_models[0] = data->collision_model;
    collision_models[1] = data->secondary_collision_model;
    fn_800804E8(collision_models, 2, 28);
    fn_80080D7C(0, M644_COURSE_COLLISION_MASK);
    fn_80080D7C(1, M644_ROCK_COLLISION_MASK);
    fn_800269E8(data->collision_model, (1 << 0));
    fn_800269E8(data->secondary_collision_model, (1 << 0));
    fn_80026A44(data->team_course_models[0], ((1 << 30) | (1 << 0)));
    fn_80026A44(data->team_course_models[1], ((1 << 30) | (1 << 0)));
    fn_80026B8C(data->team_course_models[0], 0);
    fn_80026B8C(data->team_course_models[1], 0);
    fn_80026B8C(data->course_model, 1);
    fn_80026B8C(data->lighting_model, 1);
    fn_80026B8C(data->looping_course_model, 2);
    fn_80026B8C(data->secondary_animated_model, 3);
    fn_80026B8C(data->primary_animated_model, 3);
    fn_80026B8C(data->scenery_model, 3);
    fn_800269E8(data->secondary_animated_model, ((1 << 30) | (1 << 0)));
    fn_80029850(data->lighting_model, 1);
    light_id = fn_80029D24(data->lighting_model, "no644douro-inf_light1");
    Hu3DGLightInfinitytSet(light_id);
    Hu3DGLightStaticSet(light_id, 1);

    {
        s16 shadow_night_mode;
        /* These preset vectors are prepared but are not passed to shadow setup. */
        Vec shadow_position = {-100.0f, 3000.0f, 0.0f};
        Vec shadow_up = {0.0f, 0.0f, 1.0f};
        Vec shadow_target = {0.0f, 0.0f, 0.0f};

        fn_8002A3B0(45.0f, 20.0f, 5000.0f, 3);
        fn_8002A8CC(64, 3);
        shadow_night_mode = lbl_802C0278;
        if (shadow_night_mode == 0) {
            fn_8002A7D8(0.7f, 3);
            fn_8002AA3C(79, 110, 128, 3);
        } else {
            fn_8002A7D8(0.6f, 3);
            fn_8002AA3C(15, 46, 64, 3);
        }
    }
    fn_80026FFC(data->course_model);
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
    fn_80023398(lbl_1_bss_40.rock_hook_model, object_names.name[entry_index], matrix);
    Hu3DMtxTransGet(matrix, position);
}

/* Looks up the collision-section matrix for a paired route index. */
void fn_1_1F38(s32 entry_index, Mtx matrix)
{
    ModelObjectNames object_names = { { "no644douro-col1", "no644douro-col2", "no644douro-col3",
                                        "no644douro-col4", "no644douro-col5" } };
    fn_80023398(lbl_1_bss_40.collision_model, object_names.name[entry_index >> 1], matrix);
}

/* Selects a team's course-model animation when its route sequence enters one of the first four
 * sections. */
void fn_1_1FB4(s32 team_index, s32 route_sequence) {
    if ((s32) (route_sequence >> 1) <= 3) {
        OSReport("camera index = %d, seq = %d\n", team_index, route_sequence >> 1);
        fn_8002C95C(lbl_1_bss_40.team_course_models[team_index],
                      lbl_1_data_80[route_sequence >> 1]);
        fn_80026A44(lbl_1_bss_40.team_course_models[team_index], ((1 << 30) | (1 << 0)));
    }
}

/* Pauses a team's course-model animation when the route controller slows to a stop. */
void fn_1_205C(s32 team_index) {
    s32 status;
    fn_800269E8(lbl_1_bss_40.team_course_models[team_index], ((1 << 30) | (1 << 1)));
}

/* Resumes the third linked scene piece for a team at the route controller's transition. */
void fn_1_20A0(s32 team_index) {
    s32 status;
    fn_80026A44(lbl_1_bss_40.day_piece_models[team_index][2], ((1 << 30) | (1 << 1)));
}

/* Enables the engine's COM stick override for the supplied player. */
void fn_1_20E4(MGPLAYER *player) {
    fn_80087158(player, (1 << 0));
}

/* Restores normal stick input for the supplied player. */
void fn_1_2110(MGPLAYER *player)
{
    fn_80087168(player, (1 << 0));
}

/* Rejects player-to-player contacts; installed on each player's collision body. */
int fn_1_213C(COL_NARROW_PARAM *contact, COL_NARROW_PARAM *other_contact) {
    if ((other_contact->paramB & M644_PLAYER_CONTACT_BITS) != 0) {
        return 0;
    } else {
        return 1;
    }
}
