/* Sprite indicators, letter animation, and message windows for the microphone activity. */
#include "REL/mikeactdll/module_types.h"

#include "game/object.h"

#include "math.h"

#include "dolphin/gx.h"

#include "game/main.h"

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
#include "messdir_enum.h"

#include "game/hu3d.h"

#include "game/window.h"

#include "dolphin.h"

#include "game/flag.h"

#include "game/mgdata.h"

#include "game/process.h"

#include "game/thpmain.h"

#include "dolphin/os.h"

#include "humath.h"
#include "msm_se.h"

extern s32 lbl_1_bss_118;

extern MikeActSpriteGroups lbl_1_bss_19C;

extern HUPROCESS *lbl_1_bss_6C;

void fn_1_E6AC(OMOBJ *object);

void fn_1_E780(OMOBJ *object);

void fn_1_E308(void)
{
    s32 member;
    s16 group;
    s16 sprite;
    ANIMDATA *animation;

    group = HuSprGrpCreate(2);
    animation = HuSprAnimRead(fn_80008634(DATANUM(0xa70000, 13), 0x10000000));
    sprite = HuSprCreate(animation, 0x420, 0);
    HuSprGrpMemberSet(group, 0, sprite);
    HuSprPosSet(group, 0, 288.0f, 145.0f);

    animation = HuSprAnimRead(fn_80008634(DATANUM(0xa70000, 12), 0x10000000));
    sprite = HuSprCreate(animation, 0x410, 0);
    HuSprGrpMemberSet(group, 1, sprite);
    HuSprPosSet(group, 1, 38.0f, 141.0f);
    lbl_1_bss_19C.scene = group;
    fn_8003AB38(lbl_1_bss_6C, 0x2F00, 0, 0, -1, fn_1_E6AC);

    group = HuSprGrpCreate(9);
    lbl_1_bss_19C.progress = group;
    animation = HuSprAnimRead(fn_80008634(DATANUM(0xa70000, 14), 0x10000000));
    sprite = HuSprCreate(animation, 0x410, 0);
    HuSprGrpMemberSet(group, 0, sprite);
    HuSprPosSet(group, 0, 85.0f, 426.0f);

    animation = HuSprAnimRead(fn_80008634(DATANUM(0xa70000, 15), 0x10000000));
    for (member = 0; member < 4; member++) {
        sprite = HuSprCreate(animation, 0x410, 0);
        HuSprGrpMemberSet(group, member + 1, sprite);
        HuSprPosSet(group, member + 1, member * 0x18 + 0x92, 427.0f);
    }

    animation = HuSprAnimRead(fn_80008634(DATANUM(0xa70000, 16), 0x10000000));
    for (member = 0; member < 4; member++) {
        sprite = HuSprCreate(animation, 0x420, 0);
        HuSprGrpMemberSet(group, member + 5, sprite);
        HuSprPosSet(group, member + 5, member * 0x18 + 0x92, 427.0f);
        HuSprAttrSet(group, member + 5, 4);
    }

    fn_8003AB38(lbl_1_bss_6C, 0x2F00, 0, 0, -1, fn_1_E780);
    lbl_1_bss_118 = 1;
    fn_1_E648(0);
}

/* Show or hide the scene and progress indicators. */
void fn_1_E648(s32 state) {
    fn_1_EA08(state);
    if (state != 0) {
        HuSprGrpAttrReset(lbl_1_bss_19C.scene, 4);
        return;
    }
    HuSprGrpAttrSet(lbl_1_bss_19C.scene, 4);
}

void fn_1_E6AC(OMOBJ *object)
{
    f32 centerX;
    s32 spriteX;

    centerX = lbl_80266A4C.x;
    if (centerX < 0.0f) {
        centerX = 0.0f;
    }
    spriteX = 488.0f * (centerX / 49000.0f);
    HuSprPosSet(lbl_1_bss_19C.scene, 1, spriteX + 0x26, 145.0f);
}

void fn_1_E780(OMOBJ *object)
{
    f32 scale;
    f32 sine;
    s16 spriteGroup;
    s16 playerState;
    u32 playerTimer;
    s32 spriteIndex;

    spriteGroup = lbl_1_bss_19C.progress;
    if (lbl_1_bss_118 != 0) {
        if (lbl_1_bss_B8 != 0) {
            playerState = lbl_1_bss_B8->speedIndex;
            if (fn_1_45D8(lbl_1_bss_B8, 0xC) != 0) {
                playerState += 10;
            }
        } else {
            playerState = 0;
        }
        if (playerState >= 10) {
            playerTimer = lbl_1_bss_B8->timer;
            sine = (f32) sin((M_PI * (f64) ((90.0f * (f32) playerTimer) / 30.0f)) / 180.0);
            if (sine < 0.0f) {
                sine = -sine;
            }
            scale = 1.0f + 0.2f * sine;
        }
        spriteIndex = 0;
        while (spriteIndex < 4) {
            if (playerState < 10) {
                if (spriteIndex < playerState) {
                    HuSprBankSet(spriteGroup, spriteIndex + 1, 1);
                    HuSprTPLvlSet(spriteGroup, spriteIndex + 1, 1.0f);
                } else {
                    HuSprBankSet(spriteGroup, spriteIndex + 1, 0);
                    HuSprTPLvlSet(spriteGroup, spriteIndex + 1, 0.6f);
                }
                HuSprAttrSet(spriteGroup, spriteIndex + 5, 4);
            } else {
                HuSprTPLvlSet(spriteGroup, spriteIndex + 1, 1.0f);
                HuSprBankSet(spriteGroup, spriteIndex + 1, 2);
                HuSprAttrReset(spriteGroup, spriteIndex + 5, 4);
                HuSprScaleSet(spriteGroup, spriteIndex + 5, scale, scale);
            }
            spriteIndex++;
        }
    }
}

/* Set the progress mode and update its sprite visibility. */
void fn_1_EA08(s32 mode) {
    s32 member;

    lbl_1_bss_118 = mode;
    if (mode != 0) {
        member = 0;
        while (member < 4) {
            HuSprAttrReset(lbl_1_bss_19C.progress, (s16) (member + 1), 4);
            member += 1;
        }
        HuSprAttrReset(lbl_1_bss_19C.progress, 0, 4);
        HuSprTPLvlSet(lbl_1_bss_19C.progress, 0, 1.0f);
        return;
    }
    HuSprGrpAttrSet(lbl_1_bss_19C.progress, 4);
}

extern s16 lbl_1_bss_11C;

extern OMOBJ *lbl_1_bss_120;

typedef struct {
    s16 state;
    u8 unknown_02[6]; /* No code names this field. */
    s16 timer;
    u8 unknown_0A[2]; /* No code names this field. */
} MikeActLetterState;

extern MikeActLetterState lbl_1_bss_124[10];

void fn_1_ED94(OMOBJ *object);

void fn_1_EACC(void)
{
    HUSPRID sprite;
    HUSPR_GROUPID group;
    ANIMDATA *animation;
    s32 i;

    group = HuSprGrpCreate(0x14);
    lbl_1_bss_19C.letters = group;
    animation = HuSprAnimRead(fn_80008634(DATANUM(0xa70000, 17), 0x10000000));
    i = 0;
    while (i < 10) {
        sprite = HuSprCreate(animation, 0x420, 1);
        HuSprGrpMemberSet(group, i, sprite);
        HuSprPosSet(group, i, (i % 5) * 0x19 + 0x1BD, (i / 5) * 0x2A + 0x177);
        i++;
    }

    animation = HuSprAnimRead(fn_80008634(DATANUM(0xa70000, 18), 0x10000000));
    i = 0;
    while (i < 10) {
        sprite = HuSprCreate(animation, 0x410, 0);
        HuSprGrpMemberSet(group, i + 10, sprite);
        HuSprPosSet(group, i + 10, (i % 5) * 0x19 + 0x1BD, (i / 5) * 0x2A + 0x177);
        HuSprAttrSet(group, i + 10, 4);
        i++;
    }

    memset(lbl_1_bss_124, 0, 0x78);
    lbl_1_bss_11C = 0;
    lbl_1_bss_120 = fn_8003AB38(lbl_1_bss_6C, 0x2F00, 0, 0, -1, fn_1_ED94);
    lbl_1_bss_120->work[0] = 0;
    lbl_1_bss_120->work[1] = 0;
}

void fn_1_ED94(OMOBJ *object)
{
    HUSPR_GROUPID group;
    f32 alpha;
    s32 index;

    group = lbl_1_bss_19C.letters;
    if (object->work[0] == 1U) {
        object->work[1]++;
    }

    index = 0;
    while (index < 10) {
        switch (lbl_1_bss_124[index].state) {
        case 0:
            break;
        case 1:
            lbl_1_bss_124[index].timer = 0;
            HuSprAttrReset(group, (s16)(index + 10), 4);
            HuSprTPLvlSet(group, (s16)(index + 10), 0.0f);
            lbl_1_bss_124[index].state = 2;
            HuAudFXPlay(0x53B);
            break;
        case 2:
            lbl_1_bss_124[index].timer++;
            alpha = 0.04f * (f32)lbl_1_bss_124[index].timer;
            HuSprTPLvlSet(group, (s16)(index + 10), alpha);
            if (lbl_1_bss_124[index].timer == 20) {
                HuSprBankSet(group, (s16)index, 0);
                lbl_1_bss_124[index].state = 3;
            }
            break;
        case 3:
            lbl_1_bss_124[index].timer--;
            alpha = 0.04f * (f32)lbl_1_bss_124[index].timer;
            HuSprTPLvlSet(group, (s16)(index + 10), alpha);
            if (lbl_1_bss_124[index].timer == 0) {
                HuSprAttrSet(group, (s16)(index + 10), 4);
                lbl_1_bss_124[index].state = 4;
            }
            break;
        case 4:
            if (index == (s32)(lbl_1_bss_11C - 1) &&
                object->work[1] >= 30U) {
                object->work[1] = 0;
                HuSprBankSet(group, (s16)index, 1);
                lbl_1_bss_124[index].state = 5;
                lbl_1_bss_11C--;
            }
            break;
        case 5:
            break;
        }
        index++;
    }
}

/* Advance the next letter into its highlight animation. */
void fn_1_F0B8(void) {
    if (lbl_1_bss_11C != 0xA) {
        if (lbl_1_bss_124[lbl_1_bss_11C].state == 0) {
            lbl_1_bss_124[lbl_1_bss_11C].state = 1;
        }
        lbl_1_bss_11C += 1;
    }
}

s16 fn_1_F138(void)
{
    return lbl_1_bss_11C;
}

/* Start the final letter animation once every letter has been selected. */
void fn_1_F148(void) {
    if (lbl_1_bss_11C == 0xA) {
        lbl_1_bss_120->work[0] = 1;
        lbl_1_bss_120->work[1] = 0;
    }
}

/* Destroy the letter sprites and their update object. */
void fn_1_F188(void) {
    if (lbl_1_bss_120 != NULL) {
        HuSprGrpKill(lbl_1_bss_19C.letters);
        omDelObjEx(lbl_1_bss_6C, lbl_1_bss_120);
        lbl_1_bss_120 = NULL;
    }
}

void fn_1_F630(OMOBJ *object);

extern s32 lbl_1_bss_100;

extern OMOBJ *lbl_1_bss_104;

extern s16 lbl_1_bss_110;

extern s16 lbl_1_bss_10E;

extern s16 lbl_1_bss_112;

extern s16 lbl_1_bss_114;

extern s16 lbl_1_bss_116;

void fn_1_F1F8(void)
{
    HuWinInit(1);
    lbl_1_bss_116 = HuWinExCreateFrame(166.0f, 34.0f, 0x190, 0x60, 6, 4);
    HuWinDispOff(lbl_1_bss_116);
    lbl_1_bss_114 = 0;
    lbl_1_bss_112 = 0;
    lbl_1_bss_10E = 0x63;
    lbl_1_bss_100 = 0;
    lbl_1_bss_104 = fn_8003AB38(lbl_1_bss_6C, 0x2F00, 0, 0, -1, fn_1_F630);
}

/* Show or hide the message window. */
void fn_1_F2D8(s32 visible) {
    if (visible)
    {
        HuWinDispOn(lbl_1_bss_116);
        return;
    }
    HuWinDispOff(lbl_1_bss_116);
}

void fn_1_F328(void) {
    omDelObjEx(lbl_1_bss_6C, lbl_1_bss_104);
    HuWinKill(lbl_1_bss_116);
}

extern u32 lbl_1_bss_108;

/* Set the message and insertion state, or clear the window cursor. */
void fn_1_F370(u32 messageNo, u16 insertNo) {
    if (messageNo != 0U) {
        lbl_1_bss_108 = messageNo;
        HuWinMesSet(lbl_1_bss_116, messageNo);
        if (insertNo != 0) {
            lbl_1_bss_112 = (s16) insertNo;
            lbl_1_bss_10E = 4;
            return;
        }
        lbl_1_bss_10E = 3;
        return;
    }
    HuWinHomeClear(lbl_1_bss_116);
}

extern s16 lbl_1_bss_10C;

void fn_1_F420(u32 messageNo, u16 insertNo)
{
    if (messageNo != 0U) {
        lbl_1_bss_108 = messageNo;
        HuWinMesSet(lbl_1_bss_116, messageNo);
        if (insertNo != 0) {
            lbl_1_bss_112 = (s16)insertNo;
            lbl_1_bss_10E = 4;
        } else {
            lbl_1_bss_10E = 3;
        }
    } else {
        HuWinHomeClear(lbl_1_bss_116);
    }
    HuWinInsertMesSet(lbl_1_bss_116, lbl_1_bss_C4[0]->character_no, 0);
}

void fn_1_F4F4(u32 messageNo)
{
    if (lbl_1_bss_10E != 5) {
        if (lbl_1_bss_10E <= 1) {
            lbl_1_bss_108 = 0;
        }
        lbl_1_bss_110 = 0;
        lbl_1_bss_10C = lbl_1_bss_10E;
        lbl_1_bss_10E = 5;
        HuWinMesSet(lbl_1_bss_116, messageNo);
    }
}

void fn_1_F598(void) {
    lbl_1_bss_10E = 0;
    lbl_1_bss_112 = 0;
}

/* Play the guide introduction and display its message. */
void fn_1_F5BC(void) {
    HuAudFXPlay(MSM_SE_GUIDE_DAY_INTRO);
    lbl_1_bss_10E = 2;
    lbl_1_bss_112 = 0;
    HuWinMesSet(lbl_1_bss_116, MESSNUM(MESS_MICGO, 11));
    lbl_1_bss_108 = MESSNUM(MESS_MICGO, 11);
    fn_1_1E78();
}

typedef struct {
    u32 messages[9];
} MikeActMessageTable;

extern MikeActMessageTable lbl_1_data_4E8[2];

void fn_1_F630(OMOBJ *object)
{
    switch (lbl_1_bss_10E) {
    case 0:
        if (lbl_1_bss_114 == 0 && lbl_1_bss_100 == 0) {
            lbl_1_bss_100 = 1;
            HuAudFXPlay(MSM_SE_GUIDE_DAY_EXPLAIN);
        }
        /* fall through */
    case 1:
        if (lbl_1_bss_F0->owner == NULL) {
            if (lbl_1_bss_10E == 1) {
                lbl_1_bss_10E = 0;
                lbl_1_bss_114 = 0;
                lbl_1_bss_112 = 0;
            }
        } else if (lbl_1_bss_10E == 0) {
            lbl_1_bss_10E = 1;
            lbl_1_bss_114 = 0;
            lbl_1_bss_112 = 0;
        }
        if (lbl_1_bss_112++ == 0) {
            HuWinMesSet(lbl_1_bss_116, lbl_1_data_4E8[lbl_1_bss_10E].messages[lbl_1_bss_114]);
            return;
        }
        if (lbl_1_bss_112 == 0xB4) {
            lbl_1_bss_112 = 0;
            lbl_1_bss_114++;
            if (lbl_1_data_4E8[lbl_1_bss_10E].messages[lbl_1_bss_114] == 0) {
                lbl_1_bss_114 = 0;
            }
        }
        break;
    case 2:
        if (fn_1_1F70() != 0) {
            HuWinHomeClear(lbl_1_bss_116);
            lbl_1_bss_10E = 3;
        }
        break;
    case 4:
        lbl_1_bss_112--;
        if (lbl_1_bss_112 == 0) {
            lbl_1_bss_10E = 0;
            lbl_1_bss_114 = 0;
        }
        break;
    case 5:
        if (lbl_1_bss_110++ >= 0x5A) {
            lbl_1_bss_10E = lbl_1_bss_10C;
            if (lbl_1_bss_108 != 0) {
                HuWinMesSet(lbl_1_bss_116, lbl_1_bss_108);
            }
        }
        break;
    }
}

void fn_1_F968(OMOBJ *object);

HUSPR_GROUPID fn_1_FF58(u32 messNum);

extern OMOBJ *lbl_1_bss_F8;

extern s32 lbl_1_bss_FC;

void fn_1_F968(OMOBJ *object)
{
    f32 scale;

    switch (object->work[3]) {
    case 1:
        lbl_1_bss_FC = HuAudFXPlay(0x53C);
        HuWinDispOn((s16)object->work[0]);
        HuSprGrpAttrReset((s16)object->work[1], 4);
        HuWinBGTPLvlSet((s16)object->work[0], 0.5f);
        HuSprGrpTPLvlSet((s16)object->work[1], 1.0f);
        object->work[2] = 0;
        object->work[3] = 2;
        HuWinScaleSet((s16)object->work[0], 0.0f, 0.0f);
        HuSprGrpScaleSet((s16)object->work[1], 0.0f, 0.0f);
        return;
    case 2:
        object->work[2]++;
        scale = (f32)object->work[2] / 20.0f;
        scale = (f32)(sin((M_PI * (f64)(120.0f * scale)) / 180.0) *
                      (1.0 / sin(M_PI * 120.0 / 180.0)));
        HuWinScaleSet((s16)object->work[0], scale, scale);
        HuSprGrpScaleSet((s16)object->work[1], scale, scale);
        if (object->work[2] >= 20U) {
            object->work[2] = 0;
            object->work[3] = 0;
        }
        break;
    case 3:
        fn_1_3128(lbl_1_bss_FC);
        object->work[2] = 0;
        object->work[3] = 4;
        return;
    case 4:
        object->work[2]++;
        HuWinBGTPLvlSet((s16)object->work[0], 0.5f * ((f32)(15 - object->work[2]) / 15.0f));
        HuSprGrpTPLvlSet((s16)object->work[1], (f32)(15 - object->work[2]) / 15.0f);
        if (object->work[2] >= 15U) {
            object->work[2] = 0;
            HuWinDispOff((s16)object->work[0]);
            HuSprGrpAttrSet((s16)object->work[1], 4);
            object->work[3] = 0;
        }
        break;
    case 0:
        break;
    }
}

s16 fn_1_FC80(s32 selection)
{
    HUSPR_GROUP *group;
    f32 windowDimensions[2];

    lbl_1_bss_F8 = fn_8003AB38(lbl_1_bss_6C, 0x2F00, 0, 0, -1, fn_1_F968);
    lbl_1_bss_F8->work[1] = fn_1_FF58((u32)selection);
    fn_800097F4(0x270000);

    group = &HuSprGrpData[lbl_1_bss_F8->work[1]];
    windowDimensions[0] = (f32)((group->work[0] + 0x1F) & 0xFFF0);
    windowDimensions[1] = 48.0f;
    HuSprGrpPosSet(lbl_1_bss_F8->work[1], 288.0f, 240.0f);
    lbl_1_bss_F8->work[0] =
        HuWinCreate( 288.0f, 240.0f, (s16)windowDimensions[0], (s16)windowDimensions[1], 0);
    HuWinCenterPosSet(lbl_1_bss_F8->work[0],
                      windowDimensions[0] / 2.0f,
                      windowDimensions[1] / 2.0f);
    HuWinBGTPLvlSet(lbl_1_bss_F8->work[0], 0.5f);
    HuWinDispOff(lbl_1_bss_F8->work[0]);
    HuSprGrpAttrSet(lbl_1_bss_F8->work[1], 4);
    return (s16)lbl_1_bss_F8->work[1];
}

/* Select the window opening or closing animation. */
void fn_1_FE94(s32 visible) {
    if (visible != 0) {
        lbl_1_bss_F8->work[3] = 1;
        return;
    }
    lbl_1_bss_F8->work[3] = 3;
}

/* Destroy the animated window and its sprite group. */
void fn_1_FECC(void) {
    if (lbl_1_bss_F8) {
        HuWinKill(lbl_1_bss_F8->work[0]);
        HuSprGrpKill(lbl_1_bss_F8->work[1]);
        omDelObjEx(lbl_1_bss_6C, lbl_1_bss_F8);
        lbl_1_bss_F8 = NULL;
    }
}

extern u32 lbl_1_data_530[];
extern u8 lbl_1_data_848[];

HUSPR_GROUPID fn_1_FF58(u32 messNum)
{
    char *message;
    s16 width;
    s16 *yPosTbl;
    s16 i;
    s16 *xPosTbl;
    ANIMDATA **animDataTbl;
    HUSPR_GROUPID groupId;
    HUSPR_GROUP *group;
    HUSPRID sprId;
    s16 code;

    {
    s16 sprNum;
    {
    u32 *fileNumTbl;
    fileNumTbl = lbl_1_data_530;
    animDataTbl = HuMemDirectMalloc(0, 400);
    xPosTbl = HuMemDirectMalloc(0, 200);
    yPosTbl = HuMemDirectMalloc(0, 200);
    message = HuWinMesPtrGet(messNum);
    width = 0;
    sprNum = 0;

    while (*message != '\0') {
        code = *message;
        if (*message == ' ' || *message == 16) {
            width += lbl_1_data_848[*message];
        } else if (*message >= 48) {
            /* inst name data yoffset bit. */
            u32 fileNum;
            fileNum = fileNumTbl[*message];
            animDataTbl[sprNum] = HuSprAnimRead( fn_80008DD0(fileNum & ~(1U << 31), 0x10000000, 2));
            xPosTbl[sprNum] = width;
            yPosTbl[sprNum] = 0;
            width += lbl_1_data_848[code];
            if (fileNum & (1U << 31)) {
                yPosTbl[sprNum] = 4;
            }
            sprNum++;
        }
        message++;
    }

    }

    groupId = HuSprGrpCreate(sprNum);
    group = &HuSprGrpData[groupId];
    group->work[0] = width;
    width = (width / 2) - 14;
    for (i = 0; i < sprNum; i++) {
        sprId = HuSprCreate(animDataTbl[i], 999, 0);
        HuSprGrpMemberSet(groupId, i, sprId);
        HuSprPosSet(groupId, i, xPosTbl[i] - width, yPosTbl[i]);
    }

    HuMemDirectFree(animDataTbl);
    HuMemDirectFree(xPosTbl);
    HuMemDirectFree(yPosTbl);
    return groupId;
    }
}

MikeActMessageTable lbl_1_data_4E8[2] = { { { 0x240002, 0x240003, 0x24001F, 0, 0, 0, 0, 0, 0 } },
    { { 0x240004, 0x240005, 0x240006, 0x240007, 0x240008, 0x240009, 0x24000A, 0x240025, 0 } } };
u32 lbl_1_data_530[198] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x270000, 0x270001,
    0x270002, 0x270003, 0x270004, 0x270005, 0x270006, 0x270007, 0x270008, 0x270009, 0x270000,
    0x270000, 0x270000, 0x27000A, 0x270000, 0x270000, 0x270000, 0x270010, 0x270011, 0x270012,
    0x270013, 0x270014, 0x270015, 0x270016, 0x270017, 0x270018, 0x270019, 0x27001A, 0x27001B,
    0x27001C, 0x27001D, 0x27001E, 0x27001F, 0x270020, 0x270021, 0x270022, 0x270023, 0x270024,
    0x270025, 0x270026, 0x270027, 0x270028, 0x270029, 0x27001E, 0x27000D, 0x27001E, 0x27001E,
    0x27001E, 0x27001E, 0x8027002A, 0x8027002B, 0x8027002C, 0x8027002D, 0x8027002E, 0x8027002F,
    0x80270030, 0x80270031, 0x80270032, 0x80270033, 0x80270034, 0x80270035, 0x80270036, 0x80270037,
    0x80270038, 0x80270039, 0x8027003A, 0x8027003B, 0x8027003C, 0x8027003D, 0x8027003E, 0x8027003F,
    0x80270040, 0x80270041, 0x80270042, 0x80270043, 0x270000, 0x270000, 0x270000, 0x27000E,
    0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x27000A, 0x8027000F, 0x270000, 0x270000,
    0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000,
    0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000,
    0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000,
    0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000,
    0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000,
    0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000, 0x270000,
    0x270000, 0x270000, 0x270000, 0x270000, 0x8027000B, 0x8027000C, 0x270000, 0x270000 };
u8 lbl_1_data_848[272] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 12, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 22, 30, 30, 30, 30, 30, 30, 30, 28, 28, 30, 30, 20, 30, 30, 30, 32,
    30, 32, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 16, 30, 30, 30, 12, 24, 22, 24, 22, 24,
    24, 22, 24, 16, 20, 22, 16, 24, 24, 24, 24, 24, 22, 24, 22, 24, 24, 24, 24, 24, 24, 30, 30, 30,
    30, 30, 30, 30, 16, 30, 30, 16, 30, 32, 32, 32, 32, 32, 32, 32, 32, 32, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 18, 24, 30, 30,
    30, 32, 32, 32, 32, 32, 32, 32, 32, 32, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 24, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30 };

MikeActSpriteGroups lbl_1_bss_19C;
MikeActLetterState lbl_1_bss_124[10];
OMOBJ *lbl_1_bss_120;
s16 lbl_1_bss_11C;
s32 lbl_1_bss_118;
s16 lbl_1_bss_116;
s16 lbl_1_bss_114;
s16 lbl_1_bss_112;
s16 lbl_1_bss_110;
s16 lbl_1_bss_10E;
s16 lbl_1_bss_10C;
u32 lbl_1_bss_108;
OMOBJ *lbl_1_bss_104;
s32 lbl_1_bss_100;
s32 lbl_1_bss_FC;
OMOBJ *lbl_1_bss_F8;
