#include "dolphin.h"
#include "game/frand.h"
#include "game/process.h"

#define OPENING_CHAR_COUNT 10

void fn_1_470C(s16 animIndex, s16 frameIndex);

void fn_1_4068(u32 frameCount)
{
    s16 i;
    s16 j;
    s16 timer[OPENING_CHAR_COUNT];
    s16 count[OPENING_CHAR_COUNT];

    for (i = 0; i < OPENING_CHAR_COUNT; i++) {
        timer[i] = (s16)frandmod(30);
        count[i] = 1;
    }

    for (j = 0; j < frameCount - 10; j++) {
        for (i = 0; i < OPENING_CHAR_COUNT; i++) {
            if (timer[i] == 0) {
                fn_1_470C((s16)i, (s16)((i * 7) + (count[i] & 1)));
                count[i]++;
                timer[i] = (s16)(frandmod(20) + 30);
            }
            timer[i]--;
        }
        HuPrcVSleep();
    }

    HuPrcSleep(11);
}

void fn_1_41B4(void)
{
    s16 i;

    for (i = 0; i < OPENING_CHAR_COUNT; i++) {
        fn_1_470C((s16)i, (s16)((i * 7) + 5));
    }
    HuPrcSleep(10);
}

void fn_1_420C(void)
{
    s16 i;

    for (i = 0; i < OPENING_CHAR_COUNT; i++) {
        fn_1_470C((s16)i, (s16)((i * 7) + 6));
    }
    HuPrcSleep(10);
}
