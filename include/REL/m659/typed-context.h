/* Camera and shared minigame state for Asteroad Rage. */
#ifndef M659_CONTEXT_H
#define M659_CONTEXT_H

typedef struct M659Camera {
    unsigned char unreadCameraBytes[8]; /* Bytes with no established camera behavior in this
                                         * module. */
    s32 cameraMask; /* Camera bit used by the graphics engine. */
    OMOBJ *target; /* Player object followed by this split-screen camera. */
    HuVecF position; /* Camera position in world units. */
    HuVecF targetPosition; /* Look-at position in world units. */
} M659Camera;

typedef struct M659Work {
    OMOBJMAN *objman; /* Object manager for the minigame. */
    OMOBJ *cameraObj[2]; /* Camera object for each side. */
    HU3D_LIGHTID light; /* Course light created during minigame startup. */
    s32 resultTimer; /* Frames elapsed in the result presentation. */
    s32 resultPhase; /* Step of the result presentation sequence. */
    s32 streamHandle; /* Background stream handle, or -1 when stopped. */
    s16 winners[2]; /* Side indices of the winner or tied winners. */
    s16 winnerCount; /* Number of winning sides. */
    s16 otherPlayer; /* Losing side shown during the winner presentation. */
} M659Work;

typedef struct M659Viewport {
    float y; /* Vertical origin in screen pixels. */
    float x; /* Horizontal origin in screen pixels. */
    float width; /* Viewport width in screen pixels. */
    float height; /* Viewport height in screen pixels. */
} M659Viewport;

extern M659Work lbl_1_bss_4;
extern M659Work *lbl_1_data_0;
extern M659Viewport lbl_1_data_C[2];
extern const HuVecF lbl_1_rodata_10;
extern const GXColor lbl_1_rodata_6C;
extern const HuVecF lbl_1_rodata_70;
extern const HuVecF lbl_1_rodata_7C;
extern const HuVecF lbl_1_rodata_88;
extern const GXColor lbl_1_rodata_9C;
extern const HuVecF lbl_1_rodata_A0;
extern const HuVecF lbl_1_rodata_AC;
void fn_1_3D0(s16 index, OMOBJ *target);
void fn_1_408(OMOBJ *obj);
void fn_1_48C(OMOBJ *obj);
void fn_1_558(OMOBJ *obj);
void fn_1_4918(HuVecF positionA, HuVecF positionB, HuVecF *difference);
double fn_1_493C(HuVecF difference);
double fn_1_4ABC(HuVecF positionA, HuVecF positionB);

int abs(int absoluteInput);
#endif
