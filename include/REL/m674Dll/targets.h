/* Prop creation, target behavior and the model entries used for collected objects. */
#ifndef REL_M674DLL_TARGETS_H
#define REL_M674DLL_TARGETS_H
#include "REL/m674Dll/player.h"
#include "REL/m674Dll/collision.h"

typedef struct M674PropWork M674PropWork;
typedef s32 (*M674TargetAction)(M674PropWork *work);
typedef struct M674PropMotionParam {
    s32 resource; /* Motion resource index; -1 skips a slot and -2 ends the list. */
    f32 blend; /* Blend fraction, scaled to frames when a motion shift begins. */
    u32 attr; /* Attributes passed to the 3D model-motion system. */
} M674PropMotionParam;
typedef struct M674PropParam {
    s32 count; /* Number of instances; -1 marks the end of the prop table. */
    s32 linkModel; /* Selects the shared linked-model path when nonzero. */
    s32 type; /* Prop category used when looking up an instance by type and ID. */
    s16 firstId; /* ID assigned to the first instance; later instances increment it. */
    s16 layer; /* 3D render layer for the prop model. */
    u32 attr; /* Model attributes applied when the prop model is created. */
    s32 shadow; /* Shadow mode: 0 off, 1 casting, 2 shadow-map model. */
    s32 modelResource; /* Model resource index; -1 creates no model. */
    M674PropMotionParam *motions; /* Optional motion-resource table for the prop. */
    Vec *position; /* Optional initial world position; NULL leaves the zeroed position. */
    f32 rotationY; /* Initial heading in degrees around the Y axis. */
    OMOBJ_FUNC update; /* Object callback used to update each prop instance. */
} M674PropParam;

typedef struct M674ModelEntry {
    s32 state; /* Collection-animation state; 0 means this pool slot is free. */
    f32 progress; /* Score-flight interpolation fraction, or countdown between five-item-pool
                   * emissions. */
    f32 yawStep; /* Degrees added to model yaw per update. */
    s32 timer; /* Remaining phase frames, or single-item emissions left in the five-item pool. */
    s32 modelId; /* Hidden model assigned to this collection-animation slot. */
    s32 scoreIndex; /* Player score-box index that owns the collected item. */
    f32 scale; /* Uniform model scale during the collection animation. */
    Vec position; /* Current world position of the animated model. */
    Vec rotation; /* Current model rotation in degrees. */
    Vec trajectoryVector; /* Start position for score interpolation, or velocity for the
                           * scripted-actor collection flight. */
    Vec targetPosition; /* World-space endpoint projected 30 screen pixels below the owning player's
                         * score box. */
} M674ModelEntry;

struct M674PropWork {
    M674ActorState state; /* Shared transform, motion and collision state for this prop. */
    M674PropParam *param; /* Table entry that configures this prop instance. */
    s32 id; /* Instance ID derived from the table entry's first ID. */
    s32 timer; /* Elapsed update count used by prop animation callbacks. */
    Vec startPosition; /* World position restored by prop reset and animation setup. */
    s32 phase; /* Current phase of the prop's scripted animation. */
    s32 positionTimer; /* Frame countdown until a new ambient-prop movement destination is
                        * chosen. */
    s32 actionTimer; /* Frame counter or countdown for the current prop or scenery phase. */
    u8 unreadPropBytes[4]; /* Unread bytes in the prop work record. */
    s32 actionIndex; /* Scripted actor action index, or the wobbling prop's impact-request pulse. */
    s32 unreadPropWord; /* Unread word in the prop work record. */
    f32 amplitude; /* Decaying height factor for the prop's bobbing animation. */
    f32 phaseAngle; /* Bobbing cycle angle in degrees. */
    f32 pitchPhase; /* Wobble phase angle for X rotation, in degrees. */
    f32 unreadPropFloat; /* Unread float in the prop work record. */
    f32 rollPhase; /* Wobble phase angle for Z rotation, in degrees. */
    u8 unreadPropState[140]; /* Unread bytes in the prop work record. */
    M674ModelEntry *models; /* Model-pool entries used by collection animations. */
    s32 secondaryModel; /* Replacement model animated and faded when this target is collected. */
};

typedef struct M674RegionHeader {
    u32 flags; /* Collision-entry state flags. */
    u32 mask; /* Entry mask compared with other collision entries. */
    u32 type; /* Collision-entry category used by registry lookups. */
    s32 id; /* Entry ID within its category. */
    s32 state; /* Collision-entry state checked by pair processing. */
    void *primaryRelation; /* Object associated with this entry. */
    void *secondaryRelation; /* Additional object or region associated with this entry. */
    OMOBJ_FUNC invoke; /* Callback slot shared with primary entries; secondary-region processing
                        * does not invoke it. */
} M674RegionHeader;
typedef struct M674AnnularRegionRecord {
    M674RegionHeader header; /* Collision registry fields shared by every region. */
    M674RegionShape shape; /* Center, vertical span, and inner/outer ellipse radii. */
} M674AnnularRegionRecord;
typedef struct M674EllipseRecord {
    M674RegionHeader header; /* Collision registry fields shared by every region. */
    M674EllipseShape shape; /* Center, radii and the side of the ellipse to constrain. */
} M674EllipseRecord;
typedef union M674RegionRecord {
    M674Entry entry; /* Common collision-entry view used by the registry. */
    M674AnnularRegionRecord annulus; /* Annular play-region view. */
    M674EllipseRecord ellipse; /* Elliptical boundary view. */
} M674RegionRecord;

extern OMOBJ *lbl_1_bss_310[26], *lbl_1_bss_4BC[128];
extern u32 lbl_1_bss_378[26];
extern M674RegionRecord lbl_1_bss_3E0[3];
extern s32 lbl_1_bss_4AC, lbl_1_bss_4B0;
extern OMOBJ *lbl_1_bss_4B4, *lbl_1_bss_4B8, *lbl_1_bss_6BC;
extern const M674AnnularRegionRecord lbl_1_rodata_2B8;
extern const M674EllipseRecord lbl_1_rodata_2FC, lbl_1_rodata_334;
extern Vec lbl_1_data_270[4];
extern Vec lbl_1_data_2A0, lbl_1_data_2AC, lbl_1_data_2B8;
extern M674PropMotionParam lbl_1_data_2C4[9];
extern M674PropParam lbl_1_data_330[14];
extern Vec lbl_1_data_5A4[4], lbl_1_data_5D4, lbl_1_data_60C;
extern M674TargetAction lbl_1_data_634[4];
extern Vec lbl_1_data_644, lbl_1_data_650, lbl_1_data_65C, lbl_1_data_668;
extern f32 lbl_1_data_674;

void fn_1_67B0(M674ActorState *state);
void fn_1_67CC(M674ActorState *state, s32 motion, f32 speed);
void fn_1_8894(M674ActorState *state);
void fn_1_88C8(M674ActorState *state, s32 model);
#endif
