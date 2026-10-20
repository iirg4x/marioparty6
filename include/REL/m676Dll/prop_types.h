/* Model settings, animation and movement state shared by the scene props. */
#ifndef REL_M676DLL_PROP_TYPES_H
#define REL_M676DLL_PROP_TYPES_H

#include "REL/m676Dll/actor_types.h"

typedef struct M676PropMotionParam {
    s32 resource;
    f32 blend;
    u32 attr;
} M676PropMotionParam;

typedef struct M676PropParam {
    s32 count;
    s32 linkModel;
    s32 type;
    s16 firstId;
    s16 layer;
    u32 attr;
    /* Selects shadow handling: 1 enables model shadow casting, while 2 enables the model's
     * shadow-map mode. */
    s32 shadow;
    s32 modelResource;
    M676PropMotionParam *motions;
    Vec *position;
    /* Initial Y rotation in degrees. */
    f32 rotationY;
    OMOBJ_FUNC update;
} M676PropParam;

typedef struct M676PropState {
    M676ActorState actor;
    s32 controlState;
    s32 controlTimer;
    /* Cleared with the prop record and never read by this module. */
    u8 unkB4, unkB5, unkB6, unkB7;
    /* Desired horizontal heading in degrees and its world-space destination. */
    f32 targetAngle;
    Vec targetPosition;
    s32 sequenceState;
    s32 sequenceTimer;
    s32 unknown_D0;
    s32 movementState;
    s32 movementTimer;
    /* Cleared with the prop record and never read by this module. */
    u8 unkDC, unkDD, unkDE, unkDF, unkE0, unkE1;
    u8 unkE2, unkE3, unkE4, unkE5, unkE6, unkE7;
    u8 unkE8, unkE9, unkEA, unkEB, unkEC, unkED;
    u8 unkEE, unkEF, unkF0, unkF1, unkF2, unkF3;
    u8 unkF4, unkF5, unkF6, unkF7, unkF8, unkF9;
    u8 unkFA, unkFB, unkFC, unkFD, unkFE, unkFF;
    u8 unk100, unk101, unk102, unk103, unk104, unk105;
    u8 unk106, unk107, unk108, unk109, unk10A, unk10B;
    u8 unk10C, unk10D, unk10E, unk10F;
    s32 motionActive;
    s32 motionStatus;
    s32 unknown_118;
    s32 jumpActive;
    /* Cleared with the prop record and never read by this module. */
    u8 unk120, unk121, unk122, unk123, unk124, unk125;
    u8 unk126, unk127, unk128, unk129, unk12A, unk12B;
    u8 unk12C, unk12D, unk12E, unk12F, unk130, unk131;
    u8 unk132, unk133, unk134, unk135, unk136, unk137;
    u8 unk138, unk139, unk13A, unk13B, unk13C, unk13D;
    u8 unk13E, unk13F, unk140, unk141, unk142, unk143;
    u8 unk144, unk145, unk146, unk147, unk148, unk149;
    u8 unk14A, unk14B;
    /* Accumulated requested movement for footstep sounds; reduced by 100 after each sound. */
    f32 footstepDistance;
    s32 poseEnabled;
    f32 poseHeight;
    Vec *savedPositionPointer;
    Vec savedPosition;
    /* Cleared with the prop record and never read by this module. */
    u8 unk168, unk169, unk16A, unk16B, unk16C, unk16D;
    u8 unk16E, unk16F, unk170, unk171, unk172, unk173;
    u8 unk174, unk175, unk176, unk177, unk178, unk179;
    u8 unk17A, unk17B, unk17C, unk17D, unk17E, unk17F;
    u8 unk180, unk181, unk182, unk183, unk184, unk185;
    u8 unk186, unk187, unk188, unk189, unk18A, unk18B;
    u8 unk18C, unk18D, unk18E, unk18F, unk190, unk191;
    u8 unk192, unk193, unk194, unk195, unk196, unk197;
    u8 unk198, unk199, unk19A, unk19B, unk19C, unk19D;
    u8 unk19E, unk19F, unk1A0, unk1A1, unk1A2, unk1A3;
} M676PropState;

typedef struct M676PropWork {
    M676PropState state;
    M676PropParam *param;
    s32 id;
    s32 timer;
    Vec startPosition;
    s32 phase;
    s32 targetChangeTimer;
    s32 frame;
    s32 unknown_1C8;
    s32 actionIndex;
    s32 pathIndex;
    /* Scenery uses wobble strength and angular phases. Lane props reuse wobbleAmount as
     * target yaw, bobPhase as a rolling sign, and pitchPhase/rollPhase as X/Z movement
     * directions. */
    f32 wobbleAmount;
    f32 bobPhase;
    f32 pitchPhase;
    f32 unknown_1E0;
    f32 rollPhase;
    /* Cleared with the prop record and never read by this module. */
    u8 unk1E8, unk1E9, unk1EA, unk1EB, unk1EC, unk1ED;
    u8 unk1EE, unk1EF, unk1F0, unk1F1, unk1F2, unk1F3;
    u8 unk1F4, unk1F5, unk1F6, unk1F7, unk1F8, unk1F9;
    u8 unk1FA, unk1FB, unk1FC, unk1FD, unk1FE, unk1FF;
    u8 unk200, unk201, unk202, unk203, unk204, unk205;
    u8 unk206, unk207, unk208, unk209, unk20A, unk20B;
    u8 unk20C, unk20D, unk20E, unk20F, unk210, unk211;
    u8 unk212, unk213, unk214, unk215, unk216, unk217;
    u8 unk218, unk219, unk21A, unk21B, unk21C, unk21D;
    u8 unk21E, unk21F, unk220, unk221, unk222, unk223;
    u8 unk224, unk225, unk226, unk227, unk228, unk229;
    u8 unk22A, unk22B, unk22C, unk22D, unk22E, unk22F;
    u8 unk230, unk231, unk232, unk233, unk234, unk235;
    u8 unk236, unk237, unk238, unk239, unk23A, unk23B;
    u8 unk23C, unk23D, unk23E, unk23F, unk240, unk241;
    u8 unk242, unk243, unk244, unk245, unk246, unk247;
    u8 unk248, unk249, unk24A, unk24B, unk24C, unk24D;
    u8 unk24E, unk24F, unk250, unk251, unk252, unk253;
    u8 unk254, unk255, unk256, unk257, unk258, unk259;
    u8 unk25A, unk25B, unk25C, unk25D, unk25E, unk25F;
    u8 unk260, unk261, unk262, unk263, unk264, unk265;
    u8 unk266, unk267, unk268, unk269, unk26A, unk26B;
    u8 unk26C, unk26D, unk26E, unk26F, unk270, unk271;
    u8 unk272, unk273;
    void *records;
    s32 soundHandle;
    s32 modelId;
    s32 movementState;
    s32 contactState;
    s32 settleTimer;
    s32 finished;
    s32 selectionState;
    s32 initialized;
    s32 outOfRange;
    f32 targetX;
    f32 targetZ;
} M676PropWork;

extern OMOBJ *lbl_1_bss_EA0[128];

#endif
