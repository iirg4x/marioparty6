/* Creates players and advances their route choices, movement, and motion states. */
#include "REL/m644dll/module_types.h"
#include "dolphin/pad.h"
#include "game/hu3d.h"

/* All engine actions except hip drop; team collision groups use paired mask bits. */
#define M644_PLAYER_ACTION_MASK 0xFFFFFFEF
#define M644_PLAYER_COLLISION_MASK 0x21

/* Debug hook for CPU attack-state checks; empty in release builds. */
static inline void DebugValue(s32 value)
{
}

/* CPU chase speed in world units per frame, indexed by difficulty. */
f32 lbl_1_data_178[4] = {10.0f, 10.0f, 11.0f, 14.0f};
/* Character motion resources in the engine player-motion slot order. */
unsigned int lbl_1_data_188[17] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 5),
    DATANUM(DATA_mariomot, 56), DATANUM(DATA_mario, 203),
    DATANUM(DATA_mariomot, 7), DATANUM(DATA_mario, 202),
    DATANUM(DATA_mariomot, 15), DATANUM(DATA_mariomot, 16),
    DATANUM(DATA_mariomot, 17), DATANUM(DATA_mariomot, 18),
    DATANUM(DATA_mariomot, 22), DATANUM(DATA_mariomot, 22),
    0
};
/* The first team activated for the current course section; -1 before either team starts. */
s32 lbl_1_data_1CC = -1;

/* Depth and side offsets scale the approach by each rock type's size. */
static const Vec lbl_1_rodata_168 = {1.8f, 1.2f, 1.1f};
static const Vec lbl_1_rodata_174 = {1.45f, 1.0f, 1.0f};

extern s32 fn_1_9124(s32 selector);
extern s32 fn_1_6588(s32 route);
extern void fn_1_1EA8(s32 entry_index, Point3d *position, Mtx matrix);
extern f32 fn_1_65A4(s32 side, s32 route, s32 variant);
extern void fn_1_6684(s32 route, s32 variant, Vec *out);
extern void fn_1_671C(s32 route, s32 variant, Vec *out);
extern f32 fn_1_6768(s32 route, s32 variant);

extern s32 fn_1_67CC(s32 route, s32 variant);

extern void fn_1_7FF4(s32 group, s32 point, Vec *out);
extern s32 frandmod(s32 range);

/* Per-player course, CPU and boarding state, linked to the other teammate. */
M644PlayerRecord lbl_1_bss_78[4];
typedef struct M644GroupVariants_s {
    /* The next boarding slot (zero or one) for each team. */
    s32 next_slot[2];
} M644GroupVariants;

extern OMOBJMAN *fn_1_A0(void);

extern void fn_1_8064(s32 entry_index, s32 slot, Mtx matrix);

typedef struct M644CameraSelect_s {
    /* One render-camera mask for each team. */
    u32 camera_bit[2];
} M644CameraSelect;

extern int fn_1_213C(COL_NARROW_PARAM *contact, COL_NARROW_PARAM *other_contact);
extern void fn_1_7E5C(HU3D_MODELID modelId, s32 group, s32 variant);

void fn_1_7FF4(s32, s32, Vec *);

s32 fn_1_9124(s32);
s32 fn_1_6588(s32);
void fn_1_8064(s32, s32, Mtx);
void fn_1_1EA8(s32, Point3d *, Mtx);
f32 fn_1_65A4(s32, s32, s32);
void fn_1_6684(s32, s32, Vec *);
void fn_1_671C(s32, s32, Vec *);
f32 fn_1_6768(s32, s32);

void fn_1_7F8C(s16, s32, s32);

void fn_1_6394(OMOBJ *);

void fn_1_21B8(M644PlayerRecord *player);
void fn_1_2288(M644PlayerRecord *player);
void fn_1_24D4(M644PlayerRecord *player);
void fn_1_28CC(M644PlayerRecord *player);
s32 fn_1_2B44(M644PlayerRecord *player);
s32 fn_1_2FE0(M644PlayerRecord *player);
s32 fn_1_32D0(M644PlayerRecord *player);
void fn_1_4440(OMOBJ *object);
void fn_1_46C0(OMOBJ *object);
s32 fn_1_4E84(void);
void fn_1_50E0(s32 player_slot);
void fn_1_51B4(s32 group, s32 phase);
s32 fn_1_5210(s32 group);
void fn_1_5284(s32 group);
void fn_1_5364(s32 group);
void fn_1_5444(s32 group);
void fn_1_54F0(s32 group);
void fn_1_559C(s32 group);
void fn_1_5660(s32 group);
void fn_1_56D4(s32 group);
void fn_1_5780(OMOBJ *object, HuVecF *position_out);
void fn_1_58A8(OMOBJ *object);
void fn_1_5AE4(OMOBJ *object);
void fn_1_5C04(OMOBJ *object);
void fn_1_5F50(OMOBJ *object);
void fn_1_60E0(OMOBJ *object);
void fn_1_6250(OMOBJ *object);
void fn_1_3338(OMOBJ *object);
void fn_1_3BBC(OMOBJ *object);
void fn_1_4940(OMOBJ *object);
s32 fn_1_5168(s32 player_slot);
void fn_1_5180(s32 player_slot);
s16 fn_1_519C(s32 player_slot);
MGPLAYER *fn_1_5150(s32 player_slot);

/* Resets CPU planning after a hit or spawn, then chooses a fresh route target. */
void fn_1_215C(M644PlayerRecord *player) {
    if (player->player_type != 0) {
        player->cpu_state = 0;
        player->cpu_timer = 0;
        player->selected_lane = -1;
        fn_1_21B8(player);
        fn_1_24D4(player);
    }
}

/* Caches the current route's rock life and approach points for CPU target selection. */
void fn_1_21B8(M644PlayerRecord *player)
{
    Point3d section_origin;
    s32 route;
    s32 route_selector;
    s32 lane_index;

    route = fn_1_9124(player->group);
    route_selector = fn_1_6588(route);
    fn_1_1EA8(route >> 1, &section_origin, player->section_matrix);
    for (lane_index = 0; lane_index < 3; lane_index++) {
        player->rock_life[lane_index] = fn_1_65A4(player->group, route, lane_index);
        fn_1_6684(route, lane_index, (Vec *)player->world_collision_points[lane_index]);
        fn_1_671C(route, lane_index, (Vec *)player->local_collision_points[lane_index]);
    }
    player->reference_radius = fn_1_6768(route, 0);
}

/* Offsets a CPU approach point around its teammate; called while choosing or following a target. */
void fn_1_2288(M644PlayerRecord *player)
{
    s32 lane_index;
    s32 route;
    s32 rock_type;
    s32 turn_side;
    Vec teammate_position;
    Vec approach_position;
    Vec rock_reference;
    Vec depth_offsets;
    Vec side_offsets;

    lane_index = player->selected_lane;
    approach_position = *(Vec *)player->local_collision_points[lane_index];
    rock_reference = *(Vec *)player->world_collision_points[lane_index];
    route = fn_1_9124(player->group);
    rock_type = fn_1_67CC(route, 0);
    depth_offsets = lbl_1_rodata_168;
    side_offsets = lbl_1_rodata_174;
    MgActorPosGet(player->teammate->player->actor, &teammate_position);
    approach_position.z += player->reference_radius * ((f32 *)&depth_offsets)[rock_type];
    PSMTXMultVec(player->section_matrix, &approach_position, &approach_position);
    turn_side = (s32)((approach_position.x * (teammate_position.z - rock_reference.z)) +
                      ((teammate_position.x * (rock_reference.z - approach_position.z)) +
                       (rock_reference.x * (approach_position.z - teammate_position.z))));
    if (turn_side != 0) {
        approach_position = *(Vec *)player->local_collision_points[lane_index];
        approach_position.z += player->reference_radius * ((f32 *)&depth_offsets)[rock_type];
        if (turn_side > 0) {
            approach_position.x += (player->reference_radius / 2.0f) *
                          ((f32 *)&side_offsets)[rock_type];
        } else {
            approach_position.x -= (player->reference_radius / 2.0f) *
                          ((f32 *)&side_offsets)[rock_type];
        }
        PSMTXMultVec(player->section_matrix, &approach_position, &approach_position);
    }
    *(Vec *)player->approach_point = approach_position;
}

/* Chooses a rock approach point and starts CPU movement there when planning a route. */
void fn_1_24D4(M644PlayerRecord *player)
{
    s32 selected_lane;
    s32 lane_index;
    s32 attempts;
    f32 first_valid_life;

    selected_lane = 0;
    first_valid_life = -1.0f;
    if (player->com_difficulty == 2 || player->com_difficulty == 3) {
        /* Later candidates are compared with the first valid rock's life, not a running minimum. */
        for (lane_index = 0; lane_index < 3; lane_index++) {
            if (player->rock_life[lane_index] < 0.0f) {
                continue;
            }
            if (first_valid_life < 0.0f) {
                first_valid_life = player->rock_life[lane_index];
                selected_lane = lane_index;
            } else {
                if (player->rock_life[lane_index] < first_valid_life) {
                    selected_lane = lane_index;
                }
                if (player->rock_life[lane_index] == first_valid_life &&
                    (rand8() % 100) < 50) {
                    selected_lane = lane_index;
                }
            }
        }
        if (player->teammate->com_difficulty == 2 ||
            player->teammate->com_difficulty == 3) {
            if (player->teammate->selected_lane != -1) {
                selected_lane = player->teammate->selected_lane;
            }
        }
    } else {
        /* After ten attempts the last random lane is kept even if its rock is unavailable. */
        attempts = 10;
        do {
            lane_index = rand8() % 3;
            attempts--;
        } while (attempts != 0 &&
                 player->rock_life[lane_index] < 0.0f);
        selected_lane = lane_index;
    }

    player->selected_lane = selected_lane;
    fn_1_2288(player);
    MgPlayerComStkOn(player->player);
    MgPlayerVecChase(player->player, (Vec *)player->approach_point,
                     player->chase_speed, 0.0f);
}

/* Chooses a randomized point around the midpoint between the route origin and averaged boarding
 * hooks for easier CPUs. */
void fn_1_28CC(M644PlayerRecord *player)
{
    s32 route;
    Mtx direction;
    Vec center;
    Vec midpoint;
    Vec first_point;
    Vec second_point;

    route = fn_1_9124(player->group);
    player->selected_lane = 0;
    fn_1_1EA8(route >> 1, &center, direction);
    fn_1_7FF4(player->group, 0, &first_point);
    fn_1_7FF4(player->group, 1, &second_point);
    midpoint.x = (first_point.x + second_point.x) / 2.0f;
    midpoint.y = (first_point.y + second_point.y) / 2.0f;
    midpoint.z = (first_point.z + second_point.z) / 2.0f;
    player->approach_point[0] = (center.x + midpoint.x) / 2.0f;
    player->approach_point[1] = (center.y + midpoint.y) / 2.0f;
    player->approach_point[2] = (center.z + midpoint.z) / 2.0f;
    OSReport("%d,  %d : %f, %f, %f\n", route >> 1, player->slot,
             player->approach_point[0], player->approach_point[1], player->approach_point[2]);
    player->approach_point[0] += (f32)(u32)frandmod(400) - 200.0f;
    if ((route >> 1) == 4) {
        player->approach_point[2] += (f32)(u32)frandmod(140) - 40.0f;
    } else {
        player->approach_point[2] += (f32)(u32)frandmod(200) - 100.0f;
    }
    MgPlayerComStkOn(player->player);
    MgPlayerVecChase(player->player, (Vec *)player->approach_point, player->chase_speed, 0.0f);
    OSReport("target %d : %f, %f, %f\n", player->slot, player->approach_point[0],
             player->approach_point[1], player->approach_point[2]);
}

/* Advances CPU movement toward its target each frame and reports arrival at striking distance. */
s32 fn_1_2B44(M644PlayerRecord *player)
{
    s32 route;
    s32 route_selector;
    s32 lane_index;
    s32 point_index;
    Vec chase_position;
    Vec player_position;
    Vec linked_position;
    Vec difference;
    f32 partner_distance_limit;
    Point3d section_origin;

    player->cpu_state = 2;
    if (player->selected_lane == -1) {
        route = fn_1_9124(player->group);
        route_selector = fn_1_6588(route);
        fn_1_1EA8(route >> 1, &section_origin, player->section_matrix);
        for (lane_index = 0; lane_index < 3; lane_index++) {
            player->rock_life[lane_index] = fn_1_65A4(player->group, route, lane_index);
            fn_1_6684(route, lane_index, (Vec *)player->world_collision_points[lane_index]);
            fn_1_671C(route, lane_index, (Vec *)player->local_collision_points[lane_index]);
        }
        player->reference_radius = fn_1_6768(route, 0);
        fn_1_24D4(player);
    }
    if (MgPlayerVecChase(player->player, (Vec *)player->approach_point,
                         player->chase_speed, 40.0f)) {
        point_index = player->selected_lane;
        chase_position = *(Vec *)player->local_collision_points[point_index];
        PSMTXMultVec(player->section_matrix, &chase_position, &chase_position);
        MgPlayerComStkOff(player->player);
        player->cpu_state = 3;
        MgActorVecChase(player->player->actor, &chase_position,
                        0.0f, 0.0f);
        return 1;
    }
    if (player->cpu_state != 7 &&
        (player->com_difficulty == 0 || player->com_difficulty == 1)) {
        partner_distance_limit = 112.0f;
        partner_distance_limit *= partner_distance_limit;
        MgActorPosGet(player->player->actor, &player_position);
        MgActorPosGet(player->teammate->player->actor, &linked_position);
        PSVECSubtract(&player_position, &linked_position, &difference);
        difference.y = 0.0f;
        /* The OR between mode tests accepts every teammate mode; proximity still triggers the jump
         * request. */
        if (PSVECSquareMag(&difference) < partner_distance_limit &&
            player->teammate->cpu_state != 7 &&
            (player->teammate->player->mode != MGPLAYER_MODE_JUMP ||
             player->teammate->player->mode != MGPLAYER_MODE_KICK)) {
            MgPlayerModeLandSet(player->player, MGPLAYER_MODE_JUMP);
            player->cpu_state = 7;
        }
    }
    if (player->com_difficulty != 0 && player->com_difficulty != 1) {
        fn_1_2288(player);
    }
    return 0;
}

/* Supplies CPU jump and attack inputs after arrival, using difficulty to choose their timing. */
s32 fn_1_2FE0(M644PlayerRecord *player)
{
    s32 chance;

    if (player->player->mode == MGPLAYER_MODE_PUNCH) {
        return;
    }
    if (player->player->mode == MGPLAYER_MODE_KICK) {
        return;
    }
    if (player->player->mode == MGPLAYER_MODE_JUMP) {
        player->cpu_timer--;
        if (player->cpu_timer == 0) {
            MgPlayerPadSet(player->player, 0, 0, PAD_BUTTON_B, 0);
            player->cpu_state = 6;
        } else if (player->cpu_timer != 1) {
            MgPlayerPadSet(player->player, 0, 0, 0, PAD_BUTTON_A);
        }
    } else {
        chance = 0;
        switch (player->com_difficulty) {
        case 0:
            chance = 5;
            break;
        case 1:
            chance = 40;
            break;
        case 2:
            chance = 50;
            break;
        case 3:
            chance = 65;
            if (lbl_1_data_1CC != player->group || lbl_1_data_1CC == -1) {
                chance = 80;
            }
            break;
        }

        if (rand8() % 100 > chance) {
            MgPlayerModeLandSet(player->player, MGPLAYER_MODE_PUNCH);
            player->cpu_state = 6;
            player->cpu_timer = 0;
        } else {
            MgPlayerPadSet(player->player, 0, 0, PAD_BUTTON_A, 0);
            switch (player->com_difficulty) {
            case 0:
                player->cpu_timer = (rand8() % 4) + 12;
                break;
            case 1:
                player->cpu_timer = (rand8() % 4) + 10;
                break;
            case 2:
                player->cpu_timer = (rand8() % 4) + 6;
                break;
            case 3:
                player->cpu_timer = (rand8() % 4) + 4;
                if (lbl_1_data_1CC != player->group || lbl_1_data_1CC == -1) {
                    player->cpu_timer = (rand8() % 2) + 4;
                }
                break;
            }
        }
    }
    if (player->cpu_timer != 0) {
        return;
    }
    if (player->cpu_state == 6) {
        return;
    }
    DebugValue(player->cpu_state != 6);
}

/* Lets hard CPUs plan again once an attack ends and the player is back on the ground. */
s32 fn_1_32D0(M644PlayerRecord *player)
{
    if (player->player->mode != MGPLAYER_MODE_PUNCH && player->player->mode != MGPLAYER_MODE_KICK &&
        MgPlayerModeAttrCheck(player->player, MGPLAYER_MODEATTR_AIR) == 0) {
        player->cpu_state = 0;
    }
}

/* Runs the easy CPU's per-frame plan, movement, waiting and attack states. */
void fn_1_3338(OMOBJ *object)
{
    M644PlayerRecord *data = object->data;
    MGPLAYER *player = data->player;

    if (MgPlayerVibAttrCheck(data->player, (MGPLAYER_VIBATTR_HIT | MGPLAYER_VIBATTR_KNOCKBACK)) !=
        0) {
        fn_1_215C(data);
    }

    switch (data->cpu_state) {
    case 0:
        fn_1_21B8(data);
        if ((s32)(rand8() % 100) < 50) {
            fn_1_24D4(data);
        } else {
            fn_1_28CC(data);
        }
        data->cpu_timer = 0;
        fn_1_2B44(data);
        return;
    case 2:
    case 7:
        fn_1_2B44(data);
        return;
    case 3:
        if ((s32)(rand8() % 100) < 50) {
            data->cpu_state = 4;
            data->cpu_timer = 10;
        } else {
            data->cpu_state = 1;
            data->cpu_timer = 40;
        }
        break;
    case 4:
        data->cpu_timer--;
        if (data->cpu_timer == 0) {
            data->cpu_state = 5;
            fn_1_2FE0(data);
            return;
        }
        break;
    case 5:
        fn_1_2FE0(data);
        return;
    case 6:
        if (data->player->mode != MGPLAYER_MODE_PUNCH && data->player->mode != MGPLAYER_MODE_KICK &&
            MgPlayerModeAttrCheck(data->player, MGPLAYER_MODEATTR_AIR) == 0) {
            data->cpu_state = 0;
            return;
        }
        break;
    case 1:
        data->cpu_timer--;
        if (data->cpu_timer == 0) {
            fn_1_21B8(data);
            if ((rand8() % 100) < 50) {
                fn_1_24D4(data);
            } else {
                fn_1_28CC(data);
            }
            data->cpu_timer = 0;
            fn_1_2B44(data);
        }
        break;
    }
}

/* Runs the normal CPU's per-frame states, favoring rocks more often than the easy CPU. */
void fn_1_3BBC(OMOBJ *object)
{
    M644PlayerRecord *data = object->data;
    MGPLAYER *player = data->player;

    if (MgPlayerVibAttrCheck(data->player, (MGPLAYER_VIBATTR_HIT | MGPLAYER_VIBATTR_KNOCKBACK)) !=
        0) {
        fn_1_215C(data);
    }

    switch (data->cpu_state) {
    case 0:
        fn_1_21B8(data);
        if ((s32)(rand8() % 100) < 75) {
            fn_1_24D4(data);
        } else {
            fn_1_28CC(data);
        }
        data->cpu_timer = 0;
        fn_1_2B44(data);
        return;
    case 2:
    case 7:
        fn_1_2B44(data);
        return;
    case 3:
        if ((s32)(rand8() % 100) < 70) {
            data->cpu_state = 4;
            data->cpu_timer = 8;
        } else {
            data->cpu_state = 1;
            data->cpu_timer = 40;
        }
        break;
    case 4:
        data->cpu_timer--;
        if (data->cpu_timer == 0) {
            data->cpu_state = 5;
            fn_1_2FE0(data);
            return;
        }
        break;
    case 5:
        fn_1_2FE0(data);
        return;
    case 6:
        if (data->player->mode != MGPLAYER_MODE_PUNCH && data->player->mode != MGPLAYER_MODE_KICK &&
            MgPlayerModeAttrCheck(data->player, MGPLAYER_MODEATTR_AIR) == 0) {
            data->cpu_state = 0;
            return;
        }
        break;
    case 1:
        data->cpu_timer--;
        if (data->cpu_timer == 0) {
            fn_1_21B8(data);
            if ((s32)(rand8() % 100) < 75) {
                fn_1_24D4(data);
            } else {
                fn_1_28CC(data);
            }
            data->cpu_timer = 0;
            fn_1_2B44(data);
        }
        break;
    }
}

/* Runs the hard CPU each frame, waiting eight frames at its chosen rock before attacking. */
void fn_1_4440(OMOBJ *object)
{
    M644PlayerRecord *player;
    MGPLAYER *mg_player;

    player = object->data;
    mg_player = player->player;
    if (MgPlayerVibAttrCheck(player->player, (MGPLAYER_VIBATTR_HIT | MGPLAYER_VIBATTR_KNOCKBACK)) !=
        0) {
        fn_1_215C(player);
    }
    switch (player->cpu_state) {
    case 0:
        fn_1_21B8(player);
        fn_1_24D4(player);
        fn_1_2B44(player);
        break;
    case 2:
    case 7:
        fn_1_2B44(player);
        break;
    case 3:
        player->cpu_state = 4;
        player->cpu_timer = 8;
        break;
    case 4:
        player->cpu_timer--;
        if (player->cpu_timer == 0) {
            player->cpu_state = 5;
            fn_1_2FE0(player);
        }
        break;
    case 5:
        fn_1_2FE0(player);
        break;
    case 6:
        fn_1_32D0(player);
        break;
    case 1:
        break;
    }
}

/* Runs the very hard CPU each frame, waiting five frames at its chosen rock before attacking. */
void fn_1_46C0(OMOBJ *object)
{
    M644PlayerRecord *player;
    MGPLAYER *mg_player;

    player = object->data;
    mg_player = player->player;
    if (MgPlayerVibAttrCheck(player->player, (MGPLAYER_VIBATTR_HIT | MGPLAYER_VIBATTR_KNOCKBACK)) !=
        0) {
        fn_1_215C(player);
    }
    switch (player->cpu_state) {
    case 0:
        fn_1_21B8(player);
        fn_1_24D4(player);
        fn_1_2B44(player);
        break;
    case 2:
    case 7:
        fn_1_2B44(player);
        break;
    case 3:
        player->cpu_state = 4;
        player->cpu_timer = 5;
        break;
    case 4:
        player->cpu_timer--;
        if (player->cpu_timer == 0) {
            player->cpu_state = 5;
            fn_1_2FE0(player);
        }
        break;
    case 5:
        fn_1_2FE0(player);
        break;
    case 6:
        fn_1_32D0(player);
        break;
    case 1:
        break;
    }
}

/* Dispatches active CPU updates to the configured difficulty's behavior. */
void fn_1_4940(OMOBJ *object)
{
    M644PlayerRecord *data;

    data = object->data;
    switch (data->com_difficulty) {
    case 0:
        fn_1_3338(object);
        break;
    case 1:
        fn_1_3BBC(object);
        break;
    case 2:
        fn_1_4440(object);
        break;
    case 3:
        fn_1_46C0(object);
        break;
    }
}

/* Creates four player objects during scene setup and pairs teammates with opposite boarding
 * slots. */
s32 fn_1_4E84(void)
{
    M644PlayerRecord *player;
    s32 player_index = 0;
    OMOBJ *object;
    s32 teammate_index;

    M644GroupVariants group_start_slots = {{0, 0}};
        group_start_slots.next_slot[0] = rand8() % 2;
        group_start_slots.next_slot[1] = rand8() % 2;
    for (player_index = 0; player_index < 4; player_index++) {
        object = omAddObjEx(fn_1_A0(), 512, 0, 0, 0, fn_1_58A8);
        object->data = &lbl_1_bss_78[player_index];
        memset(object->data, 0, sizeof(M644PlayerRecord));
        player = object->data;
        player->slot = player_index;
        player->group = GwPlayerConf[player_index].grpNo;
        player->boarding_slot = group_start_slots.next_slot[player->group];
        group_start_slots.next_slot[player->group] =
            1 - group_start_slots.next_slot[player->group];
        player->player_type = GwPlayerConf[player_index].type;
        player->com_difficulty = GwPlayerConf[player_index].comDif;
        player->attack_contact = 0;
        player->boarding_phase = 1;
        player->controller_index = GwPlayerConf[player_index].padNo;
        player->cpu_state = 0;
        player->cpu_timer = 0;
        player->selected_lane = -1;
        player->chase_speed = lbl_1_data_178[player->com_difficulty];
    }
    for (player_index = 0; player_index < 4; player_index++) {
        for (teammate_index = 0; teammate_index < 4; teammate_index++) {
            if (player_index != teammate_index &&
                lbl_1_bss_78[player_index].group ==
                    lbl_1_bss_78[teammate_index].group) {
                lbl_1_bss_78[player_index].teammate =
                    &lbl_1_bss_78[teammate_index];
                break;
            }
        }
    }
}

/* Invalidates a broken rock's selected lane for the hitter and any teammate targeting the same
 * lane. */
void fn_1_50E0(s32 player_slot) {
    if (lbl_1_bss_78[player_slot].selected_lane ==
        lbl_1_bss_78[player_slot].teammate->selected_lane) {
        lbl_1_bss_78[player_slot].teammate->selected_lane = -1;
    }
    lbl_1_bss_78[player_slot].selected_lane = -1;
}

/* Returns the engine player for a Rocky Road player slot. */
MGPLAYER *fn_1_5150(s32 player_slot) {
    return lbl_1_bss_78[player_slot].player;
}

/* Returns the attack-contact latch, set at attack count ten and cleared by later attack frames or
 * rock-contact consumption; leaving attack mode does not clear it. */
s32 fn_1_5168(s32 player_slot) {
    return lbl_1_bss_78[player_slot].attack_contact;
}

/* Clears a player's attack-contact trigger after rock collision handling consumes it. */
void fn_1_5180(s32 player_slot) {
    lbl_1_bss_78[player_slot].attack_contact = 0;
}

/* Returns the team assigned to a player slot. */
s16 fn_1_519C(s32 player_slot) {
    return lbl_1_bss_78[player_slot].group;
}

/* Sets both teammates' boarding phase when the route controller advances their sequence. */
void fn_1_51B4(s32 group, s32 phase) {
    s32 player_index = 0;
    for (player_index = 0; player_index < 4; player_index++) {
        if (group == lbl_1_bss_78[player_index].group) {
            lbl_1_bss_78[player_index].boarding_phase = phase;
        }
    }
}

/* Reports whether either teammate is still active while the route controller waits for boarding. */
s32 fn_1_5210(s32 group) {
    s32 active_found;
    s32 player_index;

    active_found = 0;
    player_index = 0;
    while (player_index < 4) {
        if ((group == lbl_1_bss_78[player_index].group) &&
            (lbl_1_bss_78[player_index].active != 0)) {
            active_found = 1;
        }
        player_index += 1;
    }
    return active_found;
}

/* Blends a team's characters to the hip-drop ending pose and resets slot group * 2 to walking
 * mode. */
void fn_1_5284(s32 group) {
    s32 player_index;
    MGPLAYER *player;
    MGPLAYER *group_player;

    player_index = 0;
    for (player_index = 0; player_index < 4; player_index++) {
        if (group == lbl_1_bss_78[player_index].group) {
            player = lbl_1_bss_78[player_index].player;
            CharMotionShiftSet(
                player->charNo,
                player->omObj->mtnId[MGPLAYER_MOT_HIPDROP_END],
                0.0f, 15.0f, HU3D_MOTATTR_LOOP);
            /* The mode reset selects slot group * 2 rather than the loop's current teammate. */
            group_player = lbl_1_bss_78[group * 2].player;
            MgPlayerModeSet(group_player, MGPLAYER_MODE_WALK);
        }
    }
}

/* Blends a team's characters to idle and resets slot group * 2 to walking mode. */
void fn_1_5364(s32 group) {
    s32 player_index;
    MGPLAYER *player;
    MGPLAYER *group_player;

    player_index = 0;
    for (player_index = 0; player_index < 4; player_index++) {
        if (group == lbl_1_bss_78[player_index].group) {
            player = lbl_1_bss_78[player_index].player;
            CharMotionShiftSet(
                player->charNo,
                player->omObj->mtnId[MGPLAYER_MOT_IDLE],
                0.0f, 15.0f, HU3D_MOTATTR_LOOP);
            /* As above, the mode reset selects slot group * 2 for each teammate. */
            group_player = lbl_1_bss_78[group * 2].player;
            MgPlayerModeSet(group_player, MGPLAYER_MODE_WALK);
        }
    }
}

/* Starts the team's jump animation at a route model's motion timing cue. */
void fn_1_5444(s32 group) {
    MGPLAYER *player;
    s32 player_index = 0;

    for (player_index = 0; player_index < 4; player_index += 1) {
        if (group == lbl_1_bss_78[player_index].group) {
            player = lbl_1_bss_78[player_index].player;
            CharMotionShiftSet(
                player->charNo,
                player->omObj->mtnId[MGPLAYER_MOT_JUMP],
                0.0f,
                15.0f,
                0);
        }
    }
}

/* Starts the team's landing animation at the next route model motion timing cue. */
void fn_1_54F0(s32 group) {
    MGPLAYER *player;
    s32 player_index = 0;

    for (player_index = 0; player_index < 4; player_index += 1) {
        if (group == lbl_1_bss_78[player_index].group) {
            player = lbl_1_bss_78[player_index].player;
            CharMotionShiftSet(
                player->charNo,
                player->omObj->mtnId[MGPLAYER_MOT_LAND],
                0.0f,
                15.0f,
                0);
        }
    }
}

/* Starts the team's looping hip-drop pose from the minigame's end-sequence callback. */
void fn_1_559C(s32 group) {
    s32 player_index = 0;
    MGPLAYER *player;

    for (player_index = 0; player_index < 4; player_index += 1) {
        if (group == lbl_1_bss_78[player_index].group) {
            player = lbl_1_bss_78[player_index].player;
            CharMotionShiftSet(
                player->charNo,
                player->omObj->mtnId[MGPLAYER_MOT_HIPDROP],
                0.0f,
                15.0f,
                HU3D_MOTATTR_LOOP);
            /* Enables the low attribute bit on the hip-drop motion. */
            Hu3DMotionAttrSet(
                player->omObj->mtnId[MGPLAYER_MOT_HIPDROP],
                1);
        }
    }
}

/* Requests rumble for both teammates when a rock breaks; the engine applies it only to humans. */
void fn_1_5660(s32 group)
{
    s32 player_index = 0;
    for (player_index = 0; player_index < 4; player_index++) {
        if (group == lbl_1_bss_78[player_index].group) {
            omVibrate(player_index, 20, 20, 0);
        }
    }
}

/* Starts the team's hip-drop landing animation from the minigame's end-sequence callback. */
void fn_1_56D4(s32 group) {
    MGPLAYER *player;
    s32 player_index = 0;

    for (player_index = 0; player_index < 4; player_index += 1) {
        if (group == lbl_1_bss_78[player_index].group) {
            player = lbl_1_bss_78[player_index].player;
            CharMotionShiftSet(
                player->charNo,
                player->omObj->mtnId[MGPLAYER_MOT_HIPDROP_LAND],
                0.0f,
                15.0f,
                0);
        }
    }
}

/* Places a character at its boarding hook and optionally returns that world position. */
void fn_1_5780(OMOBJ *object, HuVecF *position_out)
{
    M644PlayerRecord *data;
    Mtx matrix;
    HuVecF rotation;
    HuVecF position;

    data = object->data;
    position.x = 150.0f * data->boarding_slot;
    position.y = 0.0f;
    position.z = -200.0f;
    fn_1_8064(data->group, data->boarding_slot, matrix);
    /* The hook translation replaces the position preset above. */
    Hu3DMtxTransGet(matrix, &position);
    MgActorPosSetRaw(data->player->actor, &position);
    Hu3DModelPosSetV(data->player->actor->mdlId, &position);
    Hu3DMtxRotGet(matrix, &rotation);
    Hu3DModelRotSetV(data->player->actor->mdlId, &rotation);
    if (position_out != NULL) {
        *position_out = position;
    }
}

/* Initializes a player object, hides its character on a boarding hook and waits for play to
 * start. */
void fn_1_58A8(OMOBJ *object)
{
    M644PlayerRecord *data = object->data;
    M644CameraSelect camera_masks = {{1, 2}};
    HuVecF spawn_position;
    MGACTOR_PARAM actor_params;

    actor_params.height = 150.0f;
    actor_params.radius = 40.0f;
    actor_params.param = data->slot;
    actor_params.type = 0;
    actor_params.attr = 0;
    actor_params.correctHookParam = 0;
    actor_params.narrowHook = fn_1_213C;
    actor_params.correctHook = 0;

    data->player = MgPlayerCreate(
        data->slot, &actor_params, 4, camera_masks.camera_bit[data->group],
        M644_PLAYER_ACTION_MASK, lbl_1_data_188);

    {
        static const HuVecF start_position = {0.0f, 0.0f, 89000.0f};
        spawn_position = start_position;
    }
    /* The hook query replaces the preceding start-position preset. */
    fn_1_7FF4(data->group, data->boarding_slot, &spawn_position);
    MgActorPosSet(data->player->actor, &spawn_position);
    Hu3DModelLayerSet(data->player->actor->mdlId, 4);
    CharEffectLayerSet(5);
    CharModelVoiceFlagSet(data->player->charNo, 1);
    MgPlayerVibrateCreate(data->player);
    MgActorColMaskSet(data->player->actor,
                      M644_PLAYER_COLLISION_MASK + data->group * M644_PLAYER_COLLISION_MASK);
    Hu3DModelCameraSet(
        data->player->actor->mdlId, camera_masks.camera_bit[data->group]);
    Hu3DModelAttrReset(data->player->actor->mdlId, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowSet(data->player->actor->mdlId);
    MgPlayerComStkOn(data->player);
    MgActorColAttrSet(data->player->actor, (COLBODY_ATTR_MESHCOL_OFF | COLBODY_ATTR_BODYCOL_OFF));
    MgActorGravitySet(data->player->actor, 0.0f);
    fn_1_7E5C(data->player->actor->mdlId, data->group, data->boarding_slot);
    Hu3DMotionSet(
        data->player->actor->mdlId, data->player->omObj->mtnId[MGPLAYER_MOT_HIPDROP_END]);
    MgPlayerDespawn(data->player);
    object->objFunc = fn_1_5AE4;
}

/* Waits for the play phase, prepares CPU targets and hands the player to the boarding-phase
 * callback. */
void fn_1_5AE4(OMOBJ *object)
{
    M644PlayerRecord *data;
    Point3d section_origin;
    s32 route_selector;
    s32 route;
    s32 lane_index;

    data = object->data;
    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        if (data->player_type != 0) {
            data->cpu_state = 0;
            data->cpu_timer = 0;
            data->selected_lane = -1;
            route = fn_1_9124(data->group);
            route_selector = fn_1_6588(route);
            fn_1_1EA8(route >> 1, &section_origin, data->section_matrix);
            for (lane_index = 0; lane_index < 3; lane_index++) {
                data->rock_life[lane_index] =
                    fn_1_65A4(data->group, route, lane_index);
                fn_1_6684(route, lane_index, (Vec *)data->world_collision_points[lane_index]);
                fn_1_671C(route, lane_index, (Vec *)data->local_collision_points[lane_index]);
            }
            data->reference_radius = fn_1_6768(route, 0);
            fn_1_24D4(data);
        }
        object->objFunc = fn_1_5C04;
    }
}

/* Spawns a waiting player into the course when its team's boarding phase reaches two. */
void fn_1_5C04(OMOBJ *object)
{
    M644PlayerRecord *data;
    s32 chase_route;
    MGPLAYER *player;

    Point3d spawn_position;
    Vec chase_position;

    data = object->data;
    if (data->boarding_phase == 2) {
        data->active = 1;
        if (lbl_1_data_1CC == -1) {
            lbl_1_data_1CC = data->group;
        }

        fn_1_5780(object, &spawn_position);
        player = data->player;
        MgPlayerAttrReset(player, MGPLAYER_ATTR_COMSTK);
        MgPlayerSpawn(data->player, &spawn_position);
        fn_1_7F8C(data->player->actor->mdlId, data->group,
                  data->boarding_slot);
        MgPlayerComStkOff(data->player);
        MgActorColAttrReset(data->player->actor,
                            (COLBODY_ATTR_MESHCOL_OFF | COLBODY_ATTR_BODYCOL_OFF));
        MgActorGravitySet(data->player->actor, 150.0f);
        MgPlayerModeSet(data->player, MGPLAYER_MODE_WALK);
        CharMotionShiftSet(data->player->charNo,
                           *data->player->omObj->mtnId,
                           0.0f, 15.0f,
                           HU3D_MOTATTR_LOOP);
        CharModelVoiceFlagSet(data->player->charNo, 1);
        CharModelLandDustCreate(data->player->charNo, &spawn_position);
        chase_route = fn_1_9124(data->group);
        fn_1_1EA8(chase_route >> 1, &chase_position, data->section_matrix);
        MgActorVecChase(data->player->actor, &chase_position,
                        0.0f, 0.0f);
        if (data->player_type == 1) {
            fn_1_215C(data);
        }
        object->objFunc = fn_1_5F50;
    }
}

/* Updates an active player each frame, exposing attack contact and switching to the return
 * route. */
void fn_1_5F50(OMOBJ *object)
{
    M644PlayerRecord *state;
    MGPLAYER *player;
    s32 attack_motion, contact_frame;
    Vec position;
    state = object->data;
    player = state->player;
    if (player->mode == MGPLAYER_MODE_PUNCH || player->mode == MGPLAYER_MODE_KICK) {
        switch (player->mode) {
        case MGPLAYER_MODE_PUNCH: attack_motion = 0; contact_frame = 10; break;
        case MGPLAYER_MODE_KICK: attack_motion = 1; contact_frame = 10; break;
        }
        state->attack_class = attack_motion;
        if (state->attack_frames == contact_frame) state->attack_contact = 1;
        else state->attack_contact = 0;
        state->attack_frames++;
    } else {
        state->attack_class = 4;
        state->attack_frames = 0;
    }
    if (MgPlayerVibAttrCheck(state->player, MGPLAYER_VIBATTR_SQUISH) != 0)
        CharFXPlay(state->player->charNo, CHARVOICEID(15));
    if (state->player_type == 1) fn_1_4940(object);
    if (state->boarding_phase == 3) {
        MgPlayerComStkOn(state->player);
        fn_1_7FF4(state->group, state->boarding_slot, &position);
        state->return_target = position;
        MgActorColAttrSet(state->player->actor, COLBODY_ATTR_BODYCOL_OFF);
        object->objFunc = fn_1_60E0;
    }
    if (MgSeqModeGet() == MGSEQ_MODE_FINISH || MgSeqModeGet() == MGSEQ_MODE_PREWIN ||
        MgSeqModeGet() == MGSEQ_MODE_WINNER) {
        MgPlayerComStkOn(state->player);
        object->objFunc = NULL;
    }
}

/* Returns an active player to its boarding hook, then hides and attaches the character for
 * travel. */
void fn_1_60E0(OMOBJ *object)
{
    M644PlayerRecord *state;
    MGPLAYER *player;

    state = object->data;
    if (MgSeqModeGet() == MGSEQ_MODE_FINISH ||
        MgSeqModeGet() == MGSEQ_MODE_PREWIN ||
        MgSeqModeGet() == MGSEQ_MODE_WINNER) {
        MgPlayerComStkOn(state->player);
        object->objFunc = NULL;
        return;
    }
    if (state->active != 0) {
        player = state->player;
        MgPlayerAttrSet(player, MGPLAYER_ATTR_COMSTK);
        if (state->player->stunType != 0 ||
            MgPlayerModeAttrCheck(state->player, MGPLAYER_MODEATTR_AIR) != 0 ||
            state->player->mode != MGPLAYER_MODE_WALK) {
            return;
        }
        if (MgPlayerVecChase(state->player, (HuVecF *)&state->return_target,
                             12.0f, 60.0f) != 0) {
            state->player->actor->gravity = 0.0f;
            state->active = 0;
        }
    }
    if (state->boarding_phase == 4) {
        state->attack_class = 4;
        CharModelVoiceFlagSet(state->player->charNo, 0);
        MgPlayerDespawn(state->player);
        fn_1_7E5C(state->player->actor->mdlId, state->group,
                  state->boarding_slot);
        object->objFunc = fn_1_6250;
    }
}

/* Waits while the character rides its route model, then selects the next boarding or final
 * callback. */
void fn_1_6250(OMOBJ *object) {
    M644PlayerRecord *data;

    data = object->data;
    if ((MgSeqModeGet() == MGSEQ_MODE_FINISH) || (MgSeqModeGet() == MGSEQ_MODE_PREWIN) ||
        (MgSeqModeGet() == MGSEQ_MODE_WINNER)) {
        MgPlayerComStkOn(data->player);
        object->objFunc = NULL;
        return;
    }
    if (data->boarding_phase == 1) {
        CharMotionShiftSet(data->player->charNo,
                           data->player->omObj->mtnId[MGPLAYER_MOT_HIPDROP_END], 0.0f, 15.0f,
                           HU3D_MOTATTR_LOOP);
        CharModelVoiceFlagSet(data->player->charNo, 1);
        lbl_1_data_1CC = -1;
        object->objFunc = fn_1_5C04;
    }
    if (data->boarding_phase == 5) {
        CharMotionShiftSet(data->player->charNo,
                           data->player->omObj->mtnId[MGPLAYER_MOT_HIPDROP_END], 0.0f, 15.0f,
                           HU3D_MOTATTR_LOOP);
        CharModelVoiceFlagSet(data->player->charNo, 1);
        object->objFunc = fn_1_6394;
    }
}
