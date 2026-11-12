#ifndef REL_M624DLL_TAIL_TYPES_H
#define REL_M624DLL_TAIL_TYPES_H

#include "dolphin/mtx.h"
extern "C" {
#include "game/mg/actman.h"
}
#include "REL/framework/linknode.h"
#include "REL/m624DLL/range.h"

struct M624CameraSetup {
    Vec eye;
    Vec up;
    Vec target;
    unsigned char unk24; /* The game's code never reads this member. */
};

struct M624Region {
    Point3d position;
    f32 threshold;
    u32 unk_10[5]; /* No code names this field. */
};

struct MT;

/* CA90/CF60/CD78 consume a Range followed by two angles and two pad-state words. */
class M624PadControl {
public:
    M624PadControl(u16, f32);
    ~M624PadControl();
    u16 type;
    u8 unknown02[2]; /* No code names this field. */
    Range range;
    f32 angle_primary;
    f32 angle_secondary;
    u32 flags_primary;
    u32 flags_secondary;
    void fn_1_CAF8(u32 flags);
    void fn_1_CB08(u32 flags);
    void fn_1_CB18();
    void fn_1_CB28();
    void fn_1_CB38();
};

void fn_1_CA90(M624PadControl *object, f32 value);
void fn_1_CB48(M624PadControl *, const Vec *, f32);
void fn_1_CD78(M624PadControl *, MGPLAYER *);
void fn_1_CF60(Range *self, f32 value);

struct M624LinkedObject : Node {
    void *unknown_08;
    void *unk0C; /* The game's code never reads this member. */
    void *resource;
};

struct HandleListObject;
void *fn_1_5E1C(HandleListObject *);
void *fn_1_5E24(void *handle);

struct M624ChildRecord {
    Node links;
    void *owner;
};

struct M624PlayerObject : Node {
    void *unknown_08;
    void *data0;
    void *data1;
    void *data2;
    s32 player_index;
    s32 model_id;
    s32 actor_param;
    s32 actor_mode;
    s32 difficulty;
    s32 update_count;
    s32 update_flags;
    M624ChildRecord child_record;
    s16 state_word;
    s16 state_aux;
    u8 state44;
    u8 state45;
    u8 state46;
    u8 state47;
    Vec position;
    Vec home_position;
    Vec velocity;
    Vec force;
    Vec rotation;
    f32 scale;
    MGPLAYER *player;
    u32 sequence;
    f32 timer;
    u32 mode;
    void *resource;
    u8 active;
    Vec target_position;
};

#endif
