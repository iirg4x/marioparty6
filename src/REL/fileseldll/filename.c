#include "dolphin/math.h"
#include "dolphin.h"
#include "REL/fileseldll.h"
#include "game/card.h"
#include "game/window.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/main.h"
#include "string.h"

#include "messnum/file_select.h"

enum {
    NAME_RANDOM_MES_COUNT = 7,
    NAME_CHARACTER_LIMIT = 8,
    NAME_SPRITE_COUNT = 18,
    NAME_KEYBOARD_WINDOW_COUNT = 3,
    NAME_MES_TEMP_LENGTH = 22,
    NAME_MES_LENGTH = 50,
    NAME_MES_CURSOR = 191,
    NAME_MES_WIDE_CHAR_1 = 128,
    NAME_MES_WIDE_CHAR_2 = 129,
    NAME_MES_CONTROL_CHAR = 16,
    NAMEKEYBOARD_PAGE_STEP = 1 << 8,
    NAMEKEYBOARD_PAGE_ALPHA = NAMEKEYBOARD_PAGE_STEP,
    NAMEKEYBOARD_PAGE_SYMBOL = 2 * NAMEKEYBOARD_PAGE_STEP,
    NAMEKEYBOARD_PAGE_JAPANESE = 3 * NAMEKEYBOARD_PAGE_STEP,
    NAMEKEYBOARD_DELETE = 4 * NAMEKEYBOARD_PAGE_STEP,
    NAMEKEYBOARD_CONFIRM = 5 * NAMEKEYBOARD_PAGE_STEP,
    NAMEKEYBOARD_COMMAND_MIN = NAMEKEYBOARD_PAGE_ALPHA,
    NAMEKEYBOARD_COMMAND_MAX = NAMEKEYBOARD_CONFIRM,
    NAME_SYMBOL_CODE = 14 << 8,
    FILESEL_SFX_KEYBOARD_CHANGE = 1162,
    NAME_SPR_KEYBOARD_ALPHA = 3,
    NAME_SPR_KEYBOARD_SYMBOL = 4,
    NAME_SPR_KEYBOARD_JAPANESE = 5,
    NAME_SPR_DELETE_ICON = 6,
    NAME_SPR_CONFIRM_ICON = 7,
    NAME_SPR_PAGE_LEFT = 10,
    NAME_SPR_PAGE_RIGHT = 11,
    NAME_SPR_DELETE = 12,
    NAME_SPR_CONFIRM = 13,
    NAME_SPR_ALPHA_TAB = 14,
    NAME_SPR_SYMBOL_TAB = 15,
    NAME_SPR_JAPANESE_TAB = 16,
};

typedef struct NameKeyboardBounds_s {
    f32 minX;
    f32 minY;
    f32 maxX;
    f32 maxY;
} NAMEKEYBOARD_BOUNDS;

typedef struct NameSpriteConfig_s {
    s32 dataNum;
    u8 bank;
    s16 priority;
    s16 x;
    s16 y;
} NAME_SPRITE_CONFIG;




void fn_1_8F34(s32 unused, s16 keyNo);
s16 fn_1_AAEC(s16 nameWinId);
s16 fn_1_ABF8(s16 id, f32 refX, f32 refY, f32 dirX, f32 dirY);
void fn_1_B234(NAMEKEYBOARD_BOUNDS *bounds);

NAMEKEYBOARD keyboardMain[] = {
    {80, 140, 'A'}, {110, 140, 'B'}, {140, 140, 'C'}, {170, 140, 'D'},
    {200, 140, 'E'}, {230, 140, 'F'}, {260, 140, 'G'}, {290, 140, 'H'},
    {320, 140, 'I'}, {350, 140, 'J'}, {380, 140, 'K'}, {410, 140, 'L'},
    {440, 140, 'M'}, {470, 140, 'N'},
    {80, 176, 'O'}, {110, 176, 'P'}, {140, 176, 'Q'}, {170, 176, 'R'},
    {200, 176, 'S'}, {230, 176, 'T'}, {260, 176, 'U'}, {290, 176, 'V'},
    {320, 176, 'W'}, {350, 176, 'X'}, {380, 176, 'Y'}, {410, 176, 'Z'},
    {440, 176, ' '}, {470, 176, ' '},
    {80, 212, 'a'}, {110, 212, 'b'}, {140, 212, 'c'}, {170, 212, 'd'},
    {200, 212, 'e'}, {230, 212, 'f'}, {260, 212, 'g'}, {290, 212, 'h'},
    {320, 212, 'i'}, {350, 212, 'j'}, {380, 212, 'k'}, {410, 212, 'l'},
    {440, 212, 'm'}, {470, 212, 'n'},
    {80, 248, 'o'}, {110, 248, 'p'}, {140, 248, 'q'}, {170, 248, 'r'},
    {200, 248, 's'}, {230, 248, 't'}, {260, 248, 'u'}, {290, 248, 'v'},
    {320, 248, 'w'}, {350, 248, 'x'}, {380, 248, 'y'}, {410, 248, 'z'},
    {440, 248, 194}, {470, 248, 195},
    {120, 293, NAMEKEYBOARD_PAGE_ALPHA},
    {200, 293, NAMEKEYBOARD_PAGE_SYMBOL},
    {348, 293, NAMEKEYBOARD_DELETE},
    {468, 293, NAMEKEYBOARD_CONFIRM},
    {0, 0, 0},
};

NAMEKEYBOARD keyboardNumSpc[] = {
    {114, 136, '1'}, {144, 136, '2'}, {174, 136, '3'}, {204, 136, '4'},
    {234, 136, '5'}, {264, 136, '6'}, {294, 136, '7'}, {324, 136, '8'},
    {354, 136, '9'}, {384, 136, '0'}, {414, 136, 134}, {444, 136, ' '},
    {114, 172, '['}, {144, 172, '\\'}, {174, 172, '~'}, {204, 172, 127},
    {234, 172, '<'}, {264, 172, '='}, {294, 172, '_'}, {324, 172, ']'},
    {354, 172, '^'}, {384, 172, 130}, {414, 172, 133}, {444, 172, '}'},
    {114, 208, NAME_SYMBOL_CODE | 3}, {144, 208, NAME_SYMBOL_CODE | 4},
    {174, 208, NAME_SYMBOL_CODE | 5}, {204, 208, NAME_SYMBOL_CODE | 6},
    {234, 208, NAME_SYMBOL_CODE | 9}, {264, 208, NAME_SYMBOL_CODE | 7},
    {294, 208, NAME_SYMBOL_CODE | 1}, {324, 208, NAME_SYMBOL_CODE | 11},
    {354, 208, ';'}, {384, 208, ':'},
    {414, 208, NAME_SYMBOL_CODE | 21}, {444, 208, NAME_SYMBOL_CODE | 22},
    {120, 293, NAMEKEYBOARD_PAGE_ALPHA},
    {200, 293, NAMEKEYBOARD_PAGE_SYMBOL},
    {348, 293, NAMEKEYBOARD_DELETE},
    {468, 293, NAMEKEYBOARD_CONFIRM},
    {0, 0, 0},
};
NAMEKEYBOARD *keyboardTbl[] = {keyboardMain, keyboardNumSpc, NULL, NULL};
NAME_SPRITE_CONFIG nameSpriteConfig[] = {
    {FILESEL_DATA_22, 0, 100, 0, 0},
    {FILESEL_DATA_28, 1, 120, 0, 0},
    {FILESEL_DATA_28, 6, 120, 4, -110},
    {FILESEL_DATA_28, 8, 120, -156, 107},
    {FILESEL_DATA_28, 8, 120, -76, 107},
    {FILESEL_DATA_28, 8, 120, -1000, -1000},
    {FILESEL_DATA_28, 2, 120, 60, 105},
    {FILESEL_DATA_28, 4, 120, 180, 105},
    {FILESEL_DATA_20, 0, 100, -190, -120},
    {FILESEL_DATA_20, 1, 100, 190, -120},
    {FILESEL_DATA_21, 0, 100, -230, -120},
    {FILESEL_DATA_21, 2, 100, 230, -120},
    {FILESEL_DATA_30, 0, 100, 60, 105},
    {FILESEL_DATA_30, 2, 100, 180, 105},
    {FILESEL_DATA_30, 4, 100, -156, 105},
    {FILESEL_DATA_30, 6, 100, -76, 105},
    {FILESEL_DATA_30, 6, 100, -1000, -1000},
    {FILESEL_DATA_23, 0, 125, 0, -4},
};

HUSPR_GROUPID lbl_1_bss_42E;
s16 nameKeyboardNum;
s16 lbl_1_bss_42A;
u8 nameDefault[NAME_RANDOM_MES_COUNT][FILESEL_FILENAME_LENGTH];
HUWINID lbl_1_bss_3AC[NAME_KEYBOARD_WINDOW_COUNT];
u16 nameMesTemp[NAME_MES_TEMP_LENGTH];
s16 nameCharNo;
char nameMes[NAME_MES_LENGTH];
NAMEKEYBOARD *keyboard;

void NameEnterInit(void)
{
    HUSPR_GROUPID grp;
    ANIMDATA *anim;
    HUSPRID spr;
    s16 k;
    NAME_SPRITE_CONFIG *config;
    u8 *msg;
    s16 i;

    grp = HuSprGrpCreate(NAME_SPRITE_COUNT);
    lbl_1_bss_42E = grp;
    for (i = 0; i < NAME_SPRITE_COUNT; i++) {
        config = &nameSpriteConfig[i];
        anim = HuSprAnimRead(HuDataSelHeapReadNum(config->dataNum, HU_MEMNUM_OVL, HEAP_MODEL));
        spr = HuSprCreate(anim, config->priority, config->bank);
        HuSprGrpMemberSet(grp, i, spr);
        HuSprPosSet(grp, i, config->x, config->y);
        HuSprAttrSet(grp, i, 4);
    }
    HuSprGrpPosSet(grp, 288.0f, 200.0f);
    for (i = 0; i < 2; i++) {
        HUWIN *w;
        lbl_1_bss_3AC[i] = HuWinCreate(40.0f, 110.0f, 500, 250, 0);
        w = &winData[lbl_1_bss_3AC[i]];
        HuWinBGTPLvlSet(lbl_1_bss_3AC[i], 0.0f);
        anim = HuSprAnimRead(HuDataSelHeapReadNum(FILESEL_DATA_31 + i, HU_MEMNUM_OVL, HEAP_MODEL));
        if (i == 0) {
            HuWinAnimSet(lbl_1_bss_3AC[i], anim, 0, 250.0f, 90.0f);
        } else {
            HuWinAnimSet(lbl_1_bss_3AC[i], anim, 0, 250.0f, 72.0f);
        }
        HuWinDispOff(lbl_1_bss_3AC[i]);
        HuWinPriSet(lbl_1_bss_3AC[i], 110);
    }
    lbl_1_bss_42A = 0;
    for (k = 0; k < NAME_RANDOM_MES_COUNT; k++) {
        for (i = 0; i < FILESEL_FILENAME_LENGTH; i++) {
            nameDefault[k][i] = 0;
        }
    }
    for (k = 0; k < NAME_RANDOM_MES_COUNT; k++) {
        msg = (u8 *)HuWinMesPtrGet(k);
        i = 0;
        while (*msg != 0) {
            if (*msg >= '0') {
                nameDefault[k][i++] = *msg;
            }
            msg++;
        }
    }
}

void fn_1_8F34(s32 unused, s16 keyNo)
{
    u16 btn;

    btn = keyboard[keyNo].val;
    HuSprPosSet(lbl_1_bss_42E, 0,
        24.0f + ((f32)keyboard[keyNo].x - 288.0f),
        ((f32)keyboard[keyNo].y - 200.0f) - 12.0f);
    if (btn >= NAMEKEYBOARD_PAGE_ALPHA && btn <= NAMEKEYBOARD_CONFIRM) {
        HuSprAttrSet(lbl_1_bss_42E, 1, HUSPR_ATTR_DISPOFF);
    } else {
        HuSprAttrReset(lbl_1_bss_42E, 1, HUSPR_ATTR_DISPOFF);
        HuSprPosSet(lbl_1_bss_42E, 1,
            10.0f + ((f32)keyboard[keyNo].x - 288.0f),
            12.0f + ((f32)keyboard[keyNo].y - 200.0f));
    }
}

s32 NameEnterMain(s16 boxNo)
{
    NAMEKEYBOARD_BOUNDS bounds;
    s16 keyNo;
    s16 j;
    s16 k;
    s16 keyboardNo;
    HUWINID nameWinId;
    s32 isCommandKey;
    s16 nextKeyNo;
    s16 prevKeyboard;
    u16 movedKeyValue;
    s32 result;
    u16 initialKeyValue;
    u16 pageKeyValue;
    u16 currentKeyValue;
    s16 nameLength;
    f32 dirX;
    f32 dirY;
    f32 refX;
    f32 refY;

    keyboardNo = 0;
    prevKeyboard = keyboardNo;
    keyNo = 0;
    isCommandKey = 0;
    nameCharNo = 0;
    nameMes[0] = NAME_MES_CURSOR;
    nameMes[1] = 0;
    keyboard = keyboardTbl[keyboardNo];

    initialKeyValue = keyboard[keyNo].val;
    HuSprPosSet(lbl_1_bss_42E, 0, 24.0f + ((f32)keyboard[keyNo].x - 288.0f),
                (f32)keyboard[keyNo].y - 200.0f - 12.0f);
    if (initialKeyValue >= NAMEKEYBOARD_PAGE_ALPHA && initialKeyValue <= NAMEKEYBOARD_CONFIRM) {
        HuSprAttrSet(lbl_1_bss_42E, 1, HUSPR_ATTR_DISPOFF);
    } else {
        HuSprAttrReset(lbl_1_bss_42E, 1, HUSPR_ATTR_DISPOFF);
        HuSprPosSet(lbl_1_bss_42E, 1, 10.0f + ((f32)keyboard[keyNo].x - 288.0f),
                    12.0f + ((f32)keyboard[keyNo].y - 200.0f));
    }

    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_ALPHA, 8);
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_SYMBOL, 8);
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_JAPANESE, 8);
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_DELETE_ICON, 2);
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_CONFIRM_ICON, 4);
    HuAudFXPlay(MSM_SE_CMN_01);
    NameEnterClose();
    HuWinMesSet(lbl_1_bss_2E0, FILE_SELECT_ENTER_NAME);
    HuWinMesSet(lbl_1_bss_2DE, MESSNUM(MESS_SYS_GUIDE, 2));
    HuWinDispOn(lbl_1_bss_2DE);
    nameWinId = HuWinCreate(-10000.0f, 70.0f, 176, 26, 0);
    HuWinBGTPLvlSet(nameWinId, 0.0f);
    HuWinMesSpeedSet(nameWinId, 0);
    HuWinPriSet(nameWinId, 40);
    HuWinMesSet(nameWinId, MESSNUM_PTR(nameMes));

    for (nameKeyboardNum = 0; keyboardTbl[nameKeyboardNum] != 0; nameKeyboardNum++) {
        ;
    }
    fn_1_B234(&bounds);

L_95A8:
    dirX = dirY = 0.0f;
    if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
        dirX = -1.0f;
    } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
        dirX = 1.0f;
    }
    if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
        dirY = -1.0f;
    } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
        dirY = 1.0f;
    }
    if (!dirX && !dirY) {
        goto L_99B4;
    }
    nextKeyNo = fn_1_ABF8(keyNo, (f32)keyboard[keyNo].x, (f32)keyboard[keyNo].y, dirX, dirY);
    if (nextKeyNo == -1) {
        if (dirX) {
            dirY = 0.0f;
            if (dirX < 0.0f) {
                refX = bounds.maxX;
            } else {
                refX = bounds.minX;
            }
            refY = (f32)keyboard[keyNo].y;
        } else {
            dirX = 0.0f;
            if (dirY < 0.0f) {
                refY = bounds.maxY;
            } else {
                refY = bounds.minY;
            }
            refX = (f32)keyboard[keyNo].x;
        }
        nextKeyNo = fn_1_ABF8(keyNo, refX, refY, dirX, dirY);
    }
    if (nextKeyNo == -1) {
        goto L_99B4;
    }
    HuAudFXPlay(MSM_SE_CMN_01);
    keyNo = nextKeyNo;
    movedKeyValue = keyboard[keyNo].val;
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_ALPHA, 8);
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_SYMBOL, 8);
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_JAPANESE, 8);
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_DELETE_ICON, 2);
    HuSprBankSet(lbl_1_bss_42E, NAME_SPR_CONFIRM_ICON, 4);
    if (movedKeyValue >= NAMEKEYBOARD_PAGE_ALPHA && movedKeyValue <= NAMEKEYBOARD_CONFIRM) {
        isCommandKey = 1;
        switch (movedKeyValue) {
        case NAMEKEYBOARD_PAGE_ALPHA:
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_ALPHA, 9);
            break;
        case NAMEKEYBOARD_PAGE_SYMBOL:
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_SYMBOL, 9);
            break;
        case NAMEKEYBOARD_PAGE_JAPANESE:
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_JAPANESE, 9);
            break;
        case NAMEKEYBOARD_DELETE:
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_DELETE_ICON, 3);
            break;
        case NAMEKEYBOARD_CONFIRM:
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_CONFIRM_ICON, 5);
            break;
        }
    } else {
        isCommandKey = 0;
    }

L_99B4:
    if (HuPadBtnDown[0] & PAD_BUTTON_START) {
        for (keyNo = 0; keyboard[keyNo].val != 0 && keyboard[keyNo].val != NAMEKEYBOARD_CONFIRM; keyNo++) {
        }
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_ALPHA, 8);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_SYMBOL, 8);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_JAPANESE, 8);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_DELETE_ICON, 2);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_CONFIRM_ICON, 5);
        isCommandKey = 1;
        j = fn_1_AAEC(nameWinId);
        if (j == 0) {
            goto L_A3D8;
        }
        nameCharNo = j;
        nameLength = j;
        for (j = k = 0; j < nameLength; k++, j++) {
            if (nameMes[j + 1] == NAME_MES_WIDE_CHAR_1 || nameMes[j + 1] == NAME_MES_WIDE_CHAR_2) {
                nameMesTemp[k] = (u16)((nameMes[j] << 8) | nameMes[j + 1]);
                nameCharNo--;
                j++;
            } else {
                nameMesTemp[k] = nameMes[j];
            }
        }
        goto L_A3D8;
    }
    if (HuPadBtnDown[0] & PAD_BUTTON_A) {
        if (keyboard[keyNo].val >= NAMEKEYBOARD_PAGE_ALPHA && keyboard[keyNo].val <= NAMEKEYBOARD_CONFIRM) {
            goto L_E84;
        }
        if (nameCharNo >= NAME_CHARACTER_LIMIT) {
            goto L_E84;
        }
        nameMesTemp[nameCharNo] = keyboard[keyNo].val;
        nameCharNo++;
        for (j = k = 0; j < nameCharNo; j++) {
            if (nameMesTemp[j] > 255) {
                nameMes[k++] = (u8)(nameMesTemp[j] >> 8);
                nameMes[k++] = nameMesTemp[j] & 255;
            } else {
                nameMes[k++] = nameMesTemp[j] & 255;
            }
        }
        if (nameCharNo < NAME_CHARACTER_LIMIT) {
            nameMes[k] = NAME_MES_CURSOR;
            nameMes[k + 1] = 0;
        } else {
            nameMes[k] = 0;
        }
        HuWinMesSet(nameWinId, MESSNUM_PTR(nameMes));
        if (nameCharNo < NAME_CHARACTER_LIMIT) {
            goto L_A088;
        }
        for (keyNo = 0; keyboard[keyNo].val != 0 && keyboard[keyNo].val != NAMEKEYBOARD_CONFIRM; keyNo++) {
        }
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_CONFIRM_ICON, 5);
        isCommandKey = 1;
        goto L_A088;
    L_E84:
        if (keyboard[keyNo].val == NAMEKEYBOARD_PAGE_ALPHA) {
            keyboardNo = 0;
            goto L_A088;
        }
        if (keyboard[keyNo].val == NAMEKEYBOARD_PAGE_SYMBOL) {
            keyboardNo = 1;
            goto L_A088;
        }
        if (keyboard[keyNo].val == NAMEKEYBOARD_PAGE_JAPANESE) {
            keyboardNo = 2;
            goto L_A088;
        }
        if (keyboard[keyNo].val == NAMEKEYBOARD_DELETE) {
            goto L_A0B0;
        }
        if (keyboard[keyNo].val != NAMEKEYBOARD_CONFIRM) {
            goto L_A088;
        }
        for (j = 0; nameMes[j] != 0; j++) {
            if (nameMes[j] == NAME_MES_CURSOR) {
                nameMes[j] = 0;
            }
        }
        HuAudFXPlay(MSM_SE_CMN_03);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_CONFIRM, 3);
        HuPrcSleep(10);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_CONFIRM, 2);
        if (fn_1_AAEC(nameWinId) != 0) {
            HuPrcSleep(30);
        }
        for (j = 0;; j++) {
            *((s8 *)&GwCommon.name[j]) = lbl_1_bss_D8[boxNo].fileName[j] = nameMes[j];
            if (nameMes[j] == 0) {
                break;
            }
        }
        result = 0;
        goto L_AA78;
    L_A088:
        HuAudFXPlay(MSM_SE_CMN_02);
        goto L_A3D8;
    }
    if (HuPadBtnDown[0] & PAD_BUTTON_B) {
    L_A0B0:
        nameCharNo--;
        if (nameCharNo < 0) {
            result = FILESEL_RESULT_CANCEL;
            goto L_AA78;
        }
        HuAudFXPlay(MSM_SE_CMN_04);
        for (j = k = 0; j < nameCharNo; j++) {
            if (nameMesTemp[j] > 255) {
                nameMes[k++] = (u8)(nameMesTemp[j] >> 8);
                nameMes[k++] = nameMesTemp[j] & 255;
            } else {
                nameMes[k++] = nameMesTemp[j] & 255;
            }
        }
        nameMes[k] = NAME_MES_CURSOR;
        nameMes[k + 1] = 0;
        HuWinMesSet(nameWinId, MESSNUM_PTR(nameMes));
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_DELETE, 1);
        HuPrcSleep(5);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_DELETE, 0);
        goto L_A3D8;
    }
    if (HuPadBtnDown[0] & PAD_TRIGGER_R) {
        keyboardNo++;
        if (keyboardNo >= nameKeyboardNum) {
            keyboardNo = 0;
        }
        for (keyNo = 0; keyboard[keyNo].val != 0; keyNo++) {
            if (keyboard[keyNo].val == (keyboardNo + 1) * NAMEKEYBOARD_PAGE_STEP) {
                break;
            }
        }
        isCommandKey = 0;
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_PAGE_RIGHT, 3);
        goto L_A3D8;
    }
    if (HuPadBtnDown[0] & PAD_TRIGGER_L) {
        keyboardNo--;
        if (keyboardNo < 0) {
            keyboardNo = nameKeyboardNum - 1;
        }
        for (keyNo = 0; keyboard[keyNo].val != 0; keyNo++) {
            if (keyboard[keyNo].val == (keyboardNo + 1) * NAMEKEYBOARD_PAGE_STEP) {
                break;
            }
        }
        isCommandKey = 0;
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_PAGE_LEFT, 1);
    }

L_A3D8:
    if (keyboardNo != prevKeyboard) {
        HuAudFXPlay(FILESEL_SFX_KEYBOARD_CHANGE);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_ALPHA, 8);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_SYMBOL, 8);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_JAPANESE, 8);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_DELETE_ICON, 2);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_CONFIRM_ICON, 4);
        if (keyboardNo == 0) {
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_ALPHA_TAB, 5);
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_ALPHA, 9);
        } else if (keyboardNo == 1) {
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_SYMBOL_TAB, 7);
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_SYMBOL, 9);
        } else {
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_JAPANESE_TAB, 9);
            HuSprBankSet(lbl_1_bss_42E, NAME_SPR_KEYBOARD_JAPANESE, 9);
        }
        k = keyboard[keyNo].val;
        HuWinDispOff(lbl_1_bss_3AC[prevKeyboard]);
        keyboard = keyboardTbl[keyboardNo];
        fn_1_B234(&bounds);
        HuWinDispOn(lbl_1_bss_3AC[keyboardNo]);
        for (keyNo = 0; keyboard[keyNo].val != 0; keyNo++) {
            if (k == keyboard[keyNo].val) {
                break;
            }
        }
        pageKeyValue = keyboard[keyNo].val;
        HuSprPosSet(lbl_1_bss_42E, 0, 24.0f + ((f32)keyboard[keyNo].x - 288.0f),
                    (f32)keyboard[keyNo].y - 200.0f - 12.0f);
        if (pageKeyValue >= NAMEKEYBOARD_PAGE_ALPHA && pageKeyValue <= NAMEKEYBOARD_CONFIRM) {
            HuSprAttrSet(lbl_1_bss_42E, 1, HUSPR_ATTR_DISPOFF);
        } else {
            HuSprAttrReset(lbl_1_bss_42E, 1, HUSPR_ATTR_DISPOFF);
            HuSprPosSet(lbl_1_bss_42E, 1, 10.0f + ((f32)keyboard[keyNo].x - 288.0f),
                        12.0f + ((f32)keyboard[keyNo].y - 200.0f));
        }
        prevKeyboard = keyboardNo;
        HuPrcSleep(10);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_PAGE_LEFT, 0);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_PAGE_RIGHT, 2);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_ALPHA_TAB, 4);
        HuSprBankSet(lbl_1_bss_42E, NAME_SPR_SYMBOL_TAB, 6);
    }

L_A868:
    currentKeyValue = keyboard[keyNo].val;
    HuSprPosSet(lbl_1_bss_42E, 0, 24.0f + ((f32)keyboard[keyNo].x - 288.0f),
                (f32)keyboard[keyNo].y - 200.0f - 12.0f);
    if (currentKeyValue >= NAMEKEYBOARD_PAGE_ALPHA && currentKeyValue <= NAMEKEYBOARD_CONFIRM) {
        HuSprAttrSet(lbl_1_bss_42E, 1, HUSPR_ATTR_DISPOFF);
    } else {
        HuSprAttrReset(lbl_1_bss_42E, 1, HUSPR_ATTR_DISPOFF);
        HuSprPosSet(lbl_1_bss_42E, 1, 10.0f + ((f32)keyboard[keyNo].x - 288.0f),
                    12.0f + ((f32)keyboard[keyNo].y - 200.0f));
    }
    HuPrcVSleep();
    goto L_95A8;

L_AA78:
    HuWinHomeClear(lbl_1_bss_2DE);
    HuWinHomeClear(lbl_1_bss_2E0);
    HuWinKill(nameWinId);
    HuAudFXPlay(MSM_SE_CMN_01);
    FileCommonInit();
    HuPrcVSleep();
    return result;
}

s16 fn_1_AAEC(s16 nameWinId)
{
    s16 nameLength;
    s16 i;

    for (i = nameLength = 0; nameMes[i] != 0; i++) {
        if (nameMes[i] != 0 && nameMes[i] != NAME_MES_CONTROL_CHAR && nameMes[i] != ' ') {
            nameLength++;
        }
    }
    if (nameLength == 0) {
        strcpy(nameMes, (char *)nameDefault[frandmod(NAME_RANDOM_MES_COUNT)]);
        HuWinMesSet(nameWinId, MESSNUM_PTR(nameMes));
        return (s16)strlen(nameMes);
    }
    return 0;
}

s16 fn_1_ABF8(s16 id, f32 refX, f32 refY, f32 dirX, f32 dirY)
{
    f32 ndx, ndy;
    f32 len;
    f32 px, py;
    f32 dist;
    f32 angDist;
    f32 best;
    s16 i;
    s16 bestIdx;

    len = sqrtf(dirX * dirX + dirY * dirY);
    ndx = dirX / len;
    ndy = dirY / len;
    best = 409600.0f;
    bestIdx = -1;
    for (i = 0; keyboard[i].val != 0; i++) {
        if (i != id) {
            px = (f32)keyboard[i].x - refX;
            py = (f32)keyboard[i].y - refY;
            dist = sqrtf(px * px + py * py);
            px = px / dist;
            py = py / dist;
            angDist = sqrtf((px - ndx) * (px - ndx) + (py - ndy) * (py - ndy));
            if (keyboard[id].val >= NAMEKEYBOARD_PAGE_ALPHA && keyboard[id].val <= NAMEKEYBOARD_CONFIRM &&
                keyboard[i].val >= NAMEKEYBOARD_PAGE_ALPHA && keyboard[i].val <= NAMEKEYBOARD_CONFIRM &&
                angDist < 0.1 && dist < 55.0f + best) {
                best = dist;
                bestIdx = i;
            } else if (angDist < 0.8 && dist < best) {
                best = dist;
                bestIdx = i;
            }
        }
    }
    return bestIdx;
}

void fn_1_B234(NAMEKEYBOARD_BOUNDS *bounds)
{
    s16 i;

    bounds->minX = bounds->minY = 10000.0f;
    bounds->maxX = bounds->maxY = -10000.0f;
    for (i = 0; keyboard[i].val != 0; i++) {
        if ((f32)keyboard[i].x < bounds->minX) {
            bounds->minX = (f32)keyboard[i].x;
        }
        if ((f32)keyboard[i].y < bounds->minY) {
            bounds->minY = (f32)keyboard[i].y;
        }
        if ((f32)keyboard[i].x > bounds->maxX) {
            bounds->maxX = (f32)keyboard[i].x;
        }
        if ((f32)keyboard[i].y > bounds->maxY) {
            bounds->maxY = (f32)keyboard[i].y;
        }
    }
    bounds->minX -= 20.0f;
    bounds->minY -= 20.0f;
    bounds->maxX += 20.0f;
    bounds->maxY += 20.0f;
}

void NameEnterClose(void)
{
    s16 i;

    for (i = 0; i < 18; i++) {
        HuSprAttrReset(lbl_1_bss_42E, i, HUSPR_ATTR_DISPOFF);
        HuSprTPLvlSet(lbl_1_bss_42E, i, 1.0f);
    }
    HuWinDispOn(lbl_1_bss_3AC[0]);
}
