/* Builds glyph sprites for the localized minigame title shown on the instruction screen. */
#include <math.h>
#include "dolphin.h"
#include "dolphin/gx.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/mgdata.h"
#include "game/object.h"
#include "game/process.h"
#include "game/window.h"
#include "datadir_enum.h"
#include "game/charman.h"
#include "game/thpmain.h"
#include "game/wipe.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "dolphin/os.h"
#include "humath.h"
#include "REL/instDll.h"

/* Encoded character-name glyph mapping and spacing constants. */
#define INST_NAME_DATA_YOFFSET_BIT (1U << 31) /* Mark glyphs drawn four units lower. */
#define INST_NAME_LOWER_GLYPH_DATA(number) (INST_NAME_DATA_YOFFSET_BIT | DATANUM(0x260000, number))
enum {
    INST_NAME_SPACING_CODE = 16,
    INST_NAME_FIRST_GLYPH_CODE = 48
};

typedef struct {
    s16 characterMotionIds[10]; /* Loaded character motions used by the preview. */
} InstDllMotionRecord;

/* Shared instruction-screen state and resources. */

/* Maps encoded message characters to their sprite resources. */
#define INST_NAME_NO_GLYPH 0
#define INST_NAME_DEFAULT_GLYPH DATANUM(0x260000, 42)
u32 lbl_1_data_F8[] = {
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_NO_GLYPH, INST_NAME_NO_GLYPH,
    INST_NAME_DEFAULT_GLYPH, DATANUM(0x260000, 43),
    DATANUM(0x260000, 44), DATANUM(0x260000, 45),
    DATANUM(0x260000, 46), DATANUM(0x260000, 47),
    DATANUM(0x260000, 48), DATANUM(0x260000, 49),
    DATANUM(0x260000, 50), DATANUM(0x260000, 51),
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, DATANUM(0x260000, 52),
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, DATANUM(0x260000, 58),
    DATANUM(0x260000, 59), DATANUM(0x260000, 60),
    DATANUM(0x260000, 61), DATANUM(0x260000, 62),
    DATANUM(0x260000, 63), DATANUM(0x260000, 64),
    DATANUM(0x260000, 65), DATANUM(0x260000, 66),
    DATANUM(0x260000, 67), DATANUM(0x260000, 68),
    DATANUM(0x260000, 69), DATANUM(0x260000, 70),
    DATANUM(0x260000, 71), DATANUM(0x260000, 72),
    DATANUM(0x260000, 73), DATANUM(0x260000, 74),
    DATANUM(0x260000, 75), DATANUM(0x260000, 76),
    DATANUM(0x260000, 77), DATANUM(0x260000, 78),
    DATANUM(0x260000, 79), DATANUM(0x260000, 80),
    DATANUM(0x260000, 81), DATANUM(0x260000, 82),
    DATANUM(0x260000, 83), DATANUM(0x260000, 72),
    DATANUM(0x260000, 55), DATANUM(0x260000, 72),
    DATANUM(0x260000, 72), DATANUM(0x260000, 72),
    DATANUM(0x260000, 72), INST_NAME_LOWER_GLYPH_DATA(84),
    INST_NAME_LOWER_GLYPH_DATA(85), INST_NAME_LOWER_GLYPH_DATA(86),
    INST_NAME_LOWER_GLYPH_DATA(87), INST_NAME_LOWER_GLYPH_DATA(88),
    INST_NAME_LOWER_GLYPH_DATA(89), INST_NAME_LOWER_GLYPH_DATA(90),
    INST_NAME_LOWER_GLYPH_DATA(91), INST_NAME_LOWER_GLYPH_DATA(92),
    INST_NAME_LOWER_GLYPH_DATA(93), INST_NAME_LOWER_GLYPH_DATA(94),
    INST_NAME_LOWER_GLYPH_DATA(95), INST_NAME_LOWER_GLYPH_DATA(96),
    INST_NAME_LOWER_GLYPH_DATA(97), INST_NAME_LOWER_GLYPH_DATA(98),
    INST_NAME_LOWER_GLYPH_DATA(99), INST_NAME_LOWER_GLYPH_DATA(100),
    INST_NAME_LOWER_GLYPH_DATA(101), INST_NAME_LOWER_GLYPH_DATA(102),
    INST_NAME_LOWER_GLYPH_DATA(103), INST_NAME_LOWER_GLYPH_DATA(104),
    INST_NAME_LOWER_GLYPH_DATA(105), INST_NAME_LOWER_GLYPH_DATA(106),
    INST_NAME_LOWER_GLYPH_DATA(107), INST_NAME_LOWER_GLYPH_DATA(108),
    INST_NAME_LOWER_GLYPH_DATA(109), INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    DATANUM(0x260000, 56), INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    DATANUM(0x260000, 52), INST_NAME_LOWER_GLYPH_DATA(57),
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH,
    INST_NAME_LOWER_GLYPH_DATA(53), INST_NAME_LOWER_GLYPH_DATA(54),
    INST_NAME_DEFAULT_GLYPH, INST_NAME_DEFAULT_GLYPH
};
u8 lbl_1_data_410[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    12, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 22, 30, 30,
    30, 30, 30, 30, 30, 28, 28, 30, 30, 20, 30, 30, 30, 32, 30, 32,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 16, 30, 30, 30,
    12, 24, 22, 24, 22, 24, 24, 22, 24, 16, 20, 22, 16, 24, 24, 24,
    24, 24, 22, 24, 22, 24, 24, 24, 24, 24, 24, 30, 30, 30, 30, 30,
    30, 30, 16, 30, 30, 16, 30, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 18, 24, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 24, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30
};

/* Called by fn_1_47E4 to build the localized minigame name from its message. */
HUSPR_GROUPID fn_1_4400(u32 messageId)
{
    char *nameMessage;
    s16 nameWidth;
    s16 *glyphYOffset;
    s16 glyphIndex;
    s16 *glyphXOffset;
    ANIMDATA **glyphAnimation;
    HUSPR_GROUPID nameGroupId;
    HUSPR_GROUP *nameGroup;
    HUSPRID glyphSpriteId;
    s16 characterCode;

    {
    s16 glyphCount;
    {
    u32 *glyphDataByCode;
    glyphDataByCode = lbl_1_data_F8;
    glyphAnimation = HuMemDirectMalloc(0, 400);
    glyphXOffset = HuMemDirectMalloc(0, 200);
    glyphYOffset = HuMemDirectMalloc(0, 200);
    nameMessage = HuWinMesPtrGet(messageId);
    nameWidth = 0;
    glyphCount = 0;

    while (*nameMessage != '\0') {
        characterCode = *nameMessage;
        if (*nameMessage == ' ' || *nameMessage == INST_NAME_SPACING_CODE) {
            nameWidth += lbl_1_data_410[*nameMessage];
        } else if (*nameMessage >= INST_NAME_FIRST_GLYPH_CODE) {
            /* Mark glyphs drawn four units lower. */
            u32 glyphDataAndOffset;
            glyphDataAndOffset = glyphDataByCode[*nameMessage];
            glyphAnimation[glyphCount] =
                HuSprAnimDataRead(glyphDataAndOffset & ~INST_NAME_DATA_YOFFSET_BIT);
            glyphXOffset[glyphCount] = nameWidth;
            glyphYOffset[glyphCount] = 0;
            nameWidth += lbl_1_data_410[characterCode];
            if (glyphDataAndOffset & INST_NAME_DATA_YOFFSET_BIT) {
                glyphYOffset[glyphCount] = 4;
            }
            glyphCount++;
        }
        nameMessage++;
    }

    }

    nameGroupId = HuSprGrpCreate(glyphCount);
    nameGroup = &HuSprGrpData[nameGroupId];
    nameGroup->work[0] = nameWidth;
    nameWidth = (nameWidth / 2) - 14;
    for (glyphIndex = 0; glyphIndex < glyphCount; glyphIndex++) {
        glyphSpriteId = HuSprCreate(glyphAnimation[glyphIndex], 0, 0);
        HuSprGrpMemberSet(nameGroupId, glyphIndex, glyphSpriteId);
        HuSprPosSet(nameGroupId, glyphIndex,
                    glyphXOffset[glyphIndex] - nameWidth, glyphYOffset[glyphIndex]);
    }

    HuMemDirectFree(glyphAnimation);
    HuMemDirectFree(glyphXOffset);
    HuMemDirectFree(glyphYOffset);
    return nameGroupId;
    }
}
