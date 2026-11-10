/* Circuit Maximus: object setup, player and obstacle updates, and split-screen cameras. */
#include "dolphin/gx.h"

#include "dolphin/mtx.h"

#include "string.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "game/object.h"

#include "game/hu3d.h"

#include "game/mg/actman.h"

#include "game/mg/seqman.h"

#include "game/pad.h"

#include "game/printfunc.h"

#include "game/animdata.h"

#include "game/charman.h"

#include "game/gamework.h"

#include "dolphin.h"

#include "humath.h"

#include "datadir_enum.h"

#include "messdir_enum.h"

#include "game/audio.h"

#include "game/sprite.h"

#include "game/memory.h"

#include "game/data.h"

#include "game/main.h"

#include "game/window.h"

#include "game/wipe.h"

#include "game/frand.h"

#include "game/process.h"

#include "game/hsfex.h"

#include "game/gamemes.h"

#include "game/flag.h"

#include "game/armem.h"

#include "game/mgdata.h"

#include "game/saveload.h"

#include "game/esprite.h"

#include "game/thpmain.h"
#include "msm_se.h"
typedef struct MgTimer_s {
    s32 mode;
    s32 prevMode;
    s32 type;
    s32 offType;
    BOOL dispOnF;
    BOOL stopF;
    s32 scaleDir;
    BOOL boxDispF;
    u8 flashFlag;
    s32 maxTime;
    s32 endTime;
    s32 speed;
    s32 time;
    s32 recordTime;
    s32 fadeOutTime;
    HuVec2f pos;
    s16 timeUnit;
    GAMEMESID gameMesId;
    HUSPR_GROUPID boxId;
    HUSPRID espId[15];
    float digitScale;
    HUPROCESS *proc;
    u8 digitR;
    u8 digitG;
    u8 digitB;
} MGTIMER;

MGTIMER* MgTimerCreate(int type);
void MgTimerKill(MGTIMER* timer);
int MgTimerModeGet(MGTIMER* timer);
void MgTimerParamSet(MGTIMER* timer, int maxTime, int endTime, int recordTime);
int MgTimerValueGet(MGTIMER* timer);
void MgTimerPosSet(MGTIMER* timer, float posX, float posY);
void MgTimerPosGet(MGTIMER* timer, float *posX, float *posY);
void MgTimerModeOnSet(MGTIMER* timer, int offType);
void MgTimerModeOffSet(MGTIMER* timer);
BOOL MgTimerDoneCheck(MGTIMER* timer);
void MgTimerRecordSet(MGTIMER* timer, int recordTime);
void MgTimerColorSet(MGTIMER* timer, u8 r, u8 g, u8 b);
void MgTimerBackColorSet(MGTIMER* timer, u8 r, u8 g, u8 b);
void MgTimerDispOn(MGTIMER* timer);
void MgTimerRecordDispOn(MGTIMER* timer);
void MgTimerRecordDispOff(MGTIMER* timer);

#include "game/mg/score.h"

#include "game/board/object.h"

#include "game/board/main.h"

#include "game/board/audio.h"

#include "game/board/tutorial.h"

#include "game/board/masu.h"

#include "game/board/player.h"
#include <math.h>

/* The module keeps the actor payload in the final object word. */
typedef struct M606ActorObject {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    OMOBJ_FUNC objFunc;
    Vec trans;
    Vec rot;
    Vec scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    union {
        void *data;
        s32 modelId;
    };
} M606ActorObject;





/* Returns the length of a two-dimensional vector. */




































float fn_1_0(float x, float y)
{
    return sqrtf(x * x + y * y);
}

/* Scales a two-dimensional vector down when its length exceeds maximum. */
void fn_1_134(float *x, float *y, float maximum)
{
    float length = fn_1_0(*x, *y);
    if (length > maximum) {
        *x = maximum * (*x / length);
        *y = maximum * (*y / length);
    }
}

void fn_1_2DC(int *x, int *y, float limit)
{
    float length;
    length = fn_1_0((float)*x, (float)*y);
    if (length > limit) {
        *x = (int)(limit * ((float)*x / length));
        *y = (int)(limit * ((float)*y / length));
    }
}


typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void ObjectSetup(void);

/* Runs the module constructors before setting up its stage cameras and light. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

/* Runs the module destructors when the stage overlay exits. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

typedef struct M606MotionEntry {
    s32 kind;
    s32 flags;
    s32 motionMode;
    float startX;
    float startZ;
    float previousX;
    float previousZ;
    float directionX;
    float directionZ;
    float targetX;
    float targetZ;
    float deltaX;
    float deltaZ;
    float speed;
    float angularSpeed;
    s32 collisionKind;
    float collisionNormalX;
    float collisionNormalZ;
    s32 randomOffset;
    s32 motionState;
    s32 parameter;
    s32 overlayEnabled;
    s32 overlayModelIndex;
    float scale;
} M606MotionEntry;

typedef struct M606MotionPattern {
    float kind;
    float parameter;
    float startX;
    float startZ;
    float targetX;
    float targetZ;
} M606MotionPattern;






extern s32 lbl_1_data_0[4];





extern M606MotionEntry lbl_1_bss_A168[4][19];

extern OMOBJ *lbl_1_bss_A0FC;








extern char lbl_1_data_96[];

extern char lbl_1_data_9F[];

const s32 lbl_1_rodata_30[4] = { 1, 2, 4, 8 };

const M606MotionPattern lbl_1_rodata_40[8][12] = { { { 2.0f, 0.0f, -200.0f, -1225.0f, -200.0f, -580.0f }, { 2.0f, 0.0f, 200.0f, -1225.0f, 200.0f, -580.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, { { 1.0f, 0.0f, -978.0f, -542.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, -705.0f, -650.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, -670.0f, -410.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, -640.0f, -900.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, -425.0f, -665.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, { { 2.0f, 0.0f, -1240.0f, -142.0f, -590.0f, -142.0f }, { 2.0f, 0.0f, -1240.0f, 47.0f, -590.0f, 47.0f }, { 2.0f, 0.0f, -1240.0f, 220.0f, -580.0f, 220.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, { { 2.0f, 0.0f, -970.0f, 770.0f, -510.0f, 335.0f }, { 2.0f, 0.0f, -850.0f, 902.0f, -400.0f, 450.0f }, { 2.0f, 0.0f, -705.0f, 1032.0f, -270.0f, 595.0f }, { 1.0f, 0.0f, -945.0f, 525.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, -560.0f, 373.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, -340.0f, 640.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, { { 3.0f, 0.0f, 215.0f, 1085.0f, 0.0f, 0.0f }, { 3.0f, 0.0f, -100.0f, 725.0f, 0.0f, 0.0f }, { 3.0f, 0.0f, 50.0f, 940.0f, 0.0f, 0.0f }, { 3.0f, 0.0f, 260.0f, 1060.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, { { 2.0f, 0.0f, 380.0f, 460.0f, 660.0f, 1070.0f }, { 2.0f, 0.0f, 380.0f, 460.0f, 660.0f, 1070.0f }, { 2.0f, 0.0f, 455.0f, 415.0f, 865.0f, 855.0f }, { 2.0f, 0.0f, 455.0f, 415.0f, 865.0f, 855.0f }, { 2.0f, 0.0f, 520.0f, 370.0f, 1115.0f, 615.0f }, { 2.0f, 0.0f, 520.0f, 370.0f, 1115.0f, 615.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, { { 2.0f, 0.0f, 600.0f, -210.0f, 1210.0f, -210.0f }, { 2.0f, 0.0f, 665.0f, -90.0f, 1105.0f, -90.0f }, { 2.0f, 0.0f, 600.0f, 40.0f, 1210.0f, 40.0f }, { 2.0f, 0.0f, 603.0f, 68.0f, 1210.0f, 253.0f }, { 1.0f, 0.0f, 657.0f, -90.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 1110.0f, -90.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 750.0f, 227.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 1050.0f, 325.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } }, { { 3.0f, 0.0f, 425.0f, -785.0f, 0.0f, 0.0f }, { 3.0f, 0.0f, 678.0f, -520.0f, 0.0f, 0.0f }, { 3.0f, 0.0f, 845.0f, -450.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 545.0f, -890.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 525.0f, -510.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 865.0f, -592.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f } } };

void fn_1_5CC(void)
{
    s16 cameraIndex;
    float eyeZ;
    float eyeY;
    float eyeX;
    float centerZ;
    float centerY;
    float centerX;
    float directionZ;
    float directionY;
    float directionX;
    float roll;
    float rotX;
    float rotY;

    for (cameraIndex = 0; cameraIndex < 4; cameraIndex++) {
        lbl_80266BD8[cameraIndex] += 25.0 *
            (double)(((lbl_802C0010[0] & PAD_BUTTON_X) != 0) -
                     ((lbl_802C0010[0] & PAD_BUTTON_Y) != 0));
        lbl_80266A58[cameraIndex].y += (double)lbl_802BFFF4[0] / 32.0;
        lbl_80266A58[cameraIndex].x += (double)lbl_802BFFF0[0] / -32.0;
        if (cameraIndex == -1) {
            fontcolor = 0xF;
            print8(0x40, 0x188, 2.0f, lbl_1_data_96, lbl_80266B18[cameraIndex].x,
                   lbl_80266B18[cameraIndex].y, lbl_80266B18[cameraIndex].z);
            print8(0x40, 0x198, 2.0f, lbl_1_data_96, lbl_80266A58[cameraIndex].x,
                   lbl_80266A58[cameraIndex].y, lbl_80266A58[cameraIndex].z);
            print8(0x40, 0x1A8, 2.0f, lbl_1_data_9F, lbl_80266BD8[cameraIndex]);
        }
        rotX = lbl_80266A58[cameraIndex].x;
        rotY = lbl_80266A58[cameraIndex].y;
        roll = lbl_80266A58[cameraIndex].z;
        eyeX = (double)lbl_80266B18[cameraIndex].x +
            ((double)lbl_80266BD8[cameraIndex] * (sin((M_PI * (double)rotY) / 180.0) *
             cos((M_PI * (double)rotX) / 180.0)));
        eyeY = (double)lbl_80266B18[cameraIndex].y +
            ((double)lbl_80266BD8[cameraIndex] * -sin((M_PI * (double)rotX) / 180.0));
        eyeZ = (double)lbl_80266B18[cameraIndex].z +
            ((double)lbl_80266BD8[cameraIndex] * (cos((M_PI * (double)rotY) / 180.0) *
             cos((M_PI * (double)rotX) / 180.0)));
        centerX = lbl_80266B18[cameraIndex].x;
        centerY = lbl_80266B18[cameraIndex].y;
        centerZ = lbl_80266B18[cameraIndex].z;
        directionX = (float)(sin((M_PI * (double)rotY) / 180.0) *
            sin((M_PI * (double)rotX) / 180.0));
        directionY = (float)cos((M_PI * (double)rotX) / 180.0);
        directionZ = (float)(cos((M_PI * (double)rotY) / 180.0) *
            sin((M_PI * (double)rotX) / 180.0));
    }
}


extern s32 lbl_1_bss_24;

void fn_1_BD0(HU3D_MODEL *context, Mtx *matrix)
{
    Mtx identity;
    Mtx projection;
    u32 white = 0;
    if (lbl_1_bss_24 == 0)
    {
        return;
    }
    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,
                      GX_PTIDENTITY);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_RASA);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(0, GX_ALWAYS, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXSetZCompLoc(0);
    GXSetScissor(0, 0, 0x280, 0x1E0);
    GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    C_MTXOrtho(projection, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, -1.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);
    PSMTXIdentity(identity);
    GXLoadPosMtxImm(identity, GX_PNMTX0);
    white = 0xFF;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(0.0f, 0.0f, 0.0f);
    GXColor4u8((u8) white, (u8) white, (u8) white, (u8) lbl_1_bss_24);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(1.0f, 0.0f, 0.0f);
    GXColor4u8((u8) white, (u8) white, (u8) white, (u8) lbl_1_bss_24);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(1.0f, 1.0f, 0.0f);
    GXColor4u8((u8) white, (u8) white, (u8) white, (u8) lbl_1_bss_24);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(0.0f, 1.0f, 0.0f);
    GXColor4u8((u8) white, (u8) white, (u8) white, (u8) lbl_1_bss_24);
    GXTexCoord2f32(0.0f, 1.0f);
}


typedef struct M606PlayerWork {
    s32 playerIndex;
    s32 configIndex;
    s32 modelCharacter;
    s32 character;
    s32 padIndex;
    s32 computerDifficulty;
    s32 lightId;
    s32 secondLightId;
    u8 unk_20[4]; /* No code names this field. */
    s32 state;
    u8 unk_28[0x14]; /* No code names this field. */
    s32 sequenceFrame;
    s32 sequenceCount;
    s32 childIndex;
    u8 unk_48[0x10]; /* No code names this field. */
    float playerPositionX;
    float playerPositionZ;
    u8 unk_60[0xC]; /* No code names this field. */
    s32 finished;
    s32 randomValue;
    s32 currentMode;
    u8 unk_78[8]; /* No code names this field. */
    s32 framesSinceCollision;
    s32 previousStickX;
    s32 earlierStickX;
    s32 previousStickY;
    s32 earlierStickY;
    s32 stickOverrideFrames;
    s32 sameStickFrames;
} M606PlayerWork;

typedef struct M606SpriteEntry {
    u8 unknown00[2]; /* No code names this field. */
    s16 animationIndex;
} M606SpriteEntry;

typedef struct M606SpriteAnimationEntry {
    u8 unknown00[8]; /* No code names this field. */
    ANIMDATA *animation;
} M606SpriteAnimationEntry;

extern M606SpriteEntry esprite[];

extern M606SpriteAnimationEntry espanim[];

typedef struct M606SpriteState {
    s32 entry;
    u8 unknown04[0x10]; /* No code names this field. */
} M606SpriteState;

typedef struct M606TimerSlot {
    MGTIMER *timer;
    u8 unknown04[8]; /* No code names this field. */
} M606TimerSlot;

typedef struct M606AngleState {
    f32 value;
    u8 unknown04[4]; /* No code names this field. */
} M606AngleState;

extern M606SpriteState lbl_1_bss_A0C8;

extern s32 lbl_1_bss_18;

extern s32 lbl_1_bss_20;

extern s32 lbl_1_bss_2C;

extern s32 lbl_1_data_34[2];

typedef union M606ActorObjectRef {
    OMOBJ *object;
    M606ActorObject *actor;
} M606ActorObjectRef;

extern M606ActorObjectRef lbl_1_bss_A100[4];


extern void HuSprTexLoad(ANIMDATA *animation, s16 bitmapNo, s16 textureMap, s32 wrapS, s32 wrapT, s32 filter);

/* Draws the stage's player and motion overlays with the supplied model matrix. */
static inline void DebugCheckOverlayModels(HU3D_MODEL *cameraModel, HU3D_MODEL *overlay, s32 enabled)
{
    s32 playerIndex;
    s32 motionIndex;
    s32 modelIndex;

    s32 visibleCount;
    s32 hiddenCount;
    f32 playerX;
    f32 playerY;
    f32 playerZ;

    f32 overlayX;
    f32 overlayZ;

    f32 motionX;
    f32 motionZ;

    if (enabled) {
        playerX = lbl_1_bss_A100[0].object->trans.x;
        playerY = lbl_1_bss_A100[0].object->trans.y;
        playerZ = lbl_1_bss_A100[0].object->trans.z;
        overlayX = lbl_802C01C8[lbl_1_bss_A0FC->mdlId[0x7E]].pos.x;
        overlayZ = lbl_802C01C8[lbl_1_bss_A0FC->mdlId[0x7E]].pos.z;
        motionX = lbl_1_bss_A168[0][0].startX;
        motionZ = lbl_1_bss_A168[0][0].startZ;
        visibleCount = 0;
        hiddenCount = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (((1 << playerIndex) & cameraModel->cameraBit) != 0) {
                if ((lbl_802C01C8[*lbl_1_bss_A100[playerIndex].object->mdlId].attr & 1) != 0) {
                    hiddenCount++;
                } else {
                    visibleCount++;
                }
            }
            for (motionIndex = 0; motionIndex < 19; motionIndex++) {
                if (lbl_1_bss_A168[playerIndex][motionIndex].kind != 0 &&
                    lbl_1_bss_A168[playerIndex][motionIndex].overlayModelIndex == -1) {
                    OSReport("overlay missing: player %d\n", playerIndex);
                }
            }
        }
        for (modelIndex = 0; modelIndex < 2; modelIndex++) {
            if (lbl_1_bss_A0FC->mdlId[modelIndex + 0x7E] >= 0 &&
                lbl_802C01C8[lbl_1_bss_A0FC->mdlId[modelIndex + 0x7E]].cameraBit != overlay->cameraBit) {
                OSReport("overlay camera mask differs\n");
            }
        }
        if (visibleCount == 0 || hiddenCount == 0) {
            return;
        }
        if (visibleCount + hiddenCount != 4) {
            return;
        }
        OSReport("overlay models %d visible %d hidden; player %f %f %f overlay %f %f motion %f %f\n",
                 visibleCount, hiddenCount, playerX, playerY, playerZ,
                 overlayX, overlayZ, motionX, motionZ);
    }
}

void fn_1_112C(HU3D_MODEL *model, Mtx *matrix)
{
    s32 playerIndex;
    s32 overlayAlpha;
    s32 motionIndex;
    s32 modelX;
    s32 modelY;
    s32 modelZ;
    HU3D_MODEL *overlayModel;
    s32 sequenceMode;
    s32 drawMode;
    f32 radius;

    overlayAlpha = 0xD0;
    sequenceMode = fn_80071A8C();
    GXLoadPosMtxImm(*matrix, GX_PNMTX0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, 0,
                      GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_RASA, GX_CA_ZERO, GX_CA_TEXA, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 1, GX_TEVPREV);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP, GX_AF_NONE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZMode(1, GX_LEQUAL, 0);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    HuSprTexLoad((espanim[esprite[lbl_1_bss_A0C8.entry].animationIndex].animation), 0, 0, 0, 0, 1);
    if (lbl_1_bss_18 == 0) {
        if ((model->cameraBit & 1) != 0) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                for (motionIndex = 0; motionIndex < 19; motionIndex++) {
                    HU3D_MODEL *firstOverlayModel;
                    firstOverlayModel = &lbl_802C01C8[lbl_1_bss_A0FC->mdlId[(playerIndex * 0x13) + 0x7E + motionIndex]];
                    if (lbl_1_bss_A168[playerIndex][motionIndex].kind != 0) {
                        modelX = (s32)firstOverlayModel->pos.x;
                        modelY = 2;
                        modelZ = (s32)firstOverlayModel->pos.z;
                        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                        GXPosition3f32((float)(modelX - 0x32), (f32)modelY, (float)(modelZ - 0x32));
                        GXColor4u8(0, 0, 0, overlayAlpha);
                        GXTexCoord2f32(0.0f, 0.0f);
                        GXPosition3f32((float)(modelX + 0x32), (f32)modelY, (float)(modelZ - 0x32));
                        GXColor4u8(0, 0, 0, overlayAlpha);
                        GXTexCoord2f32(1.0f, 0.0f);
                        GXPosition3f32((float)(modelX + 0x32), (f32)modelY, (float)(modelZ + 0x32));
                        GXColor4u8(0, 0, 0, overlayAlpha);
                        GXTexCoord2f32(1.0f, 1.0f);
                        GXPosition3f32((float)(modelX - 0x32), (f32)modelY, (float)(modelZ + 0x32));
                        GXColor4u8(0, 0, 0, overlayAlpha);
                        GXTexCoord2f32(0.0f, 1.0f);
                    }
                }
            }
        playerIndex = 0;
        modelX = (s32)lbl_1_bss_A100[playerIndex].object->trans.x;
        modelY = (s32)(2.5f + lbl_1_bss_A100[playerIndex].object->trans.y);
        modelZ = (s32)lbl_1_bss_A100[playerIndex].object->trans.z;
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32((float)(modelX - 0x46), (float)modelY, (float)(modelZ - 0x46));
        GXColor4u8(0, 0, 0, overlayAlpha);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition3f32((float)(modelX + 0x46), (float)modelY, (float)(modelZ - 0x46));
        GXColor4u8(0, 0, 0, overlayAlpha);
        GXTexCoord2f32(1.0f, 0.0f);
        GXPosition3f32((float)(modelX + 0x46), (float)modelY, (float)(modelZ + 0x46));
        GXColor4u8(0, 0, 0, overlayAlpha);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition3f32((float)(modelX - 0x46), (float)modelY, (float)(modelZ + 0x46));
        GXColor4u8(0, 0, 0, overlayAlpha);
        GXTexCoord2f32(0.0f, 1.0f);
        }
    } else if (lbl_1_bss_2C != 0 && lbl_1_data_34[0] != 4) {
        if ((model->cameraBit & 1) != 0) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                modelX = (s32)lbl_1_bss_A100[playerIndex].object->trans.x;
                modelY = (s32)(2.5f + lbl_1_bss_A100[playerIndex].object->trans.y);
                modelZ = (s32)lbl_1_bss_A100[playerIndex].object->trans.z;
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32((float)(modelX - 0x46), (float)modelY, (float)(modelZ - 0x46));
                GXColor4u8(0, 0, 0, overlayAlpha);
                GXTexCoord2f32(0.0f, 0.0f);
                GXPosition3f32((float)(modelX + 0x46), (float)modelY, (float)(modelZ - 0x46));
                GXColor4u8(0, 0, 0, overlayAlpha);
                GXTexCoord2f32(1.0f, 0.0f);
                GXPosition3f32((float)(modelX + 0x46), (float)modelY, (float)(modelZ + 0x46));
                GXColor4u8(0, 0, 0, overlayAlpha);
                GXTexCoord2f32(1.0f, 1.0f);
                GXPosition3f32((float)(modelX - 0x46), (float)modelY, (float)(modelZ + 0x46));
                GXColor4u8(0, 0, 0, overlayAlpha);
                GXTexCoord2f32(0.0f, 1.0f);
            }
            for (playerIndex = 0; playerIndex < 2; playerIndex++) {
                HU3D_MODEL *overlayModel;
                overlayModel = &lbl_802C01C8[lbl_1_bss_A0FC->mdlId[playerIndex + 0x7E]];
                modelX = (s32)overlayModel->pos.x;
                modelY = 2;
                modelZ = (s32)overlayModel->pos.z;
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32((float)(modelX - 0x32), (f32)modelY, (float)(modelZ - 0x32));
                GXColor4u8(0, 0, 0, overlayAlpha);
                GXTexCoord2f32(0.0f, 0.0f);
                GXPosition3f32((float)(modelX + 0x32), (f32)modelY, (float)(modelZ - 0x32));
                GXColor4u8(0, 0, 0, overlayAlpha);
                GXTexCoord2f32(1.0f, 0.0f);
                GXPosition3f32((float)(modelX + 0x32), (f32)modelY, (float)(modelZ + 0x32));
                GXColor4u8(0, 0, 0, overlayAlpha);
                GXTexCoord2f32(1.0f, 1.0f);
                GXPosition3f32((float)(modelX - 0x32), (f32)modelY, (float)(modelZ + 0x32));
                GXColor4u8(0, 0, 0, overlayAlpha);
                GXTexCoord2f32(0.0f, 1.0f);
            }
        }
    } else {
        HU3D_MODEL *playerModel;

        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            playerModel = &lbl_802C01C8[*lbl_1_bss_A100[playerIndex].object->mdlId];
            if (((1 << playerIndex) & model->cameraBit) != 0) {
                for (drawMode = 0; drawMode <= lbl_1_bss_20; drawMode++) {
                    M606PlayerWork *actorData = lbl_1_bss_A100[playerIndex].object->data;
                    s32 frameValues[9] = { 0, 0x2D, 0x5A, 0x87, 0xB4, 0xE1, 0xFF, 0xC8, 0x64 };
                    radius = (f32)(drawMode == 0 ? 0x46 : 0xE6);
                    if (drawMode == 1) {
                        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ONE);
                        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_RASA, GX_CA_ZERO, GX_CA_TEXA, GX_CA_ZERO);
                        GXSetBlendMode(GX_BM_BLEND, GX_BL_DSTCLR, GX_BL_SRCALPHA, GX_LO_NOOP);
                    }
                    if ((lbl_1_bss_20 == 0 ||
                         drawMode == ((actorData->sequenceFrame >= 0 && actorData->sequenceFrame < 0x5A))) &&
                        (playerModel->attr & (1 << 0)) == 0) {
                        if (drawMode == 1) {
                            overlayModel = &lbl_802C01C8[lbl_1_bss_A0FC->mdlId[playerIndex + 0x2C]];
                        }
                        modelX = (s32)lbl_1_bss_A100[playerIndex].object->trans.x;
                        modelY = (s32)(2.5f + lbl_1_bss_A100[playerIndex].object->trans.y);
                        modelZ = (s32)lbl_1_bss_A100[playerIndex].object->trans.z;
                        overlayAlpha = drawMode == 0 ? 0xD0 : frameValues[(s32)overlayModel->motWork.time % 9];
                        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                        GXPosition3f32((f32)modelX - radius, (f32)modelY,
                                       (f32)modelZ - radius);
                        GXColor4u8(0, 0, 0, (u8)overlayAlpha);
                        GXTexCoord2f32(0.0f, 0.0f);
                        GXPosition3f32((f32)modelX + radius, (f32)modelY,
                                       (f32)modelZ - radius);
                        GXColor4u8(0, 0, 0, (u8)overlayAlpha);
                        GXTexCoord2f32(1.0f, 0.0f);
                        GXPosition3f32((f32)modelX + radius, (f32)modelY,
                                       (f32)modelZ + radius);
                        GXColor4u8(0, 0, 0, (u8)overlayAlpha);
                        GXTexCoord2f32(1.0f, 1.0f);
                        GXPosition3f32((f32)modelX - radius, (f32)modelY,
                                       (f32)modelZ + radius);
                        GXColor4u8(0, 0, 0, (u8)overlayAlpha);
                        GXTexCoord2f32(0.0f, 1.0f);
                    }
                    radius = (f32)(drawMode == 0 ? 0x32 : 0xD2);
                    for (motionIndex = 0; motionIndex < 19; motionIndex++) {
                        if (lbl_1_bss_A168[playerIndex][motionIndex].kind != 0) {
                            if (lbl_1_bss_20 == 0 ||
                                drawMode == ((((lbl_1_bss_A168[playerIndex][motionIndex].motionState == 2 &&
                                             lbl_1_bss_A168[playerIndex][motionIndex].overlayEnabled != 0)) && lbl_1_bss_A168[playerIndex][motionIndex].overlayModelIndex != -1))) {
                                if (drawMode == 1) {
                                    overlayModel = &lbl_802C01C8[lbl_1_bss_A0FC->mdlId[(lbl_1_bss_A168[playerIndex][motionIndex].overlayModelIndex * 4) + 0x30 + playerIndex]];
                                }
                                modelX = (s32)lbl_1_bss_A168[playerIndex][motionIndex].startX;
                                modelY = 2;
                                modelZ = (s32)lbl_1_bss_A168[playerIndex][motionIndex].startZ;
                                overlayAlpha = drawMode == 0 ? 0xD0 : frameValues[(s32)overlayModel->motWork.time % 9];
                                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                                GXPosition3f32((f32)modelX - radius, (f32)modelY,
                                               (f32)modelZ - radius);
                                GXColor4u8(0, 0, 0, (u8)overlayAlpha);
                                GXTexCoord2f32(0.0f, 0.0f);
                                GXPosition3f32((f32)modelX + radius, (f32)modelY,
                                               (f32)modelZ - radius);
                                GXColor4u8(0, 0, 0, (u8)overlayAlpha);
                                GXTexCoord2f32(1.0f, 0.0f);
                                GXPosition3f32((f32)modelX + radius, (f32)modelY,
                                               (f32)modelZ + radius);
                                GXColor4u8(0, 0, 0, (u8)overlayAlpha);
                                GXTexCoord2f32(1.0f, 1.0f);
                                GXPosition3f32((f32)modelX - radius, (f32)modelY,
                                               (f32)modelZ + radius);
                                GXColor4u8(0, 0, 0, (u8)overlayAlpha);
                                GXTexCoord2f32(0.0f, 1.0f);
                            }
                        }
                    }
                }
            }
        }
    }
    DebugCheckOverlayModels(model, overlayModel, 0);
}


extern HuVecF lbl_1_data_A4;

extern HuVecF lbl_1_data_B0;

extern HuVecF lbl_1_data_BC;

extern GXColor colorC8;

extern GXColor lbl_1_data_CC;

int fn_1_3144(s32 modelId)
{
    int lightId;

    lightId = fn_800288B4((s16)modelId, &lbl_1_data_A4, (lbl_1_bss_20 != 0 ? &lbl_1_data_BC : &lbl_1_data_B0), (lbl_1_bss_20 != 0 ? &lbl_1_data_CC : &colorC8));
    fn_80028B58((s16)modelId, (s16)lightId);
    fn_800296E0((s16)modelId, (s16)lightId, 1);
    return lightId;
}

s32 fn_1_3210(float *intersectionX, float *intersectionY, float *normal,
              float x1, float y1, float x2, float y2,
              float x3, float y3, float x4, float y4)
{
    float firstDx = x2 - x1;
    float firstDy = y2 - y1;
    float secondDx = x4 - x3;
    float secondDy = y4 - y3;
    float determinant = (secondDx * firstDy) - (secondDy * firstDx);
    float firstFraction;
    float secondFraction;

    if (determinant == 0.0f) {
        return 0;
    }
    firstFraction = ((secondDx * (y1 - y3)) - (secondDy * (x1 - x3)))
        / -determinant;
    secondFraction = ((firstDx * (y3 - y1)) - (firstDy * (x3 - x1)))
        / determinant;
    if (intersectionX && intersectionY) {
        if (((firstDx * firstDx) + (firstDy * firstDy))
            >= ((secondDx * secondDx) + (secondDy * secondDy))) {
            *intersectionX = x1 + (firstDx * firstFraction);
            *intersectionY = y1 + (firstDy * firstFraction);
        } else {
            *intersectionX = x3 + (secondDx * secondFraction);
            *intersectionY = y3 + (secondDy * secondFraction);
        }
    }
    if (firstFraction >= 0.0f && firstFraction <= 1.0f
        && secondFraction >= 0.0f && secondFraction <= 1.0f) {
        if (normal != 0) {
            *normal = (firstDx * -secondDy) + (firstDy * secondDx);
        }
        return 1;
    }
    return 0;
}


typedef struct CallbackStorage {
    void (*callback)(MGPLAYER *);
    u8 unknown04[0x64]; /* No code names this field. */
} CallbackStorage;

extern CallbackStorage lbl_1_bss_A060;



/* Runs the player's current callback and marks the transition into submode 3. */
void fn_1_33E8(MGPLAYER *playerP)
{
    int previousMode;

    previousMode = playerP->subMode;
    lbl_1_bss_A060.callback(playerP);
    if (previousMode == 2 && playerP->subMode == 3) {
        playerP->work[2] = 1;
    }
}

/* Selects the hit-back motion and turns the player toward the board. */
void fn_1_3450(MGPLAYER *playerP)
{
    if (playerP->motNo != 10) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[10],
            0.0f, 5.0f, 0U);
        playerP->motNo = 10;
    }
    playerP->actor->rotY *= 0.7;
}

/* Selects the hit-front motion when the player's callback runs. */
void fn_1_34DC(MGPLAYER *playerP)
{
    if (playerP->motNo != 11) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[11],
            0.0f, 1.0f, ((1 << 30) | (1 << 0)));
        playerP->motNo = 11;
    }
}

/* Selects the knockback motion when the player's callback runs. */
void fn_1_354C(MGPLAYER *playerP)
{
    if (playerP->motNo != 12) {
        fn_8005C2C8(playerP->charNo, playerP->omObj->mtnId[12],
            0.0f, 1.0f, 0U);
        playerP->motNo = 12;
    }
}


extern s32 lbl_1_bss_0;

extern s32 lbl_1_bss_14;
extern s32 lbl_1_bss_30;
extern s32 lbl_1_data_3C;

extern s32 lbl_1_bss_A0E8;

extern MGPLAYER *lbl_1_bss_A0EC[4];

extern GXColor colorD0;





/* Updates player movement and the attached stage effects. */
static inline void applyModel(M606PlayerWork*work, OMOBJ *obj, Point3d playerPosition, MGPLAYER*player)
{
    HU3D_MODEL *model;

    if (work->playerIndex <= 0 || work->state != 3) {
        model = &lbl_802C01C8[*obj->mdlId];
        obj->trans.x = playerPosition.x;
        obj->trans.y = playerPosition.y;
        obj->trans.z = playerPosition.z;
        obj->rot.y = player->actor->rotY;
        obj->scale.x = model->scale.x;
        obj->scale.y = model->scale.y;
        obj->scale.z = model->scale.z;
    }
}

void fn_1_35B8(OMOBJ *obj)
{
    M606PlayerWork *work;
    MGPLAYER *player;
    M606PlayerWork *playerWork[4];
    Point3d actorPositions[4];
    Point3d playerPosition;
    s32 angleStep;
    void *hook;
    HU3D_MODEL *model;
    s32 motionIndex;
    M606MotionEntry *motion;
    /* These initialized locals are never read; their purpose is unknown. */
    float unknownD0;
    s32 unknownCC;
    s32 unknownC8;
    s32 playerIndexSnapshot;
    float differenceX;
    float differenceZ;
    float distanceSquared;
    float distance;

    int stickX;
    int stickY;

    work = obj->data;
    player = lbl_1_bss_A0EC[work->playerIndex];
    unknownD0 = 0.0f;
    stickX = 0;
    stickY = 0;
    unknownCC = 0;
    unknownC8 = 0;
    playerIndexSnapshot = work->playerIndex;
    lbl_1_bss_A0E8 = fn_80071A8C();
    lbl_1_bss_0 = fn_80071DE4();
    for (motionIndex = 0; motionIndex < 4; motionIndex++) {
        playerWork[motionIndex] = lbl_1_bss_A100[motionIndex].object->data;
        fn_80086BC8(lbl_1_bss_A0EC[motionIndex]->actor, &actorPositions[motionIndex]);
    }

     if (lbl_1_data_34[0] != -1 &&
         ((lbl_1_data_34[0] != 4 && lbl_1_bss_2C != 0) ||
          (lbl_1_data_34[0] == 4 && lbl_1_bss_A0E8 >= 8))) {
        if (lbl_1_bss_20 != 0) {
            fn_80028E48(*obj->mdlId, (s16)work->lightId,
                colorD0.r, colorD0.g, colorD0.b, 0xFF);
        } else {
            fn_80028E48(*obj->mdlId, (s16)work->lightId,
                colorC8.r, colorC8.g, colorC8.b, 0xFF);
        }
        fn_80026A44(*obj->mdlId, (1 << 0));
        return;
    }

    fn_80086BC8(player->actor, &playerPosition);
    if (work->playerIndex <= 0 || work->state != 3) {
        model = &lbl_802C01C8[*obj->mdlId];
        obj->trans.x = playerPosition.x;
        obj->trans.y = playerPosition.y;
        obj->trans.z = playerPosition.z;
        obj->rot.y = player->actor->rotY;
        obj->scale.x = model->scale.x;
        obj->scale.y = model->scale.y;
        obj->scale.z = model->scale.z;
    }
    for (motionIndex = 0; motionIndex < 19; motionIndex++) {
        motion = &lbl_1_bss_A168[work->playerIndex][motionIndex];
        if ((motion->startX - 100.0f < playerPosition.x) &&
            (playerPosition.x < 100.0f + motion->startX) &&
            (motion->startZ - 100.0f < playerPosition.z) &&
            (playerPosition.z < 100.0f + motion->startZ)) {
            differenceX = playerPosition.x - motion->startX;
            differenceZ = playerPosition.z - motion->startZ;
            distanceSquared = (differenceX * differenceX) + (differenceZ * differenceZ);
            if (distanceSquared == 0.0f) {
                distanceSquared = 0.0001f;
                differenceZ = 0.0001f;
            }
            if (distanceSquared < 5625.0f) {
                distanceSquared = sqrtf(distanceSquared);
                playerPosition.x = motion->startX + (75.0f * (differenceX / distanceSquared));
                playerPosition.z = motion->startZ + (75.0f * (differenceZ / distanceSquared));
            }
        }
    }
    fn_80086B04(player->actor, &playerPosition);
    if (lbl_1_bss_14 != 0 && work->finished != 0) {
        fn_80080520(player, 0, 0, lbl_802C0008[player->padNo],
                       lbl_802C0010[player->padNo]);
    } else if (lbl_1_bss_A0E8 != 5) {
        fn_80080520(player, 0, 0, lbl_802C0008[player->padNo],
                       lbl_802C0010[player->padNo]);
    } else {
        if (work->padIndex != -1) {
        stickX = lbl_802BFFFC[player->padNo];
        stickY = lbl_802BFFF8[player->padNo];
        } else {
        s32 frameThresholds[4] = {0x8C, 0x84, 0x69, 0x62};
        s32 angleOffsets[4] = {0x23, 0x1F, 0x1C, 0xA};
        s32 selectionClasses[4] = {0, 0, 1, 2};
        s32 direction;
        s32 motionClass;
        s32 selectionClass;
        s32 angleIndex;
        s32 playerMode;
        s32 frameThreshold;
        s32 signX;
        s32 signY;
        s32 collisionRadius;
        s32 movementBlocked;
        f32 moveX;
        f32 angle;
        f32 moveZ;
        f32 moveLength;
        f32 desiredX;
        f32 desiredZ;
        f32 offset;
        f32 deltaLength;
        M606MotionEntry *entry;

        direction = 0;
        if (((work->computerDifficulty < 2 && lbl_1_bss_14 != 0) != 0) && lbl_1_bss_30 == 0) {
            direction = 1;
        }
        motionClass = direction ? 2 : work->computerDifficulty;
        movementBlocked = 0;
        moveX = playerPosition.x;
        moveZ = playerPosition.z;
        {
        s32 targetDistances[8] = {0x497, 0x3CF, 0x309, 0x497, 0x309, 0x3CF, 0x497, 0x3CF};
        angle = (f32)(180.0 * (atan2(-playerPosition.x, -playerPosition.z) /
                      M_PI));
        angleIndex = (s32)((360.0f + (22.5f + angle) - (f32)(work->sequenceCount == 7 ? 0 : angleOffsets[
            (work->sequenceCount == 3 && work->computerDifficulty == 0)
                ? work->computerDifficulty + 1 : work->computerDifficulty])) /
                           45.0f) % 8;
        if (angle >= 22.5f && angle <= 57.5f) {
            angleIndex = 1;
        }
        if (work->sequenceFrame == 1) {
            work->randomValue = (s32)((80.0f * ((f32)rand8() / 255.0f)) - 40.0f);
        }
        if (work->sequenceFrame > frameThresholds[motionClass]) {
            offset = (f32)fabs(angle + ((s32)(angle < 0.0f
                ? 1.0f + (-1.0f * (angle / 45.0f))
                : -1.0f * (angle / 45.0f)) * 45));
            angleStep = (offset >= 22.49 && offset <= 22.51 && work->sameStickFrames > 0x4B) ? 1 : 0;
            desiredX = (f32)(5.6f * (((f64)(work->randomValue +
                          targetDistances[angleIndex]) *
                          sin((M_PI *
                          (f64)(180.0f + (22.5f + (f32)(((s32)(((360.0f + (22.5f + angle)) / 45.0f) + angleStep) % 8) * 0x2D)))) /
                          180.0)) - (f64)moveX));
            desiredZ = (f32)(5.6f * (((f64)(work->randomValue +
                          targetDistances[angleIndex]) *
                          cos((M_PI *
                          (f64)(180.0f + (22.5f + (f32)(((s32)(((360.0f + (22.5f + angle)) / 45.0f) + angleStep) % 8) * 0x2D)))) /
                          180.0)) - (f64)moveZ));
            fn_1_134(&desiredX, &desiredZ, 56.0f);
            moveX += desiredX;
            moveZ += desiredZ;
            if (work->sameStickFrames > 0xF0) {
                float rotationPreviousX;
                float rotationX;
                float rotationZ;
                rotationX = moveX - playerPosition.x;
                rotationZ = moveZ - playerPosition.z;
                if (lbl_1_data_3C % 120 > 0x3C) {
                    rotationPreviousX = rotationX;
                    rotationX = -rotationZ;
                    rotationZ = rotationX;
                } else {
                    rotationPreviousX = rotationX;
                    rotationX = rotationZ;
                    rotationZ = -rotationX;
                }
                moveX = playerPosition.x + (2.0f * rotationX);
                moveZ = playerPosition.z + (2.0f * rotationZ);
            }
            stickX = (s32)(5.6f * (moveX - playerPosition.x) / 4.0f);
            stickY = -(s32)(5.6f * (moveZ - playerPosition.z) / 4.0f);
            fn_1_2DC(&stickX, &stickY, 56.0f);
            if (angle > -76.0f && angle < -20.0f && stickX == 0 && stickY == 0) {
                float nudgeX;
                float nudgeZ;
                nudgeX = (f32)(30.0 * -cos((M_PI * (f64)angle) / 180.0));
                nudgeZ = (f32)(30.0 * sin((M_PI * (f64)angle) / 180.0));
                moveX += nudgeX;
                desiredX += nudgeX;
                moveZ += nudgeZ;
                desiredZ += nudgeZ;
            }
            for (motionIndex = 0; motionIndex < 19; motionIndex++) {
                f32 deltaX;
                f32 deltaZ;
                entry = &lbl_1_bss_A168[work->playerIndex][motionIndex];
                if (((selectionClasses[motionClass] != 0 &&
                    (selectionClasses[motionClass] != 1 || motionIndex % 2 == 0)) ||
                     work->currentMode == motionIndex) && entry->kind != 0 &&
                    entry->startX - 200.0f < moveX &&
                    moveX < 200.0f + entry->startX &&
                    entry->startZ - 200.0f < moveZ &&
                    moveZ < 200.0f + entry->startZ) {
                    deltaX = (moveX - entry->startX) + (2.0f *
                        (entry->kind == 2 ? entry->deltaX * entry->angularSpeed : entry->directionX));
                    deltaZ = (moveZ - entry->startZ) + (2.0f *
                        (entry->kind == 2 ? entry->deltaZ * entry->angularSpeed : entry->directionZ));
                    collisionRadius = (entry->overlayEnabled != 0 &&
                        (entry->motionState == 1 || entry->motionState == 2)) ? 0x87 : 0x64;
                    deltaLength = deltaX * deltaX + deltaZ * deltaZ;
                    if (deltaLength == 0.0f) {
                        deltaLength = 1.0f;
                        deltaZ = 1.0f;
                    }
                    if (deltaLength < (f32)(collisionRadius * collisionRadius)) {
                        deltaLength = sqrtf(deltaLength);
                        moveX = entry->startX +
                            ((f32)collisionRadius * (deltaX / deltaLength));
                        moveZ = entry->startZ +
                            ((f32)collisionRadius * (deltaZ / deltaLength));
                        if (work->sequenceCount == 3 && work->currentMode == 2 &&
                            motionIndex == 2 && playerPosition.x <= moveX) {
                            moveX = playerPosition.x - 56.0f;
                            moveZ = playerPosition.z;
                        }
                        movementBlocked = 1;
                        work->framesSinceCollision = 0;
                    }
                }
            }
        }
            work->framesSinceCollision++;
            stickX = (s32)(5.6f * (moveX - playerPosition.x) / 4.0f);
            stickY = -(s32)(5.6f * (moveZ - playerPosition.z) / 4.0f);
            fn_1_2DC(&stickX, &stickY, 56.0f);
            if (angle > -76.0f && angle < -20.0f) {
                if (work->stickOverrideFrames > 0) {
                    work->stickOverrideFrames--;
                    if (work->stickOverrideFrames > 2) {
                        stickX = work->previousStickX;
                        stickY = work->previousStickY;
                    }
                    if (work->stickOverrideFrames == 0) {
                        work->currentMode = -1;
                    }
                } else if ((stickX > 5 || stickX < -5 || stickY > 5 || stickY < -5) &&
                           (work->earlierStickX > 5 || work->earlierStickX < -5 ||
                            work->earlierStickY > 5 || work->earlierStickY < -5) &&
                           fabs(stickX - work->earlierStickX) < 3.0 &&
                           fabs(stickY - work->earlierStickY) < 3.0 &&
                           ((stickX * work->previousStickX < 0) || (stickY * work->previousStickY < 0)) &&
                           (stickX * work->previousStickX <= 0) && (stickY * work->previousStickY <= 0)) {
                    stickX = -stickX + ((rand8() % 4) * ((rand8() % 2) * 2 - 1));
                    stickY = -stickY + ((rand8() % 4) * ((rand8() % 2) * 2 - 1));
                    work->stickOverrideFrames = (rand8() % 5) + 0x11;
                }
            }
            work->sameStickFrames = (stickX == work->previousStickX &&
                stickY == work->previousStickY) ? work->sameStickFrames + 1 : 0;
            work->earlierStickX = work->previousStickX;
            work->earlierStickY = work->previousStickY;
            work->previousStickX = stickX;
            work->previousStickY = stickY;
        }
        }
        fn_80080520(player, stickX, stickY,
                       lbl_802C0008[player->padNo],
                       lbl_802C0010[player->padNo]);
    }

    if (work->sequenceFrame >= 0 && work->sequenceFrame < 0x5A) {
        float scale = work->sequenceFrame <= 7
            ? 1.0f - ((float)work->sequenceFrame / 7.0f) : 0.0f;
        fn_80026A44(lbl_1_bss_A0FC->mdlId[work->playerIndex + 0x2C], (1 << 0));
        fn_800268B0(lbl_1_bss_A0FC->mdlId[work->playerIndex + 0x2C],
                  2.5f, 2.5f, 2.5f);
        fn_80026760(lbl_1_bss_A0FC->mdlId[work->playerIndex + 0x2C],
                  playerPosition.x, 15.0f + playerPosition.y,
                  playerPosition.z);
        if (lbl_1_bss_20 != 0) {
            fn_80028E48(*obj->mdlId, (s16)work->lightId,
                (u8)(colorD0.r * scale),
                (u8)(colorD0.g * scale),
                (u8)(colorD0.b * scale), 0xFFU);
        } else {
            fn_80028E48(*obj->mdlId, (s16)work->lightId,
                (u8)(colorC8.r * scale),
                (u8)(colorC8.g * scale),
                (u8)(colorC8.b * scale), 0xFFU);
        }
        {
            hook = fn_8005C0F0((s16)work->character, 8, 4);
            if (hook) {
                Point3d hookPosition;
                Hu3DModelObjPosGet(*obj->mdlId, hook, &hookPosition);
                fn_80026760(lbl_1_bss_A0FC->mdlId[work->playerIndex + 0x2C],
                    hookPosition.x, hookPosition.y, hookPosition.z);
            }
        }
    } else {
        fn_800269E8(lbl_1_bss_A0FC->mdlId[work->playerIndex + 0x2C], (1 << 0));
        if (lbl_1_bss_20 != 0) {
            fn_80028E48(*obj->mdlId, (s16)work->lightId,
                colorD0.r, colorD0.g, colorD0.b, 0xFFU);
        } else {
            fn_80028E48(*obj->mdlId, (s16)work->lightId,
                colorC8.r, colorC8.g, colorC8.b, 0xFFU);
        }
    }
    if (work->sequenceFrame == 0) {
        fn_8002D414(lbl_1_bss_A0FC->mdlId[work->playerIndex + 0x2C], 0.0f);
        fn_8007EEBC(player, 0xB);
    }
    if (work->sequenceFrame == 1) {
        fn_8005C2C8(player->charNo, player->omObj->mtnId[10],
                           6.0f, 0.0f, 0U);
    }
    if (work->sequenceFrame < 0x5A) {
        fn_80080520(player, 0, 0, lbl_802C0008[player->padNo],
                       lbl_802C0010[player->padNo]);
    }
    if (work->sequenceFrame == 0x5A) {
        fn_8005C2C8(player->charNo, *player->omObj->mtnId,
                           0.0f, 0.0f, ((1 << 30) | (1 << 0)));
    }
    if (work->sequenceFrame >= 0x5A && work->sequenceFrame <= 0x96 &&
        work->sequenceFrame % 2 == 0) {
        fn_800269E8(*obj->mdlId, (1 << 0));
    } else {
        fn_80026A44(*obj->mdlId, (1 << 0));
    }
    work->sequenceFrame += work->sequenceFrame < 0x8000;
}



/* Returns the normalized rise, hold, and fall envelope at the requested frame. */
float fn_1_58F8(float frame, float rise, float hold, float fall)
{
    float area = fall / 2.0f + (rise / 2.0f + hold);

    return frame < rise
        ? ((1.0f + frame) / 2.0f * frame) / rise / area
        : frame < rise + hold
            ? ((1.0f + rise) / 2.0f * rise) / rise / area
                + (frame - rise) / area
            : frame < fall + (rise + hold) - 1.0f
                ? 1.0f - ((1.0f + (fall + (rise + hold) - frame - 1.0f))
                    / 2.0f * (fall + (rise + hold) - 1.0f - frame - 1.0f))
                    / (fall - 1.0f) / area
                : 1.0f;
}


/* Advances the selected camera's active motion entries and updates their models. */




void fn_1_5ABC(s32 cameraIndex, s32 mode)
{
    OMOBJ *object = lbl_1_bss_A0FC;
    u32 sequenceMode = fn_80071A8C();
    s32 patternIndex;
    s32 initialRingIndex = lbl_1_data_0[cameraIndex];
    s32 modelIndex;
    M606MotionEntry *entry;
    float magnitude;
    float deltaX;
    float deltaZ;
    float adjustedDeltaX;
    float adjustedDeltaZ;
    float normalizedX;
    float normalizedZ;
    float turnAmount;

    for (patternIndex = 0; patternIndex < 12; patternIndex++) {
        entry = &lbl_1_bss_A168[cameraIndex][lbl_1_data_0[cameraIndex]];
        if (lbl_1_rodata_40[mode][patternIndex].kind == 0.0f) {
            break;
        }
        entry->kind = (s32)lbl_1_rodata_40[mode][patternIndex].kind;
        entry->parameter = (s32)lbl_1_rodata_40[mode][patternIndex].parameter;
        entry->flags = 0;
        entry->collisionKind = 0;
        entry->directionX = entry->directionZ = entry->targetX =
            entry->targetZ = entry->deltaX = entry->deltaZ =
            entry->angularSpeed = 0.0f;
        entry->motionMode = mode;
        entry->motionState = 0;
        entry->randomOffset = (s32)(entry->kind == 1 ?
            (float)(rand8() - 30) + 220.0f * ((float)rand8() / 255.0f) :
            entry->kind == 2 ? (float)rand8() +
            280.0f * ((float)rand8() / 255.0f) : 60.0f);
        entry->overlayEnabled = 0;
        entry->overlayModelIndex = -1;
        entry->scale = (s32)sequenceMode < 5 ?
            (float)(patternIndex - 7) / 5.0f +
            (mode == 0 ? 0.75f : 0.0f) : 1.0f;
        entry->scale = entry->scale > 1.0f ? 1.0f : entry->scale;

        switch (entry->kind) {
        case 1:
            entry->startX = lbl_1_rodata_40[mode][patternIndex].startX;
            entry->startZ = lbl_1_rodata_40[mode][patternIndex].startZ;
            break;
        case 2:
            entry->targetX = lbl_1_rodata_40[mode][patternIndex].startX;
            entry->targetZ = lbl_1_rodata_40[mode][patternIndex].startZ;
            entry->deltaX = lbl_1_rodata_40[mode][patternIndex].targetX -
                lbl_1_rodata_40[mode][patternIndex].startX;
            entry->deltaZ = lbl_1_rodata_40[mode][patternIndex].targetZ -
                lbl_1_rodata_40[mode][patternIndex].startZ;
            deltaZ = entry->deltaZ;
            deltaX = entry->deltaX;
            magnitude = fn_1_0(deltaX, deltaZ);
            normalizedX = 70.0f * (entry->deltaX / magnitude);
            normalizedZ = 70.0f * (entry->deltaZ / magnitude);
            entry->targetX += normalizedX;
            entry->targetZ += normalizedZ;
            entry->deltaX -= 2.0f * normalizedX;
            entry->deltaZ -= 2.0f * normalizedZ;
            entry->speed = (float)rand8() / 255.0f;
            adjustedDeltaZ = entry->deltaZ;
            adjustedDeltaX = entry->deltaX;
            entry->angularSpeed = (((150.0f / fn_1_0(adjustedDeltaX, adjustedDeltaZ)) /
                60.0f) * (float)(((rand8() % 2) * 2) - 1));
            entry->startX = entry->targetX + (entry->deltaX * entry->speed);
            entry->startZ = entry->targetZ + (entry->deltaZ * entry->speed);
            break;
        case 3:
            entry->startX = lbl_1_rodata_40[mode][patternIndex].startX;
            entry->startZ = lbl_1_rodata_40[mode][patternIndex].startZ;
            turnAmount = 360.0f * ((float)rand8() / 256.0f);
            entry->directionX = (150.0 *
                sin((M_PI * (double)turnAmount) / 180.0)) /
                60.0;
            entry->directionZ = (150.0 *
                cos((M_PI * (double)turnAmount) / 180.0)) /
                60.0;
            break;
        }

        modelIndex = (cameraIndex * 0x13) + 0x7E + lbl_1_data_0[cameraIndex];
        fn_8002C95C(object->mdlId[modelIndex],
                      object->mtnId[0]);
        fn_800269E8(object->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
        fn_80026B64(object->mdlId[modelIndex],
                           (u16)lbl_1_rodata_30[cameraIndex]);
        fn_80026760(object->mdlId[modelIndex], entry->startX,
                        75.0f, entry->startZ);
        fn_800268B0(object->mdlId[modelIndex], 0.7f,
                          0.7f, 0.7f);
        fn_8002D414(object->mdlId[modelIndex],
            fn_8002D530(object->mdlId[modelIndex]) *
            (float)rand8() / 256.0f);
        fn_80026A44(object->mdlId[modelIndex], (1 << 0));
        lbl_1_data_0[cameraIndex]++;
        lbl_1_data_0[cameraIndex] %= 19;
    }

    switch (mode) {
    case 2:
        lbl_1_bss_A168[cameraIndex][(initialRingIndex + 1) % 19].angularSpeed *= 2.0f;
        break;
    case 3:
        lbl_1_bss_A168[cameraIndex][(initialRingIndex + (rand8() % 3)) % 19].angularSpeed *= 2.0f;
        break;
    case 6:
        lbl_1_bss_A168[cameraIndex][initialRingIndex % 19].angularSpeed *= 2.0f;
        lbl_1_bss_A168[cameraIndex][(initialRingIndex + 1) % 19].angularSpeed *= 2.0f;
        lbl_1_bss_A168[cameraIndex][(initialRingIndex + 2) % 19].angularSpeed *= 2.0f;
        break;
    case 7:
        lbl_1_bss_A168[cameraIndex][initialRingIndex % 19].directionX *= 2.0f;
        lbl_1_bss_A168[cameraIndex][initialRingIndex % 19].directionZ *= 2.0f;
        lbl_1_bss_A168[cameraIndex][(initialRingIndex + 1) % 19].directionX *= 2.0f;
        lbl_1_bss_A168[cameraIndex][(initialRingIndex + 1) % 19].directionZ *= 2.0f;
        lbl_1_bss_A168[cameraIndex][(initialRingIndex + 2) % 19].directionX *= 2.0f;
        lbl_1_bss_A168[cameraIndex][(initialRingIndex + 2) % 19].directionZ *= 2.0f;
        break;
    case 5: {
        s32 selectionFlags[6] = { 0, 0, 0, 0, 0, 0 };
        patternIndex = 0;
        while (patternIndex < 3) {
            s32 skip = (patternIndex * 2) + (rand8() % 2);
            if (selectionFlags[skip] == 0) {
                lbl_1_bss_A168[cameraIndex][(initialRingIndex + skip) % 19].angularSpeed *= 2.0f;
                patternIndex++;
            }
        }
        for (patternIndex = 0; patternIndex < 6; patternIndex++) {
            lbl_1_bss_A168[cameraIndex][(initialRingIndex + patternIndex) % 19].speed =
                (patternIndex % 2) == 0 ? 0.25f : 0.75f;
        }
        break;
    }
    }
}


void fn_1_6BE0(s32 sourcePlayer, s32 targetPlayer)
{
    OMOBJ *work = lbl_1_bss_A0FC;
    s32 modelIndex;
    s32 entry;

    lbl_1_data_0[targetPlayer] = lbl_1_data_0[sourcePlayer];
    for (entry = 0; entry < 19; entry++) {
        M606MotionEntry *source = &lbl_1_bss_A168[sourcePlayer][entry];
        M606MotionEntry *destination = &lbl_1_bss_A168[targetPlayer][entry];
        memcpy(destination, source, sizeof(*destination));
        modelIndex = 0x7E + targetPlayer * 19 + entry;
        if (destination->kind == 0) {
            fn_800269E8(work->mdlId[modelIndex], (1 << 0));
        } else {
            fn_8002C95C(work->mdlId[modelIndex], work->mtnId[0]);
            fn_800269E8(work->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
            fn_80026B64(work->mdlId[modelIndex],
                               (u16)lbl_1_rodata_30[targetPlayer]);
            fn_80026760(work->mdlId[modelIndex], destination->startX,
                            75.0f, destination->startZ);
            fn_800268B0(work->mdlId[modelIndex], 0.7f,
                              0.7f, 0.7f);
            fn_8002D414(work->mdlId[modelIndex],
                fn_8002D4E0(work->mdlId[0x7E + sourcePlayer * 19 + entry]));
            fn_80026A44(work->mdlId[modelIndex], (1 << 0));
        }
    }
}

void fn_1_6DBC(s32 angleIndex, s32 side, float *x, float *y)
{
    float sideValues[2][2] = { { -385.5f, -1167.0f }, { -188.0f, -652.0f } };
    s32 diagonal;
    s32 row;
    float localX;
    float localY;

    diagonal = ((side / 2) + 7) % 8;
    row = ((side + 1) % 4) / 2;
    if (x == 0 || y == 0) {
        return;
    }
    localX = (float)(diagonal >= 4 ? -1 : 1) * sideValues[row][((diagonal + 1) / 2) % 2];
    localY = sideValues[row][1 - (((diagonal + 1) / 2) % 2)] *
        (float)(diagonal >= 2 && diagonal < 6 ? -1 : 1);
    *x = (float)(((double)localX * cos((M_PI * (double)(-angleIndex * 0x2D)) / 180.0)) -
                 ((double)localY * sin((M_PI * (double)(-angleIndex * 0x2D)) / 180.0)));
    *y = (float)(((double)localY * cos((M_PI * (double)(-angleIndex * 0x2D)) / 180.0)) +
                 ((double)localX * sin((M_PI * (double)(-angleIndex * 0x2D)) / 180.0)));
}


#define M606_BITWISE(type, expr) ((type)(expr))

struct _struct_lbl_1_bss_A118_0x8 {
    s32 actorId;
    s32 modelId;
};

M606MotionEntry lbl_1_bss_A168[4][19];
struct _struct_lbl_1_bss_A118_0x8 lbl_1_bss_A118[0xA];
OMOBJ *lbl_1_bss_A114;
OMOBJ *lbl_1_bss_A110;
M606ActorObjectRef lbl_1_bss_A100[4];
OMOBJ *lbl_1_bss_A0FC;
MGPLAYER *lbl_1_bss_A0EC[4];
s32 lbl_1_bss_A0E8;
OMOBJMAN *lbl_1_bss_A0E4;
s16 lbl_1_bss_A0E0;
s32 lbl_1_bss_A0DC;

M606SpriteState lbl_1_bss_A0C8;

CallbackStorage lbl_1_bss_A060;
s32 lbl_1_bss_40[0x2808];
s32 lbl_1_bss_3C;

M606AngleState lbl_1_bss_34;
s32 lbl_1_bss_30;
s32 lbl_1_bss_2C;
s32 lbl_1_bss_28;
s32 lbl_1_bss_24;
s32 lbl_1_bss_20;
s32 lbl_1_bss_1C;
s32 lbl_1_bss_18;
s32 lbl_1_bss_14;

M606TimerSlot lbl_1_bss_8;
MGTIMER *lbl_1_bss_4;
s32 lbl_1_bss_0;


void fn_1_5ABC(s32 cameraIndex, s32 mode);

extern s32 lbl_1_data_10[4];

extern s32 lbl_1_data_104[4];

extern s32 lbl_1_data_114[8];

extern s32 lbl_1_data_134[8];

extern s32 lbl_1_data_20[5];

extern f32 lbl_1_data_D4[4];

extern f32 lbl_1_data_E4[4];

extern s32 lbl_1_data_F4[4];












void fn_1_7110(OMOBJ *object)
{
    M606PlayerWork *playerData[4];
    f64 modelScaleDouble;
    s32 lastCameraMask;
    f32 unsplitFraction;
    s32 viewportColumn;
    s32 viewportRow;
    s32 splitWidth;
    s32 splitHeight;
    f32 viewportWidth;
    f32 viewportHeight;
    u32 playerVisibilityMask;
    s32 conditionSentinel;
    s32 entryModelIndex;
    f32 motionXCopy;
    f32 motionZCopy;
    f32 modelScale;
    s32 scaledModelHandle;
    f32 intersectionX;
    f32 intersectionZ;
    f32 intersectionCross;
    s32 boundaryHit;
    f32 minimumCross;
    f32 cornerCross;
    f32 boundaryMotionMagnitude;
    f32 entryPositionX;
    f32 entryPositionY;
    f32 entryPositionZ;
    s32 index;

    f32 transitionProgress;
    f32 splitProgress;
    f32 entryBlend;
    f32 cameraPreviousBlend;
    f32 entryOuterBlend;
    f32 viewportScale;
    f32 playerSectorAngle;

    s32 collisionShapeIndex;
    s32 copyAndEdgeEndIndex;
    s32 copyAndEdgeStartIndex;
    s32 cornerIndex;

    s32 playerIndex;
    struct _struct_lbl_1_bss_A118_0x8 *actorCounter;

    M606MotionEntry *entry;

    for (index = 0; index < 4; index++) {
        playerData[index] = lbl_1_bss_A100[index].object->data;
    }

    lbl_1_bss_A0E8 = fn_80071A8C();
    lbl_1_bss_0 = fn_80071DE4();

    lbl_1_data_3C++;
    lbl_1_bss_34.value += 0.03125f;

    lbl_1_bss_34.value -= (f32) (lbl_1_bss_34.value >= 360.0f ? 0x168 : 0);
    fn_80026808(object->mdlId[2], 0.0f, lbl_1_bss_34.value, 0.0f);
    if ((lbl_1_bss_18 == 0) || (lbl_1_bss_18 == 1)) {
        f32 cameraZoom[3] = { 8450.0f, 1250.0f, 2325.0f };
        f32 cameraCenterY[3] = { 60.0f, 75.0f, 75.0f };
        f32 cameraRotationX[3] = { -33.0f, -30.0f, -70.0f };
        f32 cameraRotationY[3] = { 0.0f, -90.0f, 0.0f };

        lbl_1_bss_1C += ((lbl_1_data_3C >= 0x1B) && (lbl_1_bss_A0E8 >= 2));
        transitionProgress = lbl_1_bss_18 == 0
            ? fn_1_58F8((f32)(lbl_1_bss_1C * 0x3C) / 60.0f, 40.0f, 40.0f, 40.0f)
            : fn_1_58F8(90.0f - (f32)(lbl_1_bss_1C * 0x3C) / 60.0f, 5.0f, 80.0f, 5.0f);
        transitionProgress = lbl_1_bss_18 == 0 ? transitionProgress : 1.0f - transitionProgress;
        cameraPreviousBlend = 1.0f - transitionProgress;
        for (index = 0; index < 4; index++) {
            lbl_80266BD8[index] = (cameraPreviousBlend * cameraZoom[lbl_1_bss_18]) +
                (transitionProgress * cameraZoom[lbl_1_bss_18 + 1]);

            lbl_80266B18[index].x = 0.0f;
            lbl_80266B18[index].y = (cameraPreviousBlend * cameraCenterY[lbl_1_bss_18]) +
                (transitionProgress * cameraCenterY[lbl_1_bss_18 + 1]);

            lbl_80266B18[index].z = -900.0f;
            lbl_80266A58[index].x = (cameraPreviousBlend * cameraRotationX[lbl_1_bss_18]) +
                (transitionProgress * cameraRotationX[lbl_1_bss_18 + 1]);

            lbl_80266A58[index].y = (cameraPreviousBlend * cameraRotationY[lbl_1_bss_18]) +
                (transitionProgress * cameraRotationY[lbl_1_bss_18 + 1]);

            lbl_80266A58[index].z = 0.0f;
            if (lbl_1_bss_18 == 1) {
                splitProgress = lbl_1_bss_1C >= 0x1E ? (f32) (lbl_1_bss_1C - 0x1E) / 60.0f : 0.0f;
                unsplitFraction = 1.0f - splitProgress;
                viewportColumn = index % 2;
                viewportRow = index / 2;
                splitWidth = (s32) (320.0f * splitProgress);
                splitHeight = (s32) (240.0f * splitProgress);
                viewportScale = 0.5f + (0.5f * unsplitFraction);
                viewportWidth = (viewportColumn != 0) ? (f32) splitWidth + (16.0f * splitProgress) : (640.0f * viewportScale) + (16.0f * splitProgress);

                viewportHeight = (viewportRow != 0 ? (f32) splitHeight + (40.0f * splitProgress) : (480.0f * viewportScale) + (40.0f * splitProgress));
                Hu3DCameraPerspectiveSet(lbl_1_rodata_30[index], 20.0f, 20.0f, 25000.0f, ((1.2f * ((640.0f * viewportScale) + (16.0f * splitProgress))) / ((480.0f * viewportScale) + (40.0f * splitProgress))) / 1.3333334f);

                Hu3DCameraViewportSet(lbl_1_rodata_30[index], (viewportColumn != 0 ? (f32) (0x280 - splitWidth) - (16.0f * splitProgress) : 0.0f), (viewportRow != 0 ? (f32) (0x1E0 - splitHeight) - (40.0f * splitProgress) : 0.0f), viewportWidth, viewportHeight, 0.0f, 1.0f);

        Hu3DCameraScissorSet(lbl_1_rodata_30[index], (viewportColumn != 0 ? 0x280 - splitWidth : 0) + 2, (viewportRow != 0 ? 0x1E0 - splitHeight : 0) + 2,
                             (u32)((viewportColumn != 0 ? (f32) splitWidth : 0.9999f + (640.0f * viewportScale)) - 4.0f), (u32)((viewportRow != 0 ? (f32) splitHeight : 0.9999f + (480.0f * viewportScale)) - 4.0f));
            }
        }

        if (lbl_1_bss_1C >= (lbl_1_bss_18 == 0 ? 0x78 : 0x5A)) {
            lbl_1_bss_18++;
            lbl_1_bss_1C = 0;
            if (lbl_1_bss_18 == 1) {
                fn_800269E8(*lbl_1_bss_A0FC->mdlId, (1 << 0));
                fn_800269E8(lbl_1_bss_A0FC->mdlId[2], (1 << 0));
                index = 0;
                while (index < 8) {
                    fn_80026A44(*(lbl_1_bss_A0FC->mdlId + ((index + 0x24))), (1 << 0));
                    fn_800269E8(*(lbl_1_bss_A0FC->mdlId + ((index + 0x1C))), (1 << 0));
                    index++;
                }
                fn_1_6BE0(0, 1);
                fn_1_6BE0(0, 2);
                fn_1_6BE0(0, 3);
            }
        }
        if ((lbl_1_bss_18 == 0) && (lbl_1_bss_1C == 0x3C)) {
            index = 0;
            while (index < 8) {
                fn_80026A44(object->mdlId[(index + 0x14)], ((1 << 30) | (1 << 1)));
                index++;
            }
        }
        index = 0;
        while (index < 8) {
            lastCameraMask = lbl_1_bss_18 == 0 ? 1 : (index <= 2 || index >= 6 ? 0xFFFF : 0);
            fn_80026B64(object->mdlId[(index + 0x24)], (u16) lastCameraMask);
            fn_80026B64(object->mdlId[(index + 0x14)], (u16) lastCameraMask);
            fn_80026B64(object->mdlId[(index + 0xC)], (u16) lastCameraMask);
            index++;
        }
        fn_80026B64(object->mdlId[0], (u16) lastCameraMask);
    } else if (lbl_1_bss_18 == 2) {
        Point3d playerPosition;
        playerVisibilityMask = 0x10;
        {
            s32 playerMasks[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            s32 modelMasks[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            s32 overlayMasks[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                fn_80086BC8(lbl_1_bss_A0EC[playerIndex]->actor, &playerPosition);

                playerSectorAngle = (f32)((360.0 + (22.5 + (180.0 * (atan2(-((f64)playerPosition.x) - (110.0 * cos((M_PI * (f64)(22.5f + (f32)(lbl_1_data_F4[playerIndex] * 0x2D))) / 180.0)), (-(f64)playerPosition.z + (110.0 * sin((M_PI * (f64) (22.5f + (f32) (lbl_1_data_F4[playerIndex] * 0x2D))) / 180.0)))) / M_PI)))) / 45.0);

                playerSectorAngle = playerSectorAngle + (f32) (playerSectorAngle < 0.0f ? 8 : (playerSectorAngle >= 16.0f ? -0x10 : (playerSectorAngle >= 8.0f ? -8 : 0)));

                playerSectorAngle = playerSectorAngle + (f32) (playerSectorAngle < (f32) lbl_1_data_F4[playerIndex] ? 8 : 0);
                if ((((f32) lbl_1_data_F4[playerIndex] - 1.5f) < playerSectorAngle) && (playerSectorAngle < (0.5f + (f32) lbl_1_data_F4[playerIndex]))) {
                    playerVisibilityMask |= 1 << playerIndex;
                }

                if ((1.0f + (f32) (lbl_1_data_F4[playerIndex])) < playerSectorAngle) {
                    fn_80026A44(object->mdlId[(0x5D + (lbl_1_data_F4[playerIndex] * 4) + playerIndex)], ((1 << 30) | (1 << 1)));
                    fn_80026A44(object->mdlId[(0x5D + (lbl_1_data_F4[playerIndex] * 4) + playerIndex)], (1 << 0));
                    fn_8002D414(object->mdlId[(0x5D + (lbl_1_data_F4[playerIndex] * 4) + playerIndex)], 0.0f);
                    if ((s32) lbl_1_bss_A118[playerIndex + 1].actorId < 3) {
                        lbl_1_bss_A118[playerIndex + 1].actorId++;
                    }
                    fn_8002D414(object->mdlId[(0x5D + ((((s32) (lbl_1_data_F4[playerIndex] + 6) % 8)) * 4) + playerIndex)], 0.0f);
                    fn_800269E8(object->mdlId[(0x5D + ((((s32) (lbl_1_data_F4[playerIndex] + 6) % 8)) * 4) + playerIndex)], ((1 << 30) | (1 << 1)));
                    fn_800269E8(object->mdlId[(0x5D + ((((s32) (lbl_1_data_F4[playerIndex] + 6) % 8)) * 4) + playerIndex)], (1 << 0));
                    fn_80026A44(object->mdlId[(0x5D + ((((s32) (lbl_1_data_F4[playerIndex] + 2) % 8)) * 4) + playerIndex)], (1 << 0));

                    lbl_1_data_F4[playerIndex]++;

                    lbl_1_data_F4[playerIndex] %= 8;

                    playerData[playerIndex]->childIndex += (((s32) (((s32) ((playerData[playerIndex])->sequenceCount + 1) % 8)) == 0) && ((s32) (playerData[playerIndex])->sequenceCount == 7));

                    playerData[playerIndex]->sequenceCount++;

                    playerData[playerIndex]->sequenceCount %= 8;
                    (playerData[playerIndex])->currentMode = -1;
                    fn_1_5ABC(playerIndex, ((s32) ((playerData[playerIndex])->sequenceCount + 1) % 8));
                }
                playerSectorAngle = (f32)((360.0 + (22.5 + (180.0 * (atan2(-playerPosition.x, -playerPosition.z) / M_PI)))) / 45.0);

                playerSectorAngle = playerSectorAngle + (f32) (playerSectorAngle < 0.0f ? 8 : (playerSectorAngle >= 16.0f ? -0x10 : (playerSectorAngle >= 8.0f ? -8 : 0)));

                playerSectorAngle = playerSectorAngle + (f32) (playerSectorAngle < (f32) lbl_1_data_104[playerIndex] ? 8 : 0);
                if (((7.875f + (f32) lbl_1_data_104[playerIndex]) > playerSectorAngle) && ((f32) (lbl_1_data_104[playerIndex] + 4) < playerSectorAngle)) {

                    lbl_1_data_104[playerIndex] -= 1;

                    lbl_1_data_104[playerIndex] += ((s32) lbl_1_data_104[playerIndex] < 0 ? 8 : 0);
                } else if (((1.0f + (f32) lbl_1_data_104[playerIndex]) < playerSectorAngle) && ((f32) (lbl_1_data_104[playerIndex] + 4) >= playerSectorAngle)) {

                    lbl_1_data_104[playerIndex]++;

                    lbl_1_data_104[playerIndex] %= 8;
                }
                if ((lbl_1_data_D4[playerIndex] == 1.0f) && (lbl_1_data_E4[playerIndex] == 1.0f)) {
                    lbl_1_data_D4[playerIndex] = playerPosition.x;
                    lbl_1_data_E4[playerIndex] = playerPosition.z;
                }

                lbl_1_data_D4[playerIndex] = (f32) ((f64) (0.9375f * lbl_1_data_D4[playerIndex]) - (0.0625 * (900.0 * sin((M_PI * (f64) (lbl_1_data_104[playerIndex] * 0x2D)) / 180.0))));
                lbl_1_data_E4[playerIndex] = (f32) ((f64) (0.9375f * lbl_1_data_E4[playerIndex]) - (0.0625 * (900.0 * cos((M_PI * (f64) (lbl_1_data_104[playerIndex] * 0x2D)) / 180.0))));
                lbl_80266B18[playerIndex].x = (0.625f * playerPosition.x) + (0.375f * lbl_1_data_D4[playerIndex]);
                lbl_80266B18[playerIndex].y = 75.0f;
                lbl_80266B18[playerIndex].z = (0.625f * playerPosition.z) + (0.375f * lbl_1_data_E4[playerIndex]);
                playerSectorAngle = (f32) (180.0 + (180.0 * (atan2((f64) lbl_80266B18[playerIndex].x, (f64) lbl_80266B18[playerIndex].z) / M_PI)));
                playerSectorAngle = playerSectorAngle >= 360.0f ? playerSectorAngle - 360.0f : playerSectorAngle;

                modelMasks[((s32) (playerSectorAngle / 45.0f) % 8)] |= lbl_1_rodata_30[playerIndex];
                modelMasks[((s32) ((s32) (playerSectorAngle / 45.0f) + 1) % 8)] |= lbl_1_rodata_30[playerIndex];
                modelMasks[((s32) ((s32) (playerSectorAngle / 45.0f) + 7) % 8)] |= lbl_1_rodata_30[playerIndex];
                overlayMasks[((s32) ((s32) ((22.5f + (7.5f + playerSectorAngle)) / 45.0f) + 6) % 8)] |= lbl_1_rodata_30[playerIndex];
                overlayMasks[((s32) ((s32) ((22.5f + (7.5f + playerSectorAngle)) / 45.0f) + 7) % 8)] |= lbl_1_rodata_30[playerIndex];

                overlayMasks[((s32) ((22.5f + (7.5f + playerSectorAngle)) / 45.0f) % 8)] |= lbl_1_rodata_30[playerIndex];

                overlayMasks[(((s32) ((22.5f + (7.5f + playerSectorAngle)) / 45.0f) + 1) % 8)] |= lbl_1_rodata_30[playerIndex];
                playerSectorAngle += 22.5f;
                playerSectorAngle = (playerSectorAngle >= 360.0f ? playerSectorAngle - 360.0f : playerSectorAngle) / 45.0f;
                playerMasks[((s32) playerSectorAngle % 8)] |= lbl_1_rodata_30[playerIndex];
                playerMasks[((s32) (1.0f + playerSectorAngle) % 8)] |= lbl_1_rodata_30[playerIndex];

                playerMasks[((s32) (7.0f + playerSectorAngle) % 8)] |= lbl_1_rodata_30[playerIndex];
            }
            index = 0;
            while (index < 8) {
                fn_80026B64(object->mdlId[(index + 0x24)], (u16) modelMasks[index]);
                fn_80026B64(object->mdlId[(index + 0x14)], (u16) overlayMasks[index]);
                fn_80026B64(object->mdlId[(index + 0xC)], (u16) playerMasks[index]);
                index++;
            }
        }
        fn_80080D7C(2, playerVisibilityMask);
    } else if (lbl_1_bss_18 == 3) {
        /* no code in the release build */
    }
    if (lbl_1_bss_20 != 0) {
        playerIndex = 0;
        while (playerIndex < 4) {
            playerIndex++;
        }
    }
    if ((lbl_1_data_34[0] == -1) || (lbl_1_bss_2C == 0)) {
        Point3d incomingMotion;
        Point3d contactNormal;
        Point3d reflectedMotion;
        f32 cornerDeltaX[4];
        f32 cornerDeltaZ[4];
        Point3d boundaryMotion;
        Point3d boundaryNormal;
        Point3d boundaryReflectedMotion;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            conditionSentinel = -1;
            for (index = 0; index < 0x13; index++) {
                entry = &lbl_1_bss_A168[playerIndex][index];
                entryModelIndex = 0x7E + (playerIndex * 0x13) + index;
                if (entry->kind != 0) {
                    if ((s32) entry->collisionKind != 0) {

                        motionXCopy = (entry->kind == 2 ? entry->deltaX * entry->angularSpeed : entry->directionX);
                        motionZCopy = entry->kind == 2 ? entry->deltaZ * entry->angularSpeed : entry->directionZ;
                        if (((motionXCopy * entry->collisionNormalX) + (motionZCopy * entry->collisionNormalZ)) >= 0.0f) {
                            if (entry->kind == 2) {
                                entry->angularSpeed = (f32) -entry->angularSpeed;
                            } else if (entry->kind == 3) {
                                incomingMotion.x = entry->directionX;
                                incomingMotion.y = 0.0f;
                                incomingMotion.z = entry->directionZ;
                                contactNormal.x = entry->collisionNormalX;
                                contactNormal.y = 0.0f;
                                contactNormal.z = entry->collisionNormalZ;
                                PSVECNormalize(&contactNormal, &contactNormal);
                                C_VECReflect(&incomingMotion, &contactNormal, &reflectedMotion);
                                PSVECScale(&reflectedMotion, &reflectedMotion, PSVECMag(&incomingMotion));
                                entry->directionX = reflectedMotion.x;
                                entry->directionZ = reflectedMotion.z;
                            }
                        }
                        if (((s32) entry->collisionKind == 1) && ((s32) (playerData[playerIndex])->sequenceFrame <= 1) && ((entry->motionState == 0) || (entry->motionState == 3))) {
                            entry->motionState = 3;
                            entry->randomOffset = 0x1E;
                            entry->overlayEnabled = 1;
                        }
                        entry->collisionKind = 0;
                    }
                    entry->randomOffset -= entry->randomOffset > 0;
                    if (lbl_1_data_34[0] != -1) {
                        entry->motionState = 3;
                        entry->overlayEnabled = 0;
                        entry->randomOffset = 0x5A;
                    }
                    if ((s32) entry->randomOffset == 0) {

                        switch ((entry->motionState)) {       /* switch 2; irregular */
                        case 0:                     /* switch 2 */
                            if ((entry->kind < 3) && (entry->parameter != 0) && (entry->motionMode == (s32) (playerData[playerIndex])->sequenceCount) && (lbl_1_bss_18 >= 1) && ((s32) ((lbl_1_data_10[playerIndex] + 1) % 11) != lbl_1_data_20[playerIndex])) {
                                entry->motionState = 1;
                                entry->randomOffset = 0x1E;
                                entry->overlayEnabled = 1;
                                entry->overlayModelIndex = lbl_1_data_10[playerIndex];
                                fn_8002D414(*(lbl_1_bss_A0FC->mdlId + ((0x30 + (entry->overlayModelIndex * 4)) + (playerData[playerIndex])->playerIndex)), 0.0f);

                                lbl_1_data_10[playerIndex]++;

                                lbl_1_data_10[playerIndex] %= 0xB;
                            } else {

                                entry->randomOffset = (s32) (entry->kind == 1 ? 150.0f + (60.0f * ((f32) rand8() / 255.0f)) : (entry->kind == 2 ? 240.0f + (180.0f * ((f32) rand8() / 255.0f)) : 60.0f));
                            }
                            break;
                        case 1:                     /* switch 2 */
                            entry->motionState = 2;
                            entry->randomOffset = 0x58;
                            break;
                        case 2:                     /* switch 2 */
                            entry->motionState = 3;

                            entry->randomOffset = (entry->kind == 1 ? 0 : 0x1E);
                            fn_800269E8(*(lbl_1_bss_A0FC->mdlId + ((0x30 + (entry->overlayModelIndex * 4)) + (playerData[playerIndex])->playerIndex)), (1 << 0));
                            entry->overlayModelIndex = -1;

                            lbl_1_data_20[playerIndex]++;

                            lbl_1_data_20[playerIndex] %= 0xB;
                            break;
                        case 3:                     /* switch 2 */
                            entry->motionState = 0;
                            entry->overlayEnabled = 0;
                            entry->randomOffset = 0x5A;

                            entry->randomOffset = (s32) (entry->kind == 1 ? 30.0f + (120.0f * ((f32) rand8() / 255.0f)) : (entry->kind == 2 ? 180.0f + (180.0f * ((f32) rand8() / 255.0f)) : 60.0f));
                            break;
                        }
                    }
                    if ((entry->overlayEnabled == 1) && (entry->motionState != 3) && (entry->overlayModelIndex != -1)) {
                        if (entry->motionState == 1) {
                            modelScaleDouble = 1.0;
                        } else {
                            modelScaleDouble = 1.75;
                        }
                        modelScale = (f32) modelScaleDouble;
                        scaledModelHandle = *(lbl_1_bss_A0FC->mdlId + ((0x30 + (entry->overlayModelIndex * 4)) + (playerData[playerIndex])->playerIndex));
                        fn_80026A44((s16) scaledModelHandle, (1 << 0));
                        fn_800268B0((s16) scaledModelHandle, modelScale, modelScale, modelScale);
                        fn_80026760((s16) scaledModelHandle, entry->startX, 75.0f, entry->startZ);
                    } else {
                        fn_800269E8(*(lbl_1_bss_A0FC->mdlId + ((0x30 + (entry->overlayModelIndex * 4)) + (playerData[playerIndex])->playerIndex)), (1 << 0));
                    }
                    if ((lbl_1_bss_A0E8 <= 5) && (entry->overlayEnabled == 0) && (entry->scale >= 1.0f)) {
                        if (entry->kind == 2) {
                            entry->speed += entry->angularSpeed;
                            if ((entry->speed < 0.0f) || (entry->speed > 1.0f)) {

                                entry->speed = (f32) (entry->speed < 0.0f ? 0 : 1);
                                entry->angularSpeed = (f32) -entry->angularSpeed;
                            }
                            entry->startX = (f32) (entry->targetX + (entry->deltaX * entry->speed));
                            entry->startZ = (f32) (entry->targetZ + (entry->deltaZ * entry->speed));
                        } else if ((entry->directionX != 0.0f) || (entry->directionZ != 0.0f)) {
                            intersectionX = 0.0f;
                            intersectionZ = 0.0f;
                            boundaryHit = 0;
                            collisionShapeIndex = entry->motionMode == 7;
                            copyAndEdgeStartIndex = -1;
                            {
                                f32 collisionZ[2][5] = { { -385.5f, -188.0f, 188.0f, 385.5f, 0.0f }, { 1097.7833f, 593.9697f, 328.09753f, 552.60394f, 643.11365f } };
                                f32 collisionX[2][5] = { { 1167.0f, 652.0f, 652.0f, 1167.0f, 909.5f }, { -552.60394f, -328.09753f, -593.9697f, -1097.7833f, -643.11365f } };
                                /* Initialized bytes; purpose unknown. */
                                static const u8 lbl_1_rodata_BC4[0x24] = {
                                    0x00, 0x00, 0x00, 0x00, 0x3E, 0x34, 0x00, 0x00, 0x3E, 0xB4, 0x00, 0x00, 0x3F, 0x07, 0x00, 0x00, 0x3F, 0x34, 0x00, 0x00, 0x3F, 0x61, 0x00, 0x00, 0x3F, 0x7F, 0x00, 0x00, 0x3F, 0x48, 0x00, 0x00, 0x3E, 0xC8, 0x00, 0x00
                                };
                                minimumCross = 2.6843546e8f;
                                cornerIndex = 0;
                                while (cornerIndex < 4) {
                                    cornerDeltaX[cornerIndex] = collisionZ[collisionShapeIndex][cornerIndex] - entry->startX;
                                    cornerDeltaZ[cornerIndex] = collisionX[collisionShapeIndex][cornerIndex] - entry->startZ;
                                    cornerIndex++;
                                }
                                cornerIndex = 0;
                                while (cornerIndex < 4) {
                                    cornerCross = (cornerDeltaX[cornerIndex] * cornerDeltaZ[(cornerIndex + 1) % 4]) -
                                        (cornerDeltaX[(cornerIndex + 1) % 4] * cornerDeltaZ[cornerIndex]);
                                    if (cornerCross < minimumCross) {
                                        copyAndEdgeStartIndex = cornerIndex;
                                        minimumCross = cornerCross;
                                    }
                                    cornerIndex++;
                                }
                                if (minimumCross < 0.0f) {
                                    copyAndEdgeEndIndex = ((s32) (copyAndEdgeStartIndex + 1) % 4);
                                    intersectionX = intersectionZ = 0.0f;
                                    for (cornerIndex = 0; cornerIndex < 4; cornerIndex++) {
                                        if (fn_1_3210(&intersectionX, &intersectionZ, &intersectionCross,
                                            entry->startX, entry->startZ,
                                            collisionZ[collisionShapeIndex][4], collisionX[collisionShapeIndex][4],
                                            collisionZ[collisionShapeIndex][copyAndEdgeStartIndex], collisionX[collisionShapeIndex][copyAndEdgeStartIndex],
                                            collisionZ[collisionShapeIndex][copyAndEdgeEndIndex], collisionX[collisionShapeIndex][copyAndEdgeEndIndex])) {
                                            boundaryMotion.x = entry->directionX;
                                            boundaryMotion.y = 0.0f;
                                            boundaryMotion.z = entry->directionZ;
                                            boundaryNormal.x = collisionX[collisionShapeIndex][copyAndEdgeStartIndex] - collisionX[collisionShapeIndex][copyAndEdgeEndIndex];
                                            boundaryNormal.y = 0.0f;
                                            boundaryNormal.z = -collisionZ[collisionShapeIndex][copyAndEdgeStartIndex] + collisionZ[collisionShapeIndex][copyAndEdgeEndIndex];
                                            PSVECNormalize(&boundaryNormal, &boundaryNormal);
                                            C_VECReflect(&boundaryMotion, &boundaryNormal, &boundaryReflectedMotion);
                                            boundaryMotionMagnitude = PSVECMag(&boundaryMotion);
                                            PSVECScale(&boundaryReflectedMotion, &boundaryReflectedMotion, boundaryMotionMagnitude);
                                            entry->directionX = boundaryReflectedMotion.x;
                                            entry->directionZ = boundaryReflectedMotion.z;
                                            entry->startX = (f32) (intersectionX + entry->directionX);
                                            entry->startZ = (f32) (intersectionZ + entry->directionZ);
                                            boundaryHit = 1;
                                            break;
                                        } else {
                                            copyAndEdgeStartIndex++;
                                            copyAndEdgeStartIndex %= 4;
                                            copyAndEdgeEndIndex++;
                                            copyAndEdgeEndIndex %= 4;
                                        }
                                    }
                                }
                            }
                            if (boundaryHit == 0) {
                                entry->startX += entry->directionX;
                                entry->startZ += entry->directionZ;
                            }
                            entry->previousX = (f32) entry->startX;
                            entry->previousZ = (f32) entry->startZ;
                        }
                    }
                    if ((lbl_1_bss_A0E8 <= 5) && (entry->scale < 1.0f)) {
                        entry->scale += 0.016666668f;

                        entry->scale = (entry->scale > 1.0f ? 1.0f : entry->scale);
                        if (entry->scale < 0.0f) {
                            entryBlend = entry->scale < -1.0f ? 0.0f : 1.0f + entry->scale;
                            entryOuterBlend = 1.0f - entryBlend;
                            entryPositionX = (f32) (675.0 * ((f64) entryBlend * sin((M_PI * (f64) ((entry->motionMode * 0x2D) + 0xB4)) / 180.0)));
                            entryPositionY = 200.0f;
                            entryPositionZ = (f32) (675.0 * ((f64) entryBlend * cos((M_PI * (f64) ((entry->motionMode * 0x2D) + 0xB4)) / 180.0)));
                        } else {
                            entryBlend = entry->scale > 1.0f ? 1.0f : entry->scale;
                            entryOuterBlend = 1.0f - entryBlend;
                            entryPositionX = (f32) ((f64) (entryBlend * entry->startX) + ((f64) entryOuterBlend * (675.0 * sin((M_PI * (f64) ((entry->motionMode * 0x2D) + 0xB4)) / 180.0))));
                            entryPositionY = (200.0f * entryOuterBlend) + (75.0f * entryBlend);
                            entryPositionZ = (f32) ((f64) (entryBlend * entry->startZ) + ((f64) entryOuterBlend * (675.0 * cos((M_PI * (f64) ((entry->motionMode * 0x2D) + 0xB4)) / 180.0))));
                        }
                        fn_80026760(object->mdlId[entryModelIndex], entryPositionX, entryPositionY, entryPositionZ);
                    } else {
                        fn_80026760(object->mdlId[entryModelIndex], entry->startX, 75.0f, entry->startZ);
                    }
                }
            }
            if ((lbl_1_bss_20 != 0) && (conditionSentinel != -1)) {
                if (lbl_1_data_34[0] != 4) {
                    ASSERT(FALSE);
                }
            }
        }
    }
    if (lbl_1_data_114[0] == -1) {
        index = 0;
        while (index < 8) {
            lbl_1_data_134[index] = (s32) fn_8002D4E0(object->mdlId[(index + 0x1C)]);
            lbl_1_data_114[index] = (s32) ((70.0f + (210.0f * ((f32) rand8() / 255.0f))) - (f32) lbl_1_data_134[index]);
            index++;
        }
    }
    index = 0;
    while (index < 8) {

        lbl_1_data_114[index] -= lbl_1_data_114[index] > 0;

        lbl_1_data_134[index] += lbl_1_data_134[index] < 0x3C;
        if ((s32) lbl_1_data_114[index] == 0) {
            lbl_1_data_134[index] = 0;
            lbl_1_data_114[index] = (s32) (90.0f + (145.0f * ((f32) rand8() / 255.0f)));
        }
        fn_8002D414(object->mdlId[(index + 0x24)], (f32) lbl_1_data_134[index]);
        fn_8002D414(object->mdlId[(index + 0x1C)], (f32) lbl_1_data_134[index]);
        index++;
    }
}


#undef fn_800288B4

/* The contents between position and offset are unknown. */
typedef struct Position {
    HuVecF position;
    unsigned char unk_C[12]; /* No code names this field. */
    HuVecF offset;
} Position;

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern s8 lbl_1_bss_218;

extern s8 lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

void fn_1_B1C4(s16 mode, s16 frameNo)
{
}

void fn_1_B1C8(s16 mode, s16 frameNo)
{
}

void fn_1_B1CC(s16 mode, s16 frameNo)
{
}

void fn_1_B1D0(s16 mode, s16 frameNo)
{
}

void fn_1_B1D4(s16 mode, s16 frameNo)
{
}

void fn_1_B1D8(s16 mode, s16 frameNo)
{
}

void fn_1_B1DC(s16 mode, s16 frameNo);

void fn_1_B1DC(s16 mode, s16 frameNo)
{
    if (frameNo >= 60) {
        fn_80071A9C();
    }
}

extern s32 lbl_1_data_40;

extern MGTIMER *lbl_1_data_44[4];

extern s32 lbl_1_data_58;

extern s32 lbl_1_data_5C;

extern u8 lbl_1_data_88[14];

typedef struct M606SequenceTable { s32 values[16]; } M606SequenceTable;

typedef struct M606FourValueTable { s32 values[4]; } M606FourValueTable;





static inline void DebugTimeValue(f32 value) { }

void fn_1_FF2C(s16 mode, s16 frameNo);
void fn_1_10078(s16 mode, s16 frameNo);
s32 lbl_1_data_0[4] = { 0, 0, 0, 0 };
s32 lbl_1_data_10[4] = { 0, 0, 0, 0 };
s32 lbl_1_data_20[5] = { 0, 0, 0, 0, -65536 };
s32 lbl_1_data_34[2] = { -1, -1 };
s32 lbl_1_data_3C = -1;
s32 lbl_1_data_40 = -1;
MGTIMER *lbl_1_data_44[4] = { 0, 0, 0, 0 };
s32 lbl_1_data_54 = 1;
s32 lbl_1_data_58 = 300;
s32 lbl_1_data_5C = 3600;
MGSEQ_PARAM lbl_1_data_60 = { 0, 0, fn_1_FF2C, fn_1_B1C4, fn_1_B1C8, fn_1_B1CC, fn_1_B1D0, fn_1_B1D4, fn_1_B1D8, fn_1_B1DC, fn_1_10078 };
u8 lbl_1_data_88[14] = { 0, 0, 24, 90, 30, 0, 18, 42, 96, 30, 42, 30, 30, 30 };
char lbl_1_data_96[] = "%f,%f,%f";
char lbl_1_data_9F[] = "%f";
HuVecF lbl_1_data_A4 = { 0.0f, 5000.0f, 0.0f };
HuVecF lbl_1_data_B0 = { 0.2f, -1.0f, -0.5f };
HuVecF lbl_1_data_BC = { 0.0f, -1.0f, -1.0f };
GXColor colorC8 = { 255, 255, 255, 255 };
GXColor lbl_1_data_CC = { 192, 192, 255, 255 };
GXColor colorD0 = { 118, 128, 118, 255 };
f32 lbl_1_data_D4[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
f32 lbl_1_data_E4[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
s32 lbl_1_data_F4[4] = { 0, 0, 0, 0 };
s32 lbl_1_data_104[4] = { 0, 0, 0, 0 };
s32 lbl_1_data_114[8] = { -1, 0, 0, 0, 0, 0, 0, 0 };
s32 lbl_1_data_134[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

/* Updates player models and obstacle motion, then handles the winner presentation. */
void fn_1_B20C(OMOBJ *obj) {
    M606PlayerWork *playerWork[4];

    s32 soundHandle;
    u32 characterDataNumber;


    HU3D_MODEL *playerModel;
    s32 motionDataNumber;
    M606MotionEntry *otherMotion;
    OMOBJ *winnerObject;

    f32 orbitAngle;
    f32 overlayAngle;

    s32 overlayModelIndex;
    s32 otherMotionIndex;
    s32 motionIndexOrOrbitSlot;

    s32 loopIndex;

    u32 frameNo;
    M606MotionEntry *motion;
    void *motionData;
    HU3D_MODEL *winnerModel;

    frameNo = fn_80071DE4();
    {
        s32 sequenceValues[16] = {
            -300, -100, 100, 300, 0, -300, 0, 300,
            -150, 150, -150, 150, -300, 0, 300, 0
        };
        s32 modeValues[16] = {
            0, 0, 0, 0, 300, 0, 0, 0,
            150, 150, -150, -150, 150, 150, 150, -150
        };
    }
    lbl_1_bss_A0E8 = fn_80071A8C();
    loopIndex = 0;
    while (loopIndex < 4) {
        (&playerWork[0])[loopIndex] = lbl_1_bss_A100[loopIndex].object->data;
        loopIndex++;
    }
    if (((s32) lbl_1_data_34[0] == -1) || (((s32) lbl_1_data_34[0] != 4) && (lbl_1_bss_2C == 0)) || (((s32) lbl_1_data_34[0] == 4) && (lbl_1_bss_A0E8 < 8))) {
        Point3d playerPositions[4];
        fn_800856A8();
        {
        s32 playerState[4] = {0, 0, 0, 0};
        loopIndex = 0;
        while (loopIndex < 4) {
            playerModel = &lbl_802C01C8[*lbl_1_bss_A100[loopIndex].object->mdlId];
            fn_80086BC8(lbl_1_bss_A0EC[loopIndex]->actor, &playerPositions[loopIndex]);
            lbl_1_bss_A100[loopIndex].object->trans.x = playerPositions[loopIndex].x;
            lbl_1_bss_A100[loopIndex].object->trans.y = playerPositions[loopIndex].y;
            lbl_1_bss_A100[loopIndex].object->trans.z = playerPositions[loopIndex].z;
            lbl_1_bss_A100[loopIndex].object->rot.y = lbl_1_bss_A0EC[loopIndex]->actor->rotY;
            lbl_1_bss_A100[loopIndex].object->scale.x = playerModel->scale.x;
            lbl_1_bss_A100[loopIndex].object->scale.y = playerModel->scale.y;
            lbl_1_bss_A100[loopIndex].object->scale.z = playerModel->scale.z;
            playerWork[loopIndex]->playerPositionX = playerPositions[loopIndex].x;
            playerWork[loopIndex]->playerPositionZ = playerPositions[loopIndex].z;
            if (((s32) ((&playerWork[0])[loopIndex])->sequenceCount == 0) && ((s32) ((&playerWork[0])[loopIndex])->childIndex > 0) && (playerPositions[loopIndex].x <= 0.0f)) {
                playerState[loopIndex] = 1;
                if ((lbl_1_bss_14 != 0) && ((s32) ((&playerWork[0])[loopIndex])->finished == 0)) {
                    if (lbl_1_data_44[loopIndex]->stopF == 0) {
                        lbl_1_data_44[loopIndex]->stopF = 1;
                        lbl_1_data_44[loopIndex]->mode = 1;
                    }
                    ((&playerWork[0])[loopIndex])->finished = 1;
                    if ((s32) ((&playerWork[0])[loopIndex])->padIndex != -1) {
                        lbl_1_bss_30--;
                    }
            if (((s32) (playerWork[0])->finished == 0) || ((s32) (playerWork[1])->finished == 0) || ((s32) (playerWork[2])->finished == 0) || ((s32) (playerWork[3])->finished == 0)) {
                        HuAudFXPlay(MSM_SE_CMN_TIMER_START);
                    }
                }
            }
            if (lbl_1_bss_A0E8 == 5) {
                motionIndexOrOrbitSlot = 0;
                while (motionIndexOrOrbitSlot < 0x13) {
                    motion = &lbl_1_bss_A168[loopIndex][motionIndexOrOrbitSlot];
                    if (motion->kind != 0) {
                        if (fn_1_0(playerPositions[loopIndex].x - (motion)->startX, playerPositions[loopIndex].z - (motion)->startZ) < (50.0f + (f32) ((s32) (((motion)->motionState == 2 && (motion)->overlayEnabled != 0) ? 0x64 : 0x46) / 2))) {
                            (motion)->collisionKind = 1;
                            (motion)->collisionNormalX = playerPositions[loopIndex].x - (motion)->startX;
                            (motion)->collisionNormalZ = playerPositions[loopIndex].z - (motion)->startZ;
                            ((&playerWork[0])[loopIndex])->sequenceFrame = (s32) ((&playerWork[0])[loopIndex])->sequenceFrame >= 0x96 ? 0 : ((&playerWork[0])[loopIndex])->sequenceFrame;
                            if ((s32) ((&playerWork[0])[loopIndex])->sequenceFrame == 0) {
                                if (((s32) ((&playerWork[0])[loopIndex])->computerDifficulty != 0) || ((s32) ((&playerWork[0])[loopIndex])->sequenceCount != 2) || (motionIndexOrOrbitSlot != 0xF)) {
                                    ((&playerWork[0])[loopIndex])->currentMode = motionIndexOrOrbitSlot;
                                }
                                if ((s32) lbl_1_bss_A118[loopIndex + 5].actorId < 3) {
                                    lbl_1_bss_A118[loopIndex + 5].actorId++;
                                }
                            }
                            if ((s32) ((&playerWork[0])[loopIndex])->sequenceFrame == 0) {
                                fn_8006A030((s16) ((&playerWork[0])[loopIndex])->configIndex, 0x14, 7, 3);
                            }
                        }
                        otherMotionIndex = 0;
                        while (otherMotionIndex < 0x13) {
                            otherMotion = &lbl_1_bss_A168[loopIndex][otherMotionIndex];
                            if ((otherMotionIndex != motionIndexOrOrbitSlot) && ((s32) (motion)->motionMode == (s32) (otherMotion)->motionMode) && (((((otherMotion)->startX - (motion)->startX) * ((otherMotion)->startX - (motion)->startX)) + (((otherMotion)->startZ - (motion)->startZ) * ((otherMotion)->startZ - (motion)->startZ))) < 4900.0f)) {
                                (motion)->collisionKind = 2;
                                (motion)->collisionNormalX = (f32) ((otherMotion)->startX - (motion)->startX);
                                (motion)->collisionNormalZ = (f32) ((otherMotion)->startZ - (motion)->startZ);
                                (otherMotion)->collisionKind = 1;
                                (otherMotion)->collisionNormalX = (f32) ((motion)->startX - (otherMotion)->startX);
                                (otherMotion)->collisionNormalZ = (f32) ((motion)->startZ - (otherMotion)->startZ);
                            }
                            otherMotionIndex += 1;
                        }
                    }
                    motionIndexOrOrbitSlot += 1;
                }
            }
            loopIndex += 1;
        }
        if ((lbl_1_bss_14 != 0) && (playerWork[0]->finished != 0) && (playerWork[1]->finished != 0) && (playerWork[2]->finished != 0) && (playerWork[3]->finished != 0)) {
            fn_80071A9C();
            loopIndex = 0;
            while (loopIndex < 4) {
                GWMgScoreSet(loopIndex, playerWork[loopIndex]->finished == 1 ? MgTimerValueGet(lbl_1_data_44[loopIndex]) : -1);
                loopIndex += 1;
            }
            lbl_1_data_34[0] = 4;
        } else if ((lbl_1_bss_14 == 0) && (lbl_1_bss_A0E8 == 5) && ((s32) lbl_1_data_34[0] == -1)) {
            loopIndex = 0;
            motionIndexOrOrbitSlot = (s32) rand8() % 4;
            while (loopIndex < 4) {
                if (playerState[(loopIndex + motionIndexOrOrbitSlot) % 4] != 0) {
                    lbl_1_data_34[0] = (loopIndex + motionIndexOrOrbitSlot) % 4;
                    fn_80071A9C();
                    MgTimerModeOffSet(lbl_1_bss_4);
                    fn_80071C18((s16) ((&playerWork[0])[lbl_1_data_34[0]])->character, -1, -1, -1);
                    if ((MgTimerValueGet(lbl_1_bss_4) < lbl_1_data_5C) && ((s32) ((&playerWork[0])[lbl_1_data_34[0]])->padIndex != -1) && (fn_8006A124(65551) == 0)) {
                        lbl_1_bss_28 = 1;
                        lbl_1_data_5C = MgTimerValueGet(lbl_1_bss_4);
                        fn_80071BAC(lbl_1_data_5C);
                    }
                }
                loopIndex += 1;
            }
        }
        }
    } else if (((s32) lbl_1_data_34[0] != -1) && ((s32) lbl_1_data_34[0] != 4) && (lbl_1_bss_2C != 0)) {
        Point3d orbitPosition;
        lbl_1_bss_40[0] += 1;
        loopIndex = 0;
        motionIndexOrOrbitSlot = 0;
        while (loopIndex < 4) {
            if (loopIndex == (s32) lbl_1_data_34[0]) {
                lbl_1_bss_A100[loopIndex].object->trans.x = 0.0f;
                lbl_1_bss_A100[loopIndex].object->trans.y = 150.0f;
                lbl_1_bss_A100[loopIndex].object->trans.z = -900.0f;
                lbl_1_bss_A100[loopIndex].object->rot.y = 0.0f;
            } else {
                orbitAngle = (f32) (((lbl_1_bss_40[0] * 3) % 360) - (motionIndexOrOrbitSlot * 0x2B));
                orbitPosition.x = lbl_1_bss_A100[loopIndex].object->trans.x = (f32) (200.0 * sin((M_PI * (f64) orbitAngle) / 180.0));
                orbitPosition.y = lbl_1_bss_A100[loopIndex].object->trans.y = 0.0f;
                orbitPosition.z = lbl_1_bss_A100[loopIndex].object->trans.z = (f32) ((200.0 * cos((M_PI * (f64) orbitAngle) / 180.0)) - 900.0);
                lbl_1_bss_A100[loopIndex].object->rot.y = 90.0f + orbitAngle;
            }
            omSetTra(lbl_1_bss_A100[loopIndex].object, lbl_1_bss_A100[loopIndex].object->trans.x, lbl_1_bss_A100[loopIndex].object->trans.y, lbl_1_bss_A100[loopIndex].object->trans.z);
            fn_80086B04(lbl_1_bss_A0EC[loopIndex]->actor, &orbitPosition);
            omSetRot(lbl_1_bss_A100[loopIndex].object, lbl_1_bss_A100[loopIndex].object->rot.x, lbl_1_bss_A100[loopIndex].object->rot.y, lbl_1_bss_A100[loopIndex].object->rot.z);
            lbl_1_bss_A0EC[loopIndex]->actor->rotY = lbl_1_bss_A100[loopIndex].object->rot.y;
            fn_80026A44(*lbl_1_bss_A100[loopIndex].object->mdlId, (1 << 0));
            motionIndexOrOrbitSlot += loopIndex != lbl_1_data_34[0];
            loopIndex += 1;
        }
        fn_800269E8(*lbl_1_bss_A100[lbl_1_data_34[0]].object->mdlId, (1 << 0));
        loopIndex = 0;
        while (loopIndex < 2) {
            overlayAngle = (f32) ((f64) (((lbl_1_bss_40[0] * 3) % 360) - 0x91) + (3.0 * sin((M_PI * (f64) ((lbl_1_bss_40[0] * 5) % 360)) / 180.0)));
            overlayModelIndex = loopIndex + 0x7E;
            fn_80026760(lbl_1_bss_A0FC->mdlId[overlayModelIndex],
                (f32) ((200.0 * sin((M_PI * (f64) overlayAngle) / 180.0)) + (60.0 * sin((M_PI * (f64) (overlayAngle + (f32) ((loopIndex * 0xE1) + 0x2D))) / 180.0))),
                (f32) (75.0 + (15.0 * cos((M_PI * (f64) ((lbl_1_bss_40[0] * 4) % 360)) / 180.0))),
                (f32) (((200.0 * cos((M_PI * (f64) overlayAngle) / 180.0)) + (60.0 * cos((M_PI * (f64) (overlayAngle + (f32) ((loopIndex * 0xE1) + 0x2D))) / 180.0))) - 900.0));
            fn_80026808(lbl_1_bss_A0FC->mdlId[overlayModelIndex], 0.0f, 90.0f + overlayAngle, 0.0f);
            fn_80026A44(lbl_1_bss_A0FC->mdlId[overlayModelIndex], (1 << 0));
            loopIndex += 1;
        }
    } else if ((lbl_1_bss_14 == 0) && ((s32) lbl_1_data_34[0] == 4) && (lbl_1_bss_A0E8 >= 8)) {
        loopIndex = 0;
        while (loopIndex < 4) {
            lbl_1_bss_A100[loopIndex].object->rot.y = 0.0f;
            omSetRot(lbl_1_bss_A100[loopIndex].object, lbl_1_bss_A100[loopIndex].object->rot.x, lbl_1_bss_A100[loopIndex].object->rot.y, lbl_1_bss_A100[loopIndex].object->rot.z);
            lbl_1_bss_A0EC[loopIndex]->actor->rotY = lbl_1_bss_A100[loopIndex].object->rot.y;
            fn_80026A44(*lbl_1_bss_A100[loopIndex].object->mdlId, (1 << 0));
            loopIndex += 1;
        }
    }
    loopIndex = 0;
    while (loopIndex < 0xA) {
        s32 soundIds[10] = {
            0x645, 0x646, 0x647, 0x648, 0x649,
            0x64A, 0x64B, 0x64C, 0x64D, 0x64E
        };
        if ((s32) lbl_1_bss_A118[loopIndex].actorId > 3) {
            lbl_1_bss_A118[loopIndex].actorId = 3;
        }
        if ((s32) lbl_1_bss_A118[loopIndex].actorId > 0) {
            if (soundIds[loopIndex] != -1) {

                soundHandle = HuAudFXPlay(soundIds[loopIndex]);
                lbl_1_bss_A118[loopIndex].modelId = soundHandle;
            }
            lbl_1_bss_A118[loopIndex].actorId--;
        }
        loopIndex += 1;
    }
    if (lbl_1_data_3C == 0x50) {
        HuAudFXFadeOut(lbl_1_bss_A118->modelId, 0x190);
    }
    if ((lbl_1_bss_A0E8 == 2) && ((s32) frameNo == 0xD2)) {
        HuAudFXFadeOut(lbl_1_bss_A118[9].modelId, 0x1F4);
    }
    if (lbl_1_bss_14 != 0) {
        loopIndex = 0;
        while (loopIndex < 4) {
            if ((playerWork[loopIndex]->finished == 0) && lbl_1_data_44[loopIndex] && (MgTimerDoneCheck(lbl_1_data_44[loopIndex]) != 0)) {
                playerWork[loopIndex]->finished = 2;
            }
            loopIndex += 1;
        }
    } else if ((lbl_1_bss_14 == 0) && (lbl_1_bss_A0E8 == 5) && ((s32) lbl_1_data_34[0] == -1) && lbl_1_bss_4 && (MgTimerDoneCheck(lbl_1_bss_4) != 0)) {
        fn_80071A9C();
        MgTimerModeOffSet(lbl_1_bss_4);
        lbl_1_data_34[0] = 4;
        fn_80071C18(-1, -1, -1, -1);
        if (lbl_1_bss_8.timer) {
            MgTimerDispOff(lbl_1_bss_8.timer);
            lbl_1_bss_8.timer = NULL;
        }
    }
    frameNo = fn_80071DE4();
    lbl_1_bss_A0E8 = fn_80071A8C();
    lbl_1_bss_3C += (lbl_1_bss_A0E8 == 6 || lbl_1_bss_A0E8 == 7) || lbl_1_bss_A0E8 == 8;
    switch (lbl_1_bss_A0E8) {
    case 2:
        if ((s32) frameNo == 1) {
            if ((s32) lbl_1_bss_A118->actorId < 3) {
                lbl_1_bss_A118->actorId += 1;
            }
            if ((s32) lbl_1_bss_A118[9].actorId < 3) {
                lbl_1_bss_A118[9].actorId += 1;
            }
        }
        if ((lbl_1_bss_14 == 0) && (lbl_1_bss_4 == 0)) {
            lbl_1_bss_4 = MgTimerCreate(1);
            MgTimerParamSet(lbl_1_bss_4, 0, lbl_1_data_58 * 0x3C, lbl_1_data_5C);
            MgTimerPosSet(lbl_1_bss_4, 292.0f, 70.0f);
            MgTimerRecordDispOn(lbl_1_bss_4);
            return;
        }
    case 11:
    default:
        break;
    case 3:
        if (((s32) frameNo == 0) && (lbl_1_bss_14 != 0)) {
            loopIndex = 0;
            while (loopIndex < 4) {
                lbl_1_data_44[loopIndex] = MgTimerCreate(2);
                MgTimerPosSet(lbl_1_data_44[loopIndex], (f32) (loopIndex % 2 == 0 ? 0x4C : 0x204), (f32) (loopIndex / 2 == 0 ? 0xE4 : 0x1AE));
                MgTimerColorSet(lbl_1_data_44[loopIndex], 0xFFU, 0xFFU, 0xFFU);
                MgTimerParamSet(lbl_1_data_44[loopIndex], 0, 0x1518, 0);
                loopIndex += 1;
            }
        }
        if ((lbl_1_data_40 == -1) && ((s32) frameNo == 0xA)) {
            lbl_1_data_40 = HuAudBGMPlay(MSM_STREAM_PUMPED_UP);
            return;
        }
        break;
    case 5:
        if ((s32) frameNo == 0) {
            if (lbl_1_bss_14 == 0) {
                MgTimerModeOnSet(lbl_1_bss_4, 0);
            } else {
                loopIndex = 0;
                while (loopIndex < 4) {
                    MgTimerModeOnSet(lbl_1_data_44[loopIndex], 2);
                    loopIndex += 1;
                }
            }
            HuAudFXPlay(MSM_SE_CMN_TIMER_START);
        }
        if ((lbl_1_bss_14 == 0) && lbl_1_bss_4 && (lbl_1_bss_8.timer == 0) && ((s32) (((lbl_1_data_58 - 0x1E) * 0x3C) - 1) == MgTimerValueGet(lbl_1_bss_4))) {
            lbl_1_bss_8.timer = MgTimerCreate(0);
            MgTimerParamSet(lbl_1_bss_8.timer, 0x708, 0, 0);
            MgTimerPosSet(lbl_1_bss_8.timer, 288.0f, 420.0f);
            MgTimerRecordDispOn(lbl_1_bss_8.timer);
            MgTimerModeOnSet(lbl_1_bss_8.timer, 0);
            return;
        }
        break;
    case 6:
        if ((lbl_1_data_40 != -1) && ((s32) (GameMesStatGet(fn_80071DF4()) & 0x10) != 0)) {
            HuAudSStreamFadeOut(lbl_1_data_40, 0x64);
            lbl_1_data_40 = -1;
        }
        if ((lbl_1_bss_14 == 0) && lbl_1_bss_8.timer) {
            MgTimerDispOff(lbl_1_bss_8.timer);
        }
        if ((s32) lbl_1_data_34[0] == 4) {
            break;
        }
    case 7:
            if ((s32) lbl_1_data_34[0] == 4) {
                loopIndex = 0;
                while (loopIndex < 0x2C) {
                    fn_800269E8(lbl_1_bss_A0FC->mdlId[loopIndex + 0x30], (1 << 0));
                    loopIndex += 1;
                }
                if ((s32) frameNo >= 0x3C) {
                    fn_80071A9C();
                    return;
                }
            } else if ((s32) lbl_1_data_34[0] != 4) {
                if (lbl_1_bss_2C == 0) {
                    if (lbl_1_bss_3C == 1) {
                        fn_80071B08();
                    }
                    if (lbl_1_bss_3C == 0x78) {
                        WipeCreate(2, 6, 0x3C);
                    }
                    if (lbl_1_bss_3C >= 0xB4) {
                        lbl_1_bss_2C = 1;
                        lbl_1_bss_3C = 0;
                        if (lbl_1_bss_28 != 0) {
                            fn_8003C80C(0, lbl_1_data_5C);
                            if (lbl_1_bss_14 == 0) {
                                MgTimerRecordSet(lbl_1_bss_4, -1);
                            }
                        }
                    }
                }
                if (lbl_1_bss_2C != 0) {
                    if (lbl_1_bss_3C == 0) {
                        winnerObject = lbl_1_bss_A100[lbl_1_data_34[0]].object;
                        characterDataNumber = lbl_8022E718[((&playerWork[0])[lbl_1_data_34[0]])->character][0];
                        WipeCreate(1, 6, 0x3C);
                        winnerObject->mdlId[1] = fn_80025A04(fn_80008DD0((s32) characterDataNumber, 0x10000000, 2));
                        loopIndex = 0;
                        while (loopIndex < 2) {
                            motionDataNumber = (loopIndex == 0 ? DATANUM(0x930000, 0) : DATANUM(0x930000, 38)) & 0xFFFF;
                            motionData = fn_8005607C(motionDataNumber | (&lbl_8022E718[((&playerWork[0])[lbl_1_data_34[0]])->character][0])[4], 0x10000000, 2);
                            if (motionData == 0) {
                                motionData = fn_80008DD0(motionDataNumber | (&lbl_8022E718[((&playerWork[0])[lbl_1_data_34[0]])->character][0])[4], 0x10000000, 2);
                            }
                            winnerObject->mtnId[loopIndex == 0 ? 0 : 1] = fn_80030458(winnerObject->mdlId[1], motionData);
                            loopIndex += 1;
                        }
                        fn_8002C95C(winnerObject->mdlId[1], *winnerObject->mtnId);
                        fn_800269E8(winnerObject->mdlId[1], ((1 << 30) | (1 << 0)));
                        fn_80026760(winnerObject->mdlId[1], 0.0f, 150.0f, -900.0f);
                        ((&playerWork[0])[lbl_1_data_34[0]])->secondLightId = fn_1_3144(winnerObject->mdlId[1]);
                        if (lbl_1_bss_20 != 0) {
                            fn_800269E8(winnerObject->mdlId[1], 0x10000000U);
                            fn_80028E48(winnerObject->mdlId[1], (s16) ((&playerWork[0])[lbl_1_data_34[0]])->secondLightId, colorD0.r, colorD0.g, colorD0.b, 0xFFU);
                            fn_800273EC(winnerObject->mdlId[1], 0.3f, 0.25f, 0.375f);
                        }
                        fn_80029E44(winnerObject->mdlId[1], (s16) lbl_1_bss_A0DC);
                        lbl_1_bss_18 = 3;
                        Hu3DCameraPerspectiveSet(1, 50.0f, 20.0f, 25000.0f, 1.2f);
                        fn_80026B64(lbl_1_bss_A0FC->mdlId[0], 1U);
                        fn_80026A44(lbl_1_bss_A0FC->mdlId[0x7D], (1 << 0));
                        fn_80026A44(lbl_1_bss_A0FC->mdlId[0], (1 << 0));
                        fn_80026A44(lbl_1_bss_A0FC->mdlId[2], (1 << 0));
                        fn_800269E8(lbl_1_bss_A0FC->mdlId[3], (1 << 0));
                        fn_800269E8(lbl_1_bss_A0FC->mdlId[6], (1 << 0));
                        loopIndex = 0;
                        while (loopIndex < 8) {
                            fn_80026A44(lbl_1_bss_A0FC->mdlId[loopIndex + 0xC], ((1 << 30) | (1 << 1)));
                            fn_800269E8(lbl_1_bss_A0FC->mdlId[loopIndex + 0xC], ((1 << 30) | (1 << 0)));
                            loopIndex += 1;
                        }
                        loopIndex = 0;
                        while (loopIndex < 4) {
                            fn_80026B64(*lbl_1_bss_A100[loopIndex].object->mdlId, 1U);
                            if (loopIndex == 0) {
    Hu3DCameraViewportSet(lbl_1_rodata_30[loopIndex], 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_rodata_30[loopIndex], 0U, 0U, 0x280U, 0x1E0U);
                            } else {
    Hu3DCameraViewportSet(lbl_1_rodata_30[loopIndex], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(lbl_1_rodata_30[loopIndex], 0U, 0U, 0U, 0U);
                            }
                            lbl_80266BD8[loopIndex] = 675.0f;
                            lbl_80266B18[loopIndex].x = 0.0f;
                            lbl_80266B18[loopIndex].y = 235.0f;
                            lbl_80266B18[loopIndex].z = -900.0f;
                            lbl_80266A58[loopIndex].x = 0.0f;
                            lbl_80266A58[loopIndex].y = 0.0f;
                            lbl_80266A58[loopIndex].z = 0.0f;
                            lbl_80266B18[loopIndex].x = 0.0f;
                            lbl_80266B18[loopIndex].y = 235.0f;
                            lbl_80266B18[loopIndex].z = -900.0f;
                            lbl_80266A58[loopIndex].x = -4.0f;
                            lbl_80266A58[loopIndex].y = 0.0f;
                            lbl_80266A58[loopIndex].z = 0.0f;
                            fn_8005C2C8(lbl_1_bss_A0EC[loopIndex]->charNo, lbl_1_bss_A0EC[loopIndex]->omObj->mtnId[(loopIndex == (s32) lbl_1_data_34[0] ? 0 : 7)], 0.0f, (f32) (loopIndex == (s32) lbl_1_data_34[0] ? 0 : rand8() % 22), ((1 << 30) | (1 << 0)));
                            fn_800269E8(lbl_1_bss_A0FC->mdlId[loopIndex + 0x2C], (1 << 0));
                            loopIndex += 1;
                        }
                        loopIndex = 0;
                        while (loopIndex < 8) {
                            fn_800269E8(lbl_1_bss_A0FC->mdlId[loopIndex + 0x24], (1 << 0));
                            fn_80026A44(lbl_1_bss_A0FC->mdlId[loopIndex + 0x1C], (1 << 0));
                            fn_80026B64(lbl_1_bss_A0FC->mdlId[loopIndex + 0x1C], 1U);
                            fn_80026B64(lbl_1_bss_A0FC->mdlId[loopIndex + 0x14], 1U);
                            fn_800269E8(lbl_1_bss_A0FC->mdlId[loopIndex + 0x14], (1 << 0));
                            fn_80026B64(lbl_1_bss_A0FC->mdlId[loopIndex + 0xC], 1U);
                            loopIndex += 1;
                        }
                        loopIndex = 0;
                        while (loopIndex < 4) {
                            motionIndexOrOrbitSlot = 0;
                            while (motionIndexOrOrbitSlot < 0x13) {
                                lbl_1_bss_A168[loopIndex][motionIndexOrOrbitSlot].kind = 0;
                                fn_800269E8(lbl_1_bss_A0FC->mdlId[motionIndexOrOrbitSlot + ((loopIndex * 0x13) + 0x7E)], (1 << 0));
                                motionIndexOrOrbitSlot += 1;
                            }
                            loopIndex += 1;
                        }
                        loopIndex = 0;
                        while (loopIndex < 0x2C) {
                            fn_800269E8(lbl_1_bss_A0FC->mdlId[loopIndex + 0x30], (1 << 0));
                            loopIndex += 1;
                        }
                    }
                    if ((lbl_1_bss_3C >= 0x3C) && (lbl_1_bss_28 == 0)) {
                        fn_80071A9C();
                        return;
                    }
                }
            }
        break;
    case 8:
        if ((s32) frameNo == 0) {
            if ((s32) lbl_1_data_34[0] != 4) {
                GWMgCoinBonusSet(((&playerWork[0])[lbl_1_data_34[0]])->configIndex, 0xA);
                fn_8002CCC0(lbl_1_bss_A100[lbl_1_data_34[0]].object->mdlId[1], lbl_1_bss_A100[lbl_1_data_34[0]].object->mtnId[1], 0.0f, 0.0f, 0U);
            } else if (lbl_1_bss_14 == 0) {
                loopIndex = 0;
                while (loopIndex < 4) {
                    fn_8005C2C8(lbl_1_bss_A0EC[loopIndex]->charNo, lbl_1_bss_A0EC[loopIndex]->omObj->mtnId[6], 0.0f, 0.0f, 0U);
                    loopIndex += 1;
                }
            }
        }
        if ((s32) lbl_1_data_34[0] != 4) {
            winnerModel = &lbl_802C01C8[lbl_1_bss_A100[lbl_1_data_34[0]].object->mdlId[1]];
            DebugTimeValue(winnerModel->motWork.time);
            if (((lbl_1_bss_A100[lbl_1_data_34[0]].object->mtnId[1] != 0) && ((s32) winnerModel->motWork.time == (s32) lbl_1_data_88[((&playerWork[0])[lbl_1_data_34[0]])->character])) != 0) {
                CharFXPlay((s16) ((&playerWork[0])[lbl_1_data_34[0]])->character, 0x241);
            }
        }
        if ((lbl_1_bss_14 != 0) && ((s32) lbl_1_data_34[0] == 4) && (GameMesStatGet(fn_80071DF4()) == 0)) {
            fn_80071A9C();
        }
        break;
    }
}


typedef struct PlayerOrder {
    s32 players[4];
} PlayerOrder;

extern unsigned int lbl_1_data_184[];













extern void fn_1_35B8(OMOBJ *obj);

typedef struct M606AssetTable {
    s32 values[2][8];
} M606AssetTable;












/* Sets up the stage models and callbacks used by the actor object. */
unsigned int lbl_1_data_184[] = { 0x930000, 0x930001, 0x930002, 0x930003, 0x930004, 0x930026, 0x930028, 0x930023, 0x93000A, 0x930009, 0x93001E, 0x930017, 0x930011, 0x930012, 0x930016, 0 };

void fn_1_DEBC(OMOBJ *obj)
{
    HU3D_MODELID collisionModels[16];
    s32 actorModels[4];
    s32 index;
    s32 subIndex;
    s32 modelIndex;
    for (index = 0; index < 4; index++)
    {
        actorModels[index] = lbl_1_bss_A100[index].actor->modelId;
    }
    modelIndex = 0;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_20 != 0 ? DATANUM(0x410000, 8) : DATANUM(0x410000, 1), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelIndex], 0);
    fn_80026760(obj->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
    modelIndex = 1;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_20 != 0 ? DATANUM(0x410000, 7) : DATANUM(0x410000, 0), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelIndex], 0);
    fn_80026760(obj->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
    modelIndex = 4;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x410000, 15), 0x10000000, 2));
    collisionModels[0] = obj->mdlId[modelIndex];
    fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
    modelIndex = 5;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x410000, 14), 0x10000000, 2));
    fn_80026760(obj->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
    collisionModels[1] = obj->mdlId[modelIndex];
    fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
    modelIndex = 0x5C;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x410000, 16), 0x10000000, 2));
    fn_80026760(obj->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
    collisionModels[2] = obj->mdlId[modelIndex];
    fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
    fn_800804E8(collisionModels, 3, 0x80);
    Hu3DBGColorSet(0U, 0U, 0U);
    modelIndex = 2;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_20 != 0 ? DATANUM(0x410000, 12) : DATANUM(0x410000, 5), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelIndex], 1);
    modelIndex = 3;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_20 != 0 ? DATANUM(0x410000, 24) : DATANUM(0x410000, 24), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelIndex], 0);
    fn_80026760(obj->mdlId[modelIndex], 0.0f, 0.0f, 0.0f);
    modelIndex = 0x7D;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_20 != 0 ? DATANUM(0x410000, 9) : DATANUM(0x410000, 2), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelIndex], 0);
    fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
    fn_80026B64(obj->mdlId[modelIndex], 1U);
    fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
    modelIndex = 6;
    obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x410000, 6), 0x10000000, 2));
    fn_80026B8C(obj->mdlId[modelIndex], 0);
    fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
    for (index = 0; index < 8; index++)
    {
        s32 stageAssets[2][8] = { { DATANUM(0x410000, 25), DATANUM(0x410000, 26), DATANUM(0x410000, 27), DATANUM(0x410000, 28), DATANUM(0x410000, 29), DATANUM(0x410000, 30), DATANUM(0x410000, 31), DATANUM(0x410000, 32) }, { DATANUM(0x410000, 41), DATANUM(0x410000, 42), DATANUM(0x410000, 43), DATANUM(0x410000, 44), DATANUM(0x410000, 45), DATANUM(0x410000, 46), DATANUM(0x410000, 47), DATANUM(0x410000, 48) } };
        modelIndex = index + 0xC;
        obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(stageAssets[lbl_1_bss_20][index], 0x10000000, 2));
        fn_80026B8C(obj->mdlId[modelIndex], 0);
        fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 1)));
    }
    for (index = 0; index < 8; index++)
    {
        modelIndex = index + 0x1C;
        if (index == 0)
        {
            obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_20 != 0 ? DATANUM(0x410000, 10) : DATANUM(0x410000, 3), 0x10000000, 2));
        }
        else
        {
            obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[0x1C]);
        }
        fn_80026B8C(obj->mdlId[modelIndex], 0);
        fn_8002D414(obj->mdlId[modelIndex], fn_8002D530(obj->mdlId[modelIndex]));
        fn_80026808(obj->mdlId[modelIndex], 0.0f, (float)((index * 0x2D) - 0x87), 0.0f);
        fn_80026B64(obj->mdlId[modelIndex], 0xFFFFU);
        modelIndex = index + 0x24;
        if (index == 0)
        {
            obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_20 != 0 ? DATANUM(0x410000, 11) : DATANUM(0x410000, 4), 0x10000000, 2));
        }
        else
        {
            obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[0x24]);
        }
        fn_80026B8C(obj->mdlId[modelIndex], 0);
        fn_80026808(obj->mdlId[modelIndex], 0.0f, (float)((index * 0x2D) - 0x87), 0.0f);
        fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
        fn_80026B64(obj->mdlId[modelIndex], 1U);
        modelIndex = index + 0x14;
        if (index == 0)
        {
            obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(lbl_1_bss_20 != 0 ? DATANUM(0x410000, 22) : DATANUM(0x410000, 21), 0x10000000, 2));
        }
        else
        {
            obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[0x14]);
        }
        fn_80026B8C(obj->mdlId[modelIndex], 0);
        fn_80026808(obj->mdlId[modelIndex], 0.0f, (float)(index * 0x2D), 0.0f);
        fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 1)));
    }
    for (index = 0; index < (rand8() % 3) + 1; index++)
    {
        fn_8002D414(obj->mdlId[((rand8() % 8) + 0x1C)], ((float) rand8() / 255.0f) * fn_8002D530(obj->mdlId[modelIndex]));
    }
    for (index = 0; index < 8; index++)
    {
        for (subIndex = 0; subIndex < 4; subIndex++)
        {
            modelIndex = (index * 4) + 0x5D + subIndex;
            if (subIndex == 0)
            {
                s32 actorAssets[2][8] = { { DATANUM(0x410000, 33), DATANUM(0x410000, 34), DATANUM(0x410000, 35), DATANUM(0x410000, 36), DATANUM(0x410000, 37), DATANUM(0x410000, 38), DATANUM(0x410000, 39), DATANUM(0x410000, 40) }, { DATANUM(0x410000, 49), DATANUM(0x410000, 50), DATANUM(0x410000, 51), DATANUM(0x410000, 52), DATANUM(0x410000, 53), DATANUM(0x410000, 54), DATANUM(0x410000, 55), DATANUM(0x410000, 56) } };
                obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(actorAssets[lbl_1_bss_20][index], 0x10000000, 2));
            }
            else
            {
                obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[(index * 4) + 0x5D]);
            }
            fn_80026B8C(obj->mdlId[modelIndex], 0);
            fn_80026B64(obj->mdlId[modelIndex], (u16) lbl_1_rodata_30[subIndex]);
            fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 1)));
            if (index == 7)
            {
                fn_8002D414(obj->mdlId[modelIndex], fn_8002D530(obj->mdlId[modelIndex]));
            }
            if (index != 0 && index != 7)
            {
                fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
            }
        }
    }
    memset(lbl_1_bss_A168, 0, 0x1C80U);
    for (index = 0; index < 4; index++)
    {
        for (subIndex = 0; subIndex < 0x13; subIndex++)
        {
            modelIndex = (index * 0x13) + 0x7E + subIndex;
            if (index == 0 && subIndex == 0)
            {
                obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x410000, 17), 0x10000000, 2));
                obj->mtnId[0] = fn_80030458(obj->mdlId[modelIndex], fn_80008DD0(DATANUM(0x410000, 18), 0x10000000, 2));
                obj->mtnId[1] = fn_80030458(obj->mdlId[modelIndex], fn_80008DD0(DATANUM(0x410000, 19), 0x10000000, 2));
            }
            else
            {
                obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[0x7E]);
            }
            fn_80026B8C(obj->mdlId[modelIndex], 5);
            fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
            fn_80026B64(obj->mdlId[modelIndex], (u16) lbl_1_rodata_30[index]);
            fn_800273EC(obj->mdlId[modelIndex], 2.0f, 2.0f, 2.0f);
        }
    }
    fn_1_5ABC(0, 7);
    fn_1_5ABC(0, 0);
    fn_1_5ABC(0, 1);
    fn_1_5ABC(1, 2);
    fn_1_5ABC(2, 3);
    fn_1_5ABC(1, 4);
    fn_1_5ABC(3, 5);
    fn_1_5ABC(2, 6);
    for (index = 1; index < 4; index++)
    {
        for (subIndex = 0; subIndex < 0x13; subIndex++)
        {
            fn_80026B64(obj->mdlId[(index * 0x13) + 0x7E + subIndex], 1U);
        }
    }
    for (index = 0; index < 4; index++)
    {
        modelIndex = index + 0x2C;
        if (index == 0)
        {
            obj->mdlId[modelIndex] = fn_80025A04(fn_80008DD0(DATANUM(0x410000, 20), 0x10000000, 2));
        }
        else
        {
            obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[0x2C]);
        }
        fn_800269E8(obj->mdlId[modelIndex], (1 << 22) | (1 << 0));
        fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
        fn_80026B64(obj->mdlId[modelIndex], (u16) lbl_1_rodata_30[index]);
        fn_80026B8C(obj->mdlId[modelIndex], 2);
        fn_800273EC(obj->mdlId[modelIndex], 2.0f, 2.0f, 2.0f);
        for (subIndex = 0; subIndex < 0xB; subIndex++)
        {
            modelIndex = index + 0x30 + (subIndex * 4);
            obj->mdlId[modelIndex] = fn_80025D4C(obj->mdlId[index + 0x2C]);
            fn_800269E8(obj->mdlId[modelIndex], (1 << 0));
            fn_800269E8(obj->mdlId[modelIndex], ((1 << 30) | (1 << 0)));
            fn_80026B64(obj->mdlId[modelIndex], (u16) lbl_1_rodata_30[index]);
            fn_80026B8C(obj->mdlId[modelIndex], 2);
            fn_800273EC(obj->mdlId[modelIndex], 2.0f, 2.0f, 2.0f);
        }
    }
    modelIndex = 7;
    obj->mdlId[modelIndex] = fn_800260F4(fn_1_BD0);
    fn_80026B8C(obj->mdlId[modelIndex], 7);
    fn_80026B64(obj->mdlId[modelIndex], 8U);
    lbl_1_bss_A0C8.entry = espEntry(DATANUM(0x410000, 13), 0, 0);
    espDispOff((s16) lbl_1_bss_A0C8.entry);
    for (index = 0; index < 4; index++)
    {
        modelIndex = index + 8;
        obj->mdlId[modelIndex] = fn_800260F4(fn_1_112C);
        fn_80026B8C(obj->mdlId[modelIndex], 1);
        fn_80026B64(obj->mdlId[modelIndex], (u16)(1 << index));
    }
    obj->objFunc = (OMOBJ_FUNC) fn_1_7110;
}

void fn_1_EF90(OMOBJ *obj)
{
    M606PlayerWork *work;
    s32 playerOrder[4] = {0, 1, 2, 3};
    s32 groupIndex;
    s32 playerIndex;
    s32 playerCount;
    s32 selectedCharacter;
    s32 padIndex;
    s32 computerDifficulty;
    MGACTOR_PARAM actorParam;
    MGACTOR_COLMAP_POLY collision;
    HuVecF rayEnd;

    playerCount = 0;
    work = obj->data;
    memset(work, 0, sizeof(*work));
    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if (groupIndex == lbl_80265728[playerIndex].grpNo) {
                playerOrder[playerCount] = playerIndex;
                playerCount++;
            }
        }
    }

    work->playerIndex = obj->work[0];
    work->configIndex = playerOrder[work->playerIndex];
    if (work->configIndex == 4) {
        work->configIndex = work->playerIndex;
    }
    work->character = lbl_80265728[work->configIndex].charNo;
    if (work->character >= 0xB) {
        selectedCharacter = 9;
    } else {
        selectedCharacter = work->character;
    }
    work->modelCharacter = selectedCharacter;
    if (lbl_80265728[work->configIndex].type != 0) {
        padIndex = -1;
    } else {
        padIndex = lbl_80265728[work->configIndex].padNo;
    }
    work->padIndex = padIndex;
    lbl_1_bss_30 += !lbl_80265728[work->configIndex].type;
    if (work->padIndex != -1) {
        computerDifficulty = 0;
    } else {
        computerDifficulty = lbl_80265728[work->configIndex].comDif;
    }
    work->computerDifficulty = computerDifficulty;
    work->state = 2;
    work->sequenceFrame = 0x7FFF;
    work->sequenceCount = 0;
    work->randomValue = (s32)((80.0f * ((float)rand8() / 255.0f)) - 40.0f);
    work->currentMode = -1;

    actorParam.height = 150.0f;
    actorParam.radius = 50.0f;
    actorParam.param = work->playerIndex;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.correctHookParam = 0;
    actorParam.narrowHook = NULL;
    actorParam.correctHook = NULL;
    lbl_1_bss_A0EC[work->playerIndex] = fn_80080728((s16)work->configIndex, &actorParam, 8,
        (u16)lbl_1_rodata_30[work->playerIndex], (1 << 0) | (1 << 5),
        lbl_1_data_184);
    *obj->mdlId = (s16)lbl_1_bss_A0EC[work->playerIndex]->actor->mdlId;
    fn_8005DDEC(4);
    fn_80080DD4(lbl_1_bss_A0EC[work->playerIndex]->actor, 1 << work->playerIndex);
    lbl_1_bss_A0EC[work->playerIndex]->actor->pos.x = 0.0f;
    lbl_1_bss_A0EC[work->playerIndex]->actor->pos.y = 100.0f;
    lbl_1_bss_A0EC[work->playerIndex]->actor->pos.z = -900.0f;
    lbl_1_bss_A0EC[work->playerIndex]->actor->rotY = -90.0f;
    fn_8007ED60(lbl_1_bss_A0EC[work->playerIndex]);
    rayEnd = lbl_1_bss_A0EC[work->playerIndex]->actor->pos;
    rayEnd.y = -500.0f;
    if (fn_80086910(&lbl_1_bss_A0EC[work->playerIndex]->actor->pos,
        &rayEnd, 0xFFFFU, &collision) != FALSE) {
        collision.pos.y += 75.0f;
        fn_80086AA8(lbl_1_bss_A0EC[work->playerIndex]->actor, &collision.pos);
        lbl_1_bss_A0EC[work->playerIndex]->actor->pos.y = collision.pos.y;
    }
    fn_80026B8C(*obj->mdlId, 3);
    fn_8005DDEC(4);
    fn_80026B64(*obj->mdlId, (u16)lbl_1_rodata_30[work->playerIndex]);
    work->lightId = fn_1_3144(*obj->mdlId);
    if (lbl_1_bss_20 != 0) {
        fn_800269E8(*obj->mdlId, (1 << 28));
        fn_80028E48(*obj->mdlId, (s16)work->lightId,
            colorD0.r, colorD0.g, colorD0.b, 0xFFU);
        fn_800273EC(*obj->mdlId, 0.3f,
            0.25f, 0.375f);
    }
    fn_80029E44(*obj->mdlId, (s16)lbl_1_bss_A0DC);
    obj->objFunc = fn_1_35B8;
}


/* Starts the stage sequence by installing its main update callback. */
void fn_1_F63C(OMOBJ *obj)
{
    obj->objFunc = fn_1_B20C;
}























extern void fn_1_DEBC(OMOBJ *obj);

extern void fn_1_EF90(OMOBJ *obj);

extern void fn_1_F63C(OMOBJ *obj);

/* Initializes M606's player cameras and the shared stage light. */
const Point3d lbl_1_rodata_E3C = { 0.0f, 5000.0f, 0.0f };

const Point3d lbl_1_rodata_E48 = { 0.2f, -1.0f, -0.5f };

const Point3d lbl_1_rodata_E54 = { 0.0f, -1.0f, -1.0f };

const GXColor colorE60 = { 255, 255, 255, 255 };

const GXColor colorE64 = { 192, 192, 255, 255 };

const GXColor colorE68 = { 192, 255, 192, 255 };

void ObjectSetup(void)
{
    s16 nightMode;
    float cameraVector2Z;
    float cameraVector2Y;
    float cameraVector2X;
    float cameraVector1Z;
    float cameraVector1Y;
    float cameraVector1X;
    float cameraVector0Z;
    float cameraVector0Y;
    float cameraVector0X;
    Point3d lightPosition;
    Point3d lightDirectionDay;
    Point3d lightDirectionNight;
    GXColor *lightColor;
    Point3d *lightDirection;
    s32 cameraIndex;
    GXColor lightColorDay;
    GXColor lightColorNight;
    GXColor unusedLightColor;

    OSReport("******* M606 ObjectSetup *********\n");
    nightMode = lbl_802C0278;
    if (nightMode == 1) {
        lbl_1_bss_20 = 1;
    }
    lbl_1_bss_14 = fn_8006A124((3 << 16) | 2);
    for (cameraIndex = 0; cameraIndex < 10; cameraIndex++) {
        lbl_1_bss_A118[cameraIndex].actorId = 0;
        lbl_1_bss_A118[cameraIndex].modelId = -1;
    }
    lbl_1_bss_A0E4 = fn_80080418();
    Hu3DCameraCreate(0xF);

    for (cameraIndex = 0; cameraIndex < 4; cameraIndex++) {
        int column = cameraIndex % 2;
        int row = cameraIndex / 2;

        Hu3DCameraPerspectiveSet(lbl_1_rodata_30[cameraIndex], 20.0f, 20.0f,
                                 25000.0f, 1.2f);
        Hu3DCameraViewportSet(lbl_1_rodata_30[cameraIndex],
                              (float)(column * 0x140), (float)(row * 0xF0),
                              320.0f, 240.0f,
                              0.0f, 1.0f);
        Hu3DCameraScissorSet(lbl_1_rodata_30[cameraIndex],
                             column * 0x140, row * 0xF0, 0x140U, 0xF0U);
        Hu3DCameraViewportSet(lbl_1_rodata_30[cameraIndex], (float)((column * 0x118) + 0x28),
                              (float)((row * 0xD2) + 0x14),
                              280.0f, 210.0f,
                              0.0f, 1.0f);
        Hu3DCameraScissorSet(lbl_1_rodata_30[cameraIndex], (column * 0x118) + 0x28,
                             (row * 0xD2) + 0x14, 0x118U, 0xD2U);
        if (cameraIndex == 0) {
            Hu3DCameraViewportSet(lbl_1_rodata_30[cameraIndex], 0.0f, 0.0f,
                                  640.0f, 480.0f,
                                  0.0f, 1.0f);
            Hu3DCameraScissorSet(lbl_1_rodata_30[cameraIndex], 0U, 0U, 0x280U, 0x1E0U);
        } else {
            Hu3DCameraViewportSet(lbl_1_rodata_30[cameraIndex], 0.0f, 0.0f,
                                  0.0f, 0.0f,
                                  0.0f, 1.0f);
            Hu3DCameraScissorSet(lbl_1_rodata_30[cameraIndex], 0U, 0U, 0U, 0U);
        }
        lbl_80266BD8[cameraIndex] = 2325.0f;
        lbl_80266B18[cameraIndex].x = 0.0f;
        lbl_80266B18[cameraIndex].y = 75.0f;
        lbl_80266B18[cameraIndex].z = -900.0f;
        lbl_80266A58[cameraIndex].x = -90.0f;
        lbl_80266A58[cameraIndex].y = 0.0f;
        lbl_80266A58[cameraIndex].z = 0.0f;
        lbl_80266BD8[cameraIndex] = 1250.0f;
        lbl_80266B18[cameraIndex].x = 0.0f;
        lbl_80266B18[cameraIndex].y = 75.0f;
        lbl_80266B18[cameraIndex].z = -900.0f;
        lbl_80266A58[cameraIndex].x = -30.0f;
        lbl_80266A58[cameraIndex].y = -90.0f;
        lbl_80266A58[cameraIndex].z = 0.0f;
        lbl_80266BD8[cameraIndex] = 8450.0f;
        lbl_80266B18[cameraIndex].x = 0.0f;
        lbl_80266B18[cameraIndex].y = 60.0f;
        lbl_80266B18[cameraIndex].z = -900.0f;
        lbl_80266A58[cameraIndex].x = -33.0f;
        lbl_80266A58[cameraIndex].y = 0.0f;
        lbl_80266A58[cameraIndex].z = 0.0f;
    }

    fn_800285BC();
    lightPosition = lbl_1_rodata_E3C;
    lightDirectionDay = lbl_1_rodata_E48;
    lightDirectionNight = lbl_1_rodata_E54;
    lightColorDay = colorE60;
    lightColorNight = colorE64;
    unusedLightColor = colorE68;
    if (lbl_1_bss_20 != 0) {
        lightColor = &lightColorNight;
    } else {
        lightColor = &lightColorDay;
    }
    if (lbl_1_bss_20 != 0) {
        lightDirection = &lightDirectionNight;
    } else {
        lightDirection = &lightDirectionDay;
    }
    lbl_1_bss_A0DC = fn_800286D0(&lightPosition, lightDirection, lightColor);
    Hu3DGLightStaticSet((s16)lbl_1_bss_A0DC, 1);
    Hu3DGLightInfinitytSet((s16)lbl_1_bss_A0DC);
    cameraVector2X = 0.0f;
    cameraVector2Y = 10000.0f;
    cameraVector2Z = 0.0f;
    cameraVector1X = 0.0f;
    cameraVector1Y = 1.0f;
    cameraVector1Z = 0.0f;
    cameraVector0X = 0.0f;
    cameraVector0Y = 0.0f;
    cameraVector0Z = -0.1f;
    lbl_1_bss_A0E0 = -1;
    lbl_1_data_34[0] = -1;
    fn_80070788(&lbl_1_data_60);
    lbl_1_data_5C = fn_8003C860(0);
}

/* Creates the stage setup object, four player objects, and view callbacks. */
void fn_1_FF2C(s16 mode, s16 frameNo)
{
    s32 playerIndex;
    OMOBJ *obj;

    lbl_1_bss_A0FC = fn_8003AB38(lbl_1_bss_A0E4, 100, 202, 2, 1, fn_1_DEBC);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        obj = lbl_1_bss_A100[playerIndex].object = fn_8003AB38(lbl_1_bss_A0E4,
            102, 2, 2, 0, fn_1_EF90);
        obj->data = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M606PlayerWork), 0x10000000);
        obj->work[0] = playerIndex;
    }
    lbl_1_bss_A110 = fn_8003AB38(lbl_1_bss_A0E4, 32730, 0, 0, -1, fn_8003DAA4);
    lbl_1_bss_A110->work[0] = 4;
    lbl_1_bss_A114 = fn_8003AB38(lbl_1_bss_A0E4, 103, 0, 0, -1, fn_1_F63C);
    fn_80071A9C();
}

/* Runs the overlay transition cleanup callback after advancing its sequence. */
void fn_1_10078(s16 mode, s16 frameNo)
{
    void *work;
    s32 i;

    fn_80071A9C();
    i = 0;
    while (i < 4) {
        work = lbl_1_bss_A100[i].object->data;
        i += 1;
    }
    if (lbl_1_bss_A0E0 >= 0) {
        GameMesKill(lbl_1_bss_A0E0);
    }
    GameMesClose();
    HuAudAllStop();
    fn_8003A5E4((s16)lbl_1_data_54, 1);
}
