/* State records for Snow Brawl's particles, ripples, and queued effects. */
#ifndef M626_EFFECTS_H
#define M626_EFFECTS_H
#include "game/object.h"
#include "game/hu3d.h"

typedef struct M626RippleRecord_s {
    s32 active; /* Ripple animation state: 0 unused, 1 initialized, 2 growing, 3 shrinking. */
    f32 fanRadius; /* Fan radius passed by fn_1_E114 and used to draw the ripple. */
    f32 radius; /* Current ripple scale, from 0 to 1 during its fade animation. */
    union {
        u32 value; /* Packed RGBA color used by the ripple renderer. */
        s8 components[4]; /* Signed channel bytes used while updating alpha. */
        struct {
            u8 red, green, blue; /* Ripple color channels. */
            s8 alpha; /* Ripple alpha channel during its fade. */
        } channels;
    } color;
    Vec position; /* World position at the center of the ripple. */
} M626RippleRecord;

typedef struct M626RippleWork_s {
    M626RippleRecord records[640]; /* Active and available ripple slots. */
    f32 sineTable[10]; /* Unit-circle X offsets for the ten fan vertices. */
    f32 cosineTable[10]; /* Unit-circle Z offsets for the ten fan vertices. */
} M626RippleWork;

typedef struct M626EffectRecord_s {
    s32 active; /* Nonzero while this queued effect is drawn. */
    f32 radius; /* Fan radius in world units. */
    Vec *position; /* Pointer to the effect's live world position. */
} M626EffectRecord;

typedef struct M626EffectCollection_s {
    M626EffectRecord *records[30]; /* Queued effect records; NULL marks a free slot. */
    f32 sineTable[10]; /* Unit-circle X offsets for the ten fan vertices. */
    f32 cosineTable[10]; /* Unit-circle Z offsets for the ten fan vertices. */
} M626EffectCollection;

typedef struct M626SparkParticle_s {
    s32 active; /* Nonzero while this particle participates in updates and drawing. */
    s32 life; /* Frames until the particle's fall speed is adjusted; resets to 4. */
    Vec position; /* Particle world position. */
    Vec velocity; /* Per-update particle displacement. */
} M626SparkParticle;

typedef struct M626SparkEffectData_s {
    M626SparkParticle particles[512]; /* Spark particles in the field. */
    f32 angle; /* Rotation angle in degrees used by the spark renderer. */
} M626SparkEffectData;

extern OMOBJ *lbl_1_bss_6C8;
extern HUPROCESS *lbl_1_bss_6CC;
extern GXColor lbl_1_data_8C0[2];
extern Vec lbl_1_data_8C8[5];
#endif
