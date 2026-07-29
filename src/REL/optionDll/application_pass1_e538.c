#define _MATH_H

#include "dolphin.h"
#include "game/audio.h"
#include "game/process.h"
#include "game/window.h"

typedef struct OptionAudioEntry {
    s8 type;
    u8 pad[11];
    s32 id;
} OptionAudioEntry;

typedef struct OptionMessageEntry {
    s8 type;
    u8 pad[7];
    s32 message;
    s32 value;
} OptionMessageEntry;

extern s16 lbl_1_bss_AE0[7][2];
extern OptionMessageEntry *lbl_1_bss_AFC;
extern char lbl_1_data_1AA8[];

void fn_1_E538(s16 page)
{
    OptionMessageEntry *entry = lbl_1_bss_AFC;
    s16 entryCount;
    s16 pageCount;
    s16 row;
    s16 column;

    for (entryCount = 0; entry->type != -1; entryCount++, entry++) {
    }
    pageCount = (entryCount + 1) / 2;
    entry = &lbl_1_bss_AFC[page * 2];
    for (row = 0; row < 7; row++) {
        for (column = 0; column < 2; column++, entry++) {
            if (column + (row * 2) + (page * 2) >= entryCount) {
                HuWinMesSet(lbl_1_bss_AE0[row][column],
                    (s32)lbl_1_data_1AA8);
            } else {
                HuWinMesSet(lbl_1_bss_AE0[row][column], entry->message);
            }
        }
    }
}

void fn_1_E688(OptionAudioEntry *entry)
{
    HuAudSStreamAllFadeOut(150);
    HuAudStreamFadeOut(10);
    if (entry->type == 0) {
        HuPrcSleep(30);
        HuAudSStreamPlay((s16)entry->id);
    } else {
        HuAudFXPlay(entry->id);
        HuPrcSleep(20);
    }
}
