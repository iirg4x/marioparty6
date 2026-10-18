/* Shared Rocky Road player, course, camera and progress state. */
#ifndef REL_M644DLL_MODULE_TYPES_H
#define REL_M644DLL_MODULE_TYPES_H

#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

/* Rock durability, model scale and collision dimensions for the three rock types. */
typedef struct M644Data320Entry_s {
    /* Initial remaining life. */
    s16 initial_life;
    /* Unexamined storage; no behavior is assigned to these bytes. */
    u8 unknown_after_life[6];
    /* Collision radius in world units. */
    f32 radius;
    /* Uniform model scale. */
    f32 model_scale;
    /* Collision height in world units. */
    f32 height;
} M644Data320Entry;

/* Per-player course, boarding and CPU planning state. */
typedef struct M644PlayerRecord_s {
    /* Player slot, zero through three. */
    s16 slot;
    /* Team index, zero or one. */
    s16 group;
    /* Boarding hook slot within the team, zero or one. */
    s16 boarding_slot;
    /* Zero for human input, one for CPU input. */
    s32 player_type;
    /* CPU difficulty, zero through three. */
    s16 com_difficulty;
    /* Controller index. */
    s16 controller_index;
    /* Engine player and actor state. */
    MGPLAYER *player;
    /* Attack effect class: zero for punch, one for kick, four outside those attacks. */
    s16 attack_class;
    /* Frames elapsed while punch or kick is active. */
    s16 attack_frames;
    /* Attack-contact pulse raised at frame ten; later attack frames and rock hits clear it. */
    s32 attack_contact;
    /* Boarding phase: 1 wait, 2 spawn, 3 return, 4 attach, 5 final travel. */
    s32 boarding_phase;
    /* Nonzero while the player is on the course or returning. */
    s16 active;
    /* World position of the return-to-boarding target. */
    Vec return_target;
    /* Other player on this team. */
    struct M644PlayerRecord_s *teammate;
    /* CPU state: 0 plan, 1 wait, 2 move, 3 arrived, 4 delay, 5 attack, 6 recover, 7 jump. */
    s16 cpu_state;
    /* CPU waiting or jump-input countdown in frames. */
    s16 cpu_timer;
    /* Selected rock lane, or -1 when a new choice is needed. */
    s16 selected_lane;
    /* Unexamined storage before the chase settings. */
    u8 unknown_before_chase_speed[6];
    /* CPU chase speed in world units per frame. */
    f32 chase_speed;
    /* Cached remaining rock life by lane; negative entries are unavailable. */
    f32 rock_life[3];
    /* Chosen CPU approach point in world coordinates. */
    f32 approach_point[3];
    /* Rock collision positions in world coordinates, indexed by lane. */
    f32 world_collision_points[3][3];
    /* Rock collision offsets in section-local coordinates, indexed by lane. */
    f32 local_collision_points[3][3];
    /* Reference collision radius used to space the CPU approach. */
    f32 reference_radius;
    /* Current rock section local-to-world transform. */
    f32 section_matrix[3][4];
} M644PlayerRecord;

/* Rock lanes available in one course section. */
typedef struct M644RouteEntry_s {
    /* Number of rocks in this section. */
    s16 rock_count;
    /* Rock slot for each lane, or -1 for an absent lane. */
    s16 lane_slots[3];
} M644RouteEntry;

/* One team's moving route model and boarding sequence. */
typedef struct M644StageModelEntry_s {
    /* Team and render-camera index, zero or one. */
    s32 team_index;
    /* Alternating travel and boarding motion index, zero through ten. */
    s32 route_motion;
    /* Model carrying the player and NPC hooks. */
    s16 route_model;
    /* Model attached at the NPC01 hook. */
    s16 npc01_model;
    /* Model placed at the finish and later attached at NPC02B. */
    s16 npc02_model;
    /* Character model IDs attached to the two player hooks. */
    s16 player_models[2];
    /* Motion cue phase: zero before jump, one before landing, two after landing. */
    s16 timing_phase;
    /* Unexamined storage between cue state and rock count. */
    u8 unknown_before_rock_count[4];
    /* Section rock count; also exposed to the decrement helper. */
    s16 remaining_rocks;
    /* One once result handling requests travel slowdown. */
    s32 slowdown_requested;
    /* Travel motion and sound-volume multiplier, initially one. */
    f32 travel_speed;
    /* Travel sound-effect handle, or -1 after stopping. */
    s32 travel_sound;
    /* Initial travel sound pitch supplied when playback starts. */
    s32 sound_pitch;
} M644StageModelEntry;

/* Route controllers for the two teams. */
typedef struct M644StageModelTable_s {
    /* One controller per team. */
    M644StageModelEntry entry[2];
    /* Unexamined storage after the controllers. */
    u8 unknown_tail[4];
} M644StageModelTable;

/* Animated camera rails and opening split-screen transition state. */
typedef struct M644EffectMenuData_s {
    /* Selected camera-rail model for each team. */
    s16 selector[2];
    /* Seven camera-rail models for each team. */
    s16 camera_models[2][7];
    /* Nonzero when this team's camera motion follows the route slowdown. */
    s32 slowdown_requested[2];
    /* Camera motion-speed value initialized when slowdown is requested. */
    f32 motion_speed[2];
    /* Opening split-screen interpolation amount. */
    f32 split_transition;
} M644EffectMenuData;

/* Course model and motion IDs shared by scene, route and player behavior. */
typedef struct M644ModelScene_s {
    /* Visible course model that receives the shadow map. */
    s16 course_model;
    /* First hidden collision-map model; also supplies collision-section hooks. */
    s16 collision_model;
    /* Second hidden collision-map model. */
    s16 secondary_collision_model;
    /* Model providing the course light information. */
    s16 lighting_model;
    /* First animated course model. */
    s16 primary_animated_model;
    /* Second animated course model. */
    s16 secondary_animated_model;
    /* Looping course model on render layer two. */
    s16 looping_course_model;
    /* Course model on render layer three. */
    s16 scenery_model;
    /* Main motion for the first animated course model. */
    s16 primary_model_motion;
    /* Main motion for the second animated course model. */
    s16 secondary_model_motion;
    /* Unexamined storage before the rock-hook model. */
    u8 unknown_before_rock_hook[2];
    /* Model providing the five rock-section placement hooks. */
    s16 rock_hook_model;
    /* Additional animated model used at night. */
    s16 night_model;
    /* Main motion for the additional night model. */
    s16 night_motion;
    /* Course animation model for each team camera. */
    s16 team_course_models[2];
    /* Three linked day-scene pieces for each team. */
    s16 day_piece_models[2][3];
    /* Main motions for the linked day-scene pieces. */
    s16 day_piece_motions[2][3];
} M644ModelScene;

/* Course progress display and counters for the two teams. */
typedef struct M644DisplayState_s {
    /* Sprite IDs for the progress markers and course display. */
    s16 display_id[19];
    /* Per-team progress ticks, mapped from zero to 1500 onto the display. */
    s32 progress[2];
} M644DisplayState;

/* Rock placement points and offsets shared by both teams. */
typedef struct M644RouteVectors {
    /* Rock model position in world coordinates. */
    Vec world_model_position;
    /* Collision-body position in world coordinates. */
    Vec world_collision_position;
    /* Rock model offset in section-local coordinates. */
    Vec local_model_offset;
    /* Collision-body offset in section-local coordinates. */
    Vec local_collision_offset;
    /* Rock type selecting durability, scale and dimensions. */
    s32 rock_type;
} M644RouteVectors;

/* One rock's collision body, visible model and break animation. */
typedef struct M644RouteModelEntry {
    /* Rock slot, zero through seventeen. */
    s16 slot;
    /* Team index, zero or one. */
    s16 side;
    /* Visible rock model. */
    s16 model;
    /* Break-animation model, hidden until the rock breaks. */
    s16 motion_model;
    /* Course section containing this rock. */
    s16 route_group;
    /* Remaining rock life; negative after breaking. */
    f32 life;
    /* Rock type, zero through two. */
    s16 rock_type;
    /* Collision actor associated with this rock. */
    MGACTOR *actor;
} M644RouteModelEntry;

/* Nine rocks for each of the two teams. */
typedef struct M644RouteModelTable {
    /* Rock state indexed by team * 9 plus local rock slot. */
    M644RouteModelEntry entry[18];
} M644RouteModelTable;

#endif
