/* Initializes route points and obstacle models and handles their collision animations. */
#include "REL/m644dll/module_types.h"
#include "game/hu3d.h"

M644RouteModelTable lbl_1_bss_5AC;
M644RouteVectors lbl_1_bss_3D8[9];

#define MSM_SE_M644_TEAM0_ROCK_HIT 1966
#define MSM_SE_M644_TEAM1_ROCK_HIT 1967
#define MSM_SE_M644_TEAM0_LARGE_ROCK_HIT 1968
#define MSM_SE_M644_TEAM1_LARGE_ROCK_HIT 1969
#define MSM_SE_M644_ROCK_DAMAGE 1970
#define MSM_SE_M644_LARGE_ROCK_DAMAGE 1971
#define MSM_SE_M644_ROCK_BREAK 1972
#define MSM_SE_M644_LARGE_ROCK_BREAK 1973
#define M644_ROCK_CONTACT_BITS 0x80
#define M644_PLAYER_CONTACT_BITS 0x100
#define M644_ROCK_TEAM_COLLISION_MASK 0x81
#define M644_BROKEN_ROCK_COLLISION_MASK 0xC

/* Rock damage-pose motion resources. */
s32 lbl_1_data_2C0[8] = {
    DATANUM(DATA_m644, 44), DATANUM(DATA_m644, 45), DATANUM(DATA_m644, 46), DATANUM(DATA_m644, 47),
    DATANUM(DATA_m644, 48), DATANUM(DATA_m644, 49), DATANUM(DATA_m644, 50), DATANUM(DATA_m644, 51)
};
/* Motion IDs loaded once and shared by all rocks. */
s16 lbl_1_data_2E0[8] = {-1};
/* Shared rock model, break model and break-motion IDs; -1 before the first rock loads them. */
s16 lbl_1_data_2F0 = -1;
s16 lbl_1_data_2F2 = -1;
s16 lbl_1_data_2F4 = -1;
/* Per-player hit-effect model resources. */
s32 lbl_1_data_2F8[4] = {
    DATANUM(DATA_m644, 62), DATANUM(DATA_m644, 63), DATANUM(DATA_m644, 64), DATANUM(DATA_m644, 65)
};
/* Motions for the four player hit-effect models. */
s32 lbl_1_data_308[4] = {
    DATANUM(DATA_m644, 66), DATANUM(DATA_m644, 67), DATANUM(DATA_m644, 68), DATANUM(DATA_m644, 69)
};
/* Loaded hit-effect model IDs, selected by player slot. */
s16 lbl_1_data_318[4] = {-1, 0, 0, 0};
/* Per rock type: life, unexamined bytes, radius, model scale and collision height. */
M644Data320Entry lbl_1_data_320[3] = {
    {4, {0, 0, 66, 250, 0, 0}, 74.0f, 1.0f, 162.5f},
    {8, {0, 0, 67, 72, 0, 0}, 230.0f, 1.6f, 400.0f},
    {16, {0, 0, 67, 250, 0, 0}, 400.0f, 4.0f, 1000.0f}
};
/* Rock counts and lane assignments for the five course sections; -1 means no rock. */
M644RouteEntry lbl_1_data_35C[5] = {
    {2, {1, 2, -1}}, {1, {0, -1, -1}}, {3, {3, 4, 5}}, {2, {6, 7, -1}}, {1, {8, -1, -1}}
};

typedef struct M644CameraIds {
    /* Camera-selection bits for the two teams. */
    s32 masks[2];
} M644CameraIds;

extern OMOBJMAN *fn_1_A0(void);
extern void fn_1_7548(OMOBJ *object);
extern s32 fn_1_519C(s32 index);

/* Leaves the player unchanged after its final travel callback ends. */
void fn_1_6394(void)
{
}

/* Adds a rock's model and collision offsets to caller-provided positions, wrapping the team slot to
 * nine rocks. */
void fn_1_6398(s32 rock_slot, f32 *model_position, f32 *collision_position) {
    s32 quotient;
    s32 index;

    quotient = rock_slot / 9;
    index = rock_slot - (quotient * 9);
    model_position[0] += lbl_1_bss_3D8[index].local_model_offset.x;
    model_position[1] += lbl_1_bss_3D8[index].local_model_offset.y;
    model_position[2] += lbl_1_bss_3D8[index].local_model_offset.z;
    collision_position[0] += lbl_1_bss_3D8[index].local_collision_offset.x;
    collision_position[1] += lbl_1_bss_3D8[index].local_collision_offset.y;
    collision_position[2] += lbl_1_bss_3D8[index].local_collision_offset.z;
}

/* Returns the rock type for a team rock slot, shared between the two teams. */
f32 fn_1_6494(s32 entry_index)
{
    s32 quotient = entry_index / 9;
    return (f32)lbl_1_bss_3D8[entry_index - (quotient * 9)].rock_type;
}

/* Returns the initial life for a rock type. */
s16 fn_1_6500(s32 rock_type) {
    return lbl_1_data_320[rock_type].initial_life;
}

/* Returns a rock type's collision radius as an integer. */
s32 fn_1_6518(s32 rock_type) {
    return (s32) lbl_1_data_320[rock_type].radius;
}

/* Returns a rock type's collision height as an integer. */
s32 fn_1_6544(s32 rock_type) {
    return (s32) lbl_1_data_320[rock_type].height;
}

/* Returns the model scale for a rock type. */
f32 fn_1_6570(s32 rock_type) {
    return lbl_1_data_320[rock_type].model_scale;
}

/* Returns the number of rocks in the route section selected by a paired motion index. */
s16 fn_1_6588(s32 route_motion) {
    return lbl_1_data_35C[route_motion >> 1].rock_count;
}

/* Returns a team's remaining life for a rock in the current section, or -1 for an absent lane. */
float fn_1_65A4(s32 side, s32 route, s32 variant)
{
    s32 section_index, team_base, rock_slot;
    section_index = route >> 1;
    team_base = side * 9;
    rock_slot = team_base + lbl_1_data_35C[section_index].lane_slots[variant];
    if (lbl_1_data_35C[section_index].lane_slots[variant] == -1) return -1.0f;
    return lbl_1_bss_5AC.entry[rock_slot].life;
}

/* Returns a section rock's world model position for placement queries. */
void fn_1_6638(s32 route, s32 variant, Vec *out)
{
    *out = lbl_1_bss_3D8[lbl_1_data_35C[route >> 1].lane_slots[variant]].world_model_position;
}

/* Returns a section rock's world collision position for CPU approach calculations. */
void fn_1_6684(s32 route, s32 variant, Vec *out)
{
    *out = lbl_1_bss_3D8[lbl_1_data_35C[route >> 1].lane_slots[variant]].world_collision_position;
}

/* Returns a section rock's local model offset for placement queries. */
void fn_1_66D0(s32 route, s32 variant, Vec *out)
{
    *out = lbl_1_bss_3D8[lbl_1_data_35C[route >> 1].lane_slots[variant ]].local_model_offset;
}

/* Returns a section rock's local collision offset for CPU approach calculations. */
void fn_1_671C(s32 route, s32 variant, Vec *out)
{
    *out = lbl_1_bss_3D8[lbl_1_data_35C[route >> 1].lane_slots[variant ]].local_collision_offset;
}

/* Returns the collision radius of a section rock for CPU approach spacing. */
float fn_1_6768(s32 route, s32 variant)
{
    s32 rock_slot, rock_type;
    rock_slot = lbl_1_data_35C[route >> 1].lane_slots[variant];
    rock_type = lbl_1_bss_3D8[rock_slot].rock_type;
    return lbl_1_data_320[rock_type].radius;
}

/* Returns the rock type of a selected section lane for CPU approach offsets. */
s32 fn_1_67CC(s32 route_motion, s32 lane_index) {
    s32 selected = lbl_1_data_35C[route_motion >> 1].lane_slots[lane_index];
    return lbl_1_bss_3D8[selected].rock_type;
}

/* Returns the rock slot assigned to a section lane. */
s16 fn_1_6814(s32 route_motion, s32 lane_index) {
    return lbl_1_data_35C[route_motion >> 1].lane_slots[lane_index];
}

/* Orders the route's rock points, creates both teams' rock objects and loads player hit-effect
 * models. */
s32 fn_1_6838(void)
{
    s32 index = 0;
    M644RouteVectors route_vectors[9] = {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, -70.0f}, {0.0f, 0.0f, 0.0f}, 0},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {125.0f, 0.0f, -70.0f}, {125.0f, 0.0f, 0.0f}, 0},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {-125.0f, 0.0f, -70.0f}, {-125.0f, 0.0f, 0.0f}, 0},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {-250.0f, 0.0f, -70.0f}, {-250.0f, 0.0f, 0.0f}, 0},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, -70.0f}, {0.0f, 0.0f, 0.0f}, 0},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {250.0f, 0.0f, -70.0f}, {250.0f, 0.0f, 0.0f}, 0},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {-240.00001525878906f, 0.0f, -54.0f},
         {-240.00001525878906f, 0.0f, -80.0f}, 1},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {240.00001525878906f, 0.0f, -54.0f},
         {240.00001525878906f, 0.0f, -80.0f}, 1},
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f},
         {0.0f, 0.0f, -54.0f}, {0.0f, 0.0f, 0.0f}, 2}
    };
    s16 route_groups[9];
    s32 variant_index;
    s32 selector_count;
    s32 vector_index;
    s32 route_index;
    s32 data_index;
    s16 motion_id;
    s32 camera_index;
    OMOBJ *object;
    M644RouteModelEntry *entry;

    vector_index = 0;
    route_index = 0;
    /* Rewrite section lanes into contiguous rock slots while preserving their authored
     * positions. */
    for (index = 0; index < 5; index++) {
        selector_count = lbl_1_data_35C[index].rock_count;
        for (variant_index = 0; variant_index < selector_count;
             variant_index++) {
            data_index = lbl_1_data_35C[index].lane_slots[variant_index];
            lbl_1_bss_3D8[vector_index] = route_vectors[data_index];
            vector_index++;
            lbl_1_data_35C[index].lane_slots[variant_index] =
                route_index;
            route_groups[route_index] = index;
            route_index++;
        }
    }

    for (index = 0; index < 18; index++) {
        object = omAddObjEx(fn_1_A0(), 384, 0, 0, 0, fn_1_7548);
        object->data = &lbl_1_bss_5AC.entry[index];
        memset(object->data, 0, sizeof(M644RouteModelEntry));
        entry = object->data;
        entry->slot = index;
        entry->side = index / 9;
        entry->rock_type = (s16)fn_1_6494(index);
        entry->life = (f32)fn_1_6500(entry->rock_type);
        entry->route_group = route_groups[index % 9];
    }

    for (index = 0; index < 4; index++) {
        M644CameraIds camera_ids = {{1, 2}};
        lbl_1_data_318[index] =
            Hu3DModelCreate(HuDataSelHeapReadNum(
                lbl_1_data_2F8[index], HU_MEMNUM_OVL, HEAP_MODEL));
        motion_id = Hu3DJointMotion(
            lbl_1_data_318[index],
            HuDataSelHeapReadNum(
                lbl_1_data_308[index], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet(lbl_1_data_318[index], 6);
        Hu3DModelAttrSet(lbl_1_data_318[index], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_data_318[index], HU3D_MOTATTR_LOOP);
        Hu3DMotionSet(lbl_1_data_318[index], motion_id);
        camera_index = fn_1_519C(index);
        Hu3DModelCameraSet(lbl_1_data_318[index], camera_ids.masks[camera_index]);
    }
}

typedef struct M644MotionTable_s {
    /* Rock motion slot selected by type and truncated remaining life. */
    s16 motion[3][16];
} M644MotionTable;

typedef struct M644RockMotionTable_s {
    /* Rock motion slot selected by type and truncated remaining life. */
    s16 motion[3][16];
} M644RockMotionTable;

extern void fn_1_50E0(s32 model_index);
extern s32 fn_1_5168(s32 model_index);
extern void fn_1_5180(s32 model_index);
extern void fn_1_5660(s32 group);

typedef struct M644RouteCameraIds_s {
    /* Camera-selection bits for the two teams. */
    s32 masks[2];
} M644RouteCameraIds;

extern s16 GwMgNightF;
extern void fn_1_1EA8(s32 route, Point3d *position, Mtx matrix);

s32 fn_1_6C90(s32 model_index, s32 contact_type);
int fn_1_6FBC(COL_NARROW_PARAM *rock_contact, COL_NARROW_PARAM *other_contact);
void fn_1_7548(OMOBJ *object);
void fn_1_7BCC(OMOBJ *object);
void fn_1_7C24(s32 team_index, s32 section_index);

/* Applies one contact's damage to a rock, selecting its damaged pose or starting its break
 * animation. */
s32 fn_1_6C90(s32 model_index, s32 contact_type)
{
    f32 damage;
    s32 frame_index;
    s16 motion_number;
    HU3D_MODELID model_id;
    s32 motion_table_offset;
    s32 rock_index;

    damage = contact_type == 3 ? 1.2f : 0.7f;
    lbl_1_bss_5AC.entry[model_index].life -= damage;
    if (lbl_1_bss_5AC.entry[model_index].life < 0.0f) {
        OSReport("life = %f\n", lbl_1_bss_5AC.entry[model_index].life);
        Hu3DModelAttrSet(lbl_1_bss_5AC.entry[model_index].model, HU3D_ATTR_DISPOFF);
        MgActorColAttrSet(lbl_1_bss_5AC.entry[model_index].actor, M644_BROKEN_ROCK_COLLISION_MASK);
        Hu3DModelAttrReset(lbl_1_bss_5AC.entry[model_index].motion_model, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_5AC.entry[model_index].motion_model, HU3D_MOTATTR_PAUSE);
        Hu3DMotionTimeSet(lbl_1_bss_5AC.entry[model_index].motion_model, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_5AC.entry[model_index].motion_model, 1.0f);
        return 1;
    }

    {
    M644MotionTable motion_table = {{
        {5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {7, 7, 7, 5, 5, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {7, 7, 7, 6, 6, 5, 5, 4, 4, 3, 3, 2, 2, 1, 1, 0}
    }};
    frame_index = (s32)lbl_1_bss_5AC.entry[model_index].life;
    motion_number = motion_table.motion[lbl_1_bss_5AC.entry[model_index].rock_type]
                                      [(s16)frame_index];
    model_id = lbl_1_bss_5AC.entry[model_index].model;
    /* Damage-pose sounds play only when the remaining life selects a different motion. */
    if (Hu3DMotionIDGet(model_id) != lbl_1_data_2E0[motion_number]) {
        if (lbl_1_bss_5AC.entry[model_index].rock_type == 2) {
            HuAudFXPlayPan(MSM_SE_M644_LARGE_ROCK_DAMAGE,
                           (s16) ((lbl_1_bss_5AC.entry[model_index].side << 5) + 48));
        } else {
            HuAudFXPlayPan(MSM_SE_M644_ROCK_DAMAGE,
                           (s16) ((lbl_1_bss_5AC.entry[model_index].side << 5) + 48));
        }
    }
    Hu3DMotionSet(model_id, lbl_1_data_2E0[motion_number]);
    Hu3DModelAttrReset(model_id, (HU3D_MOTATTR_LOOP | HU3D_MOTATTR_PAUSE));
    Hu3DMotionTimeSet(model_id, 0.0f);
    return 0;
    }
}

/* Handles rock contacts during collision updates, consuming player attack pulses and showing hit
 * effects. */
int fn_1_6FBC(COL_NARROW_PARAM *rock_contact, COL_NARROW_PARAM *other_contact)
{
    s32 model_index;
    s32 player_slot;
    s32 effect_index;
    Vec target_position;
    Vec source_position;

    if ((rock_contact->paramB & M644_ROCK_CONTACT_BITS) != 0 &&
        (other_contact->paramB & M644_ROCK_CONTACT_BITS) != 0) {
        return 0;
    }
    if ((other_contact->paramB & M644_PLAYER_CONTACT_BITS) != 0 &&
        fn_1_5168(other_contact->paramB - M644_PLAYER_CONTACT_BITS) != 0) {
        model_index = rock_contact->paramB - M644_ROCK_CONTACT_BITS;
        player_slot = other_contact->paramB - M644_PLAYER_CONTACT_BITS;

        if (fn_1_6C90(model_index, other_contact->type) == 1) {
            fn_1_50E0(player_slot);
            fn_1_5660(lbl_1_bss_5AC.entry[model_index].side);
            if (model_index == 8 || model_index == 17) {
                HuAudFXPlayPan(MSM_SE_M644_LARGE_ROCK_BREAK,
                               (s16) ((lbl_1_bss_5AC.entry[model_index].side << 5) + 48));
            } else {
                HuAudFXPlayPan(MSM_SE_M644_ROCK_BREAK,
                               (s16) ((lbl_1_bss_5AC.entry[model_index].side << 5) + 48));
            }
        } else {
        OSReport("rock group : %d\n",
                 lbl_1_bss_5AC.entry[model_index].side);
        if (model_index == 8 || model_index == 17) {
            struct M644RockSoundIds {
                /* Per-team impact sound IDs. */
                s32 ids[2];
            } special_sound_ids = { { MSM_SE_M644_TEAM0_LARGE_ROCK_HIT,
                                      MSM_SE_M644_TEAM1_LARGE_ROCK_HIT } };

            HuAudFXPlay(special_sound_ids.ids[lbl_1_bss_5AC.entry[model_index].side]);
        } else {
            struct M644RockSoundIds {
                /* Per-team impact sound IDs. */
                s32 ids[2];
            } group_sound_ids = {{MSM_SE_M644_TEAM0_ROCK_HIT, MSM_SE_M644_TEAM1_ROCK_HIT}};

            HuAudFXPlay(group_sound_ids.ids[lbl_1_bss_5AC.entry[model_index].side]);
    }
        omVibrate(player_slot, 20, 4, 4);
        }

    fn_1_5180(player_slot);
    effect_index = other_contact->paramB - M644_PLAYER_CONTACT_BITS;
    /* Both contact points are copied, but the effect is placed above the player's point. */
    target_position = other_contact->point;
    source_position = rock_contact->point;
    target_position.y += 70.0f;
    Hu3DModelPosSetV(lbl_1_data_318[effect_index], &target_position);
    Hu3DModelAttrReset(lbl_1_data_318[effect_index], HU3D_ATTR_DISPOFF);
    Hu3DMotionTimeSet(lbl_1_data_318[effect_index], 0.0f);
    return 0;
    }
    return 0;
}

/* Creates a rock's day or night model, break model and collision body on its first object
 * update. */
void fn_1_7548(OMOBJ *object)
{
    M644RouteModelEntry *data = object->data;
    s32 pan_index = data->side;
    M644RouteCameraIds camera_ids = {{1, 2}};
    Mtx matrix;
    Point3d model_position;
    Point3d actor_position;
    Point3d rotation;
    MGACTOR_PARAM actor_params;
    s16 model_id;
    s16 motion_model_id;
    s32 model_scale_data_index;
    s32 actor_height_data_index;
    s32 actor_radius_data_index;
    s32 route_index;
    s32 route_slot;
    s32 route_quotient;
    s32 route_point_index;
    s16 night_mode;
    s16 actor_height_value;
    s16 actor_height;
    s16 actor_radius_value;
    s16 actor_radius;
    f32 model_scale;
    f32 actor_scale;
    f32 motion_model_scale;

    if (lbl_1_data_2F0 == -1) {
        night_mode = GwMgNightF;
        if (night_mode == 0) {
            model_id = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_m644, 42), HU_MEMNUM_OVL, HEAP_MODEL));
            motion_model_id = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_m644, 53), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model_id = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_m644, 43), HU_MEMNUM_OVL, HEAP_MODEL));
            motion_model_id = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_m644, 52), HU_MEMNUM_OVL, HEAP_MODEL));
        }
        for (route_index = 0; route_index < 8; route_index++) {
            lbl_1_data_2E0[route_index] = Hu3DJointMotion(
                model_id, HuDataSelHeapReadNum(
                    lbl_1_data_2C0[route_index], HU_MEMNUM_OVL, HEAP_MODEL));
        }
        lbl_1_data_2F4 = Hu3DJointMotion(
            motion_model_id,
            HuDataSelHeapReadNum(DATANUM(DATA_m644, 54), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_data_2F0 = model_id;
        lbl_1_data_2F2 = motion_model_id;
    } else {
        model_id = Hu3DModelLink(lbl_1_data_2F0);
        motion_model_id = Hu3DModelLink(lbl_1_data_2F2);
    }

    data->model = model_id;
    data->motion_model = motion_model_id;
    Hu3DModelCameraSet(model_id, camera_ids.masks[pan_index]);
    /* Group nine is also accepted here, although the section table assigns only zero through
     * four. */
    if (data->route_group == 0 || data->route_group == 9) {
        Hu3DModelAttrReset(model_id, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(model_id, HU3D_ATTR_DISPOFF);
    }
    Hu3DMotionSet(model_id, lbl_1_data_2E0[0]);
    Hu3DMotionTimeSet(model_id, 0.0f);
    model_scale_data_index = data->rock_type;
    model_scale = lbl_1_data_320[model_scale_data_index].model_scale;
    actor_scale = model_scale;
    motion_model_scale = actor_scale;
    Hu3DModelScaleSet(model_id, motion_model_scale,
                      motion_model_scale, motion_model_scale);
    Hu3DModelLayerSet(model_id, 3);
    Hu3DModelCameraSet(motion_model_id, camera_ids.masks[pan_index]);
    Hu3DModelAttrSet(motion_model_id, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(motion_model_id, HU3D_MOTATTR_PAUSE);
    Hu3DMotionSet(motion_model_id, lbl_1_data_2F4);
    Hu3DModelScaleSet(motion_model_id, motion_model_scale,
                      motion_model_scale, motion_model_scale);
    Hu3DModelLayerSet(motion_model_id, 3);

    /* Collision dimensions are truncated to signed whole world units before actor creation. */
    actor_height_data_index = data->rock_type;
    actor_height_value = (s16)lbl_1_data_320[actor_height_data_index].height;
    actor_height = actor_height_value;
    actor_params.height = actor_height;
    actor_radius_data_index = data->rock_type;
    actor_radius_value = (s16)lbl_1_data_320[actor_radius_data_index].radius;
    actor_radius = actor_radius_value;
    actor_params.radius = actor_radius;
    actor_params.param = data->slot + M644_ROCK_CONTACT_BITS;
    actor_params.type = 5;
    actor_params.attr = 0;
    actor_params.correctHookParam = 0;
    actor_params.narrowHook = fn_1_6FBC;
    actor_params.correctHook = NULL;
    data->actor = MgActorCreate(&actor_params, -1);

    fn_1_1EA8(data->route_group, &model_position, matrix);
    Hu3DMtxRotGet(matrix, &rotation);
    model_position.x = 0.0f;
    model_position.y = 0.0f;
    model_position.z = 0.0f;
    actor_position.x = 0.0f;
    actor_position.y = 0.0f;
    actor_position.z = 0.0f;
    route_slot = data->slot;
    route_quotient = route_slot / 9;
    route_point_index = route_slot - (route_quotient * 9);
    model_position.x += lbl_1_bss_3D8[route_point_index].local_model_offset.x;
    model_position.y += lbl_1_bss_3D8[route_point_index].local_model_offset.y;
    model_position.z += lbl_1_bss_3D8[route_point_index].local_model_offset.z;
    actor_position.x += lbl_1_bss_3D8[route_point_index].local_collision_offset.x;
    actor_position.y += lbl_1_bss_3D8[route_point_index].local_collision_offset.y;
    actor_position.z += lbl_1_bss_3D8[route_point_index].local_collision_offset.z;
    PSMTXMultVec(matrix, &model_position, &model_position);
    PSMTXMultVec(matrix, &actor_position, &actor_position);
    if (data->slot < 9) {
        lbl_1_bss_3D8[data->slot].world_model_position = model_position;
        lbl_1_bss_3D8[data->slot].world_collision_position = actor_position;
    }
    OSReport("after: index = %d, pos.x = %f, pos.y = %f, pos.z = %f\n", data->slot,
             actor_position.x, actor_position.y, actor_position.z);
    MgActorPosSet(data->actor, &actor_position);
    Hu3DModelPosSetV(data->model, &model_position);
    Hu3DModelPosSetV(motion_model_id, &model_position);
    Hu3DModelRotSetV(data->model, &rotation);
    rotation.y += 180.0f;
    Hu3DModelRotSetV(motion_model_id, &rotation);
    Hu3DModelShadowSet(data->model);
    Hu3DModelShadowSet(motion_model_id);
    MgActorColMaskSet(data->actor, data->side + M644_ROCK_TEAM_COLLISION_MASK);
    object->objFunc = fn_1_7BCC;
}

/* Hides a rock's break model and stops its callback after the break animation ends. */
void fn_1_7BCC(OMOBJ *object) {
    M644RouteModelEntry *data;

    data = object->data;
    if (Hu3DMotionEndCheck(data->motion_model) != 0) {
        Hu3DModelAttrSet(data->motion_model, HU3D_ATTR_DISPOFF);
        object->objFunc = NULL;
    }
}

/* Reveals a team's rocks in a newly reached section during route travel. */
void fn_1_7C24(s32 team_index, s32 section_index) {
    s32 team_base = team_index * 9;
    s32 rock_index = 0;
    while (rock_index < 9) {
        if (section_index == lbl_1_bss_5AC.entry[team_base + rock_index].route_group) {
            Hu3DModelAttrReset(lbl_1_bss_5AC.entry[team_base + rock_index].model,
                               HU3D_ATTR_DISPOFF);
        }
        rock_index++;
    }
}
