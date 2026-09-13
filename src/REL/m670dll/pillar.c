#include "REL/m670dll.h"

float lbl_1_bss_480;
float lbl_1_bss_47C;
/* Only the first 24 timer entries are updated by the pillar loops. */
int lbl_1_bss_418[25];
float lbl_1_bss_3B8[24];
#include "game/audio.h"
#include "game/frand.h"
#include "game/mg/seqman.h"

void fn_1_2A24(unsigned int state)
{
    int i;
    switch (state) {
    case 1:
        lbl_1_bss_10.pillarObject->work[0] = 0;
        lbl_1_bss_480 = -1500.0f;
        lbl_1_bss_47C = 16.0f;
        {
            int type = lbl_1_bss_10.pillarObject->work[0];
            s8 *pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
            HU3D_MODELID model;
            for (i = 0; i < 24; pattern++, i++) {
                if (*pattern == type) {
                    model = lbl_1_bss_10.pillarModel[i];
                    Hu3DMotionSet(model, lbl_1_bss_10.pillarMotion[i][1]);
                    Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP | HU3D_ATTR_DISPOFF);
                }
            }
        }
        break;
    case 2:
        {
            s8 *pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
            OSReport("shout ... %d\n", lbl_1_bss_10.selectedWord);
            for (i = 0; i < 24; pattern++, i++) {
                if (*pattern != lbl_1_bss_10.selectedWord) {
                    Hu3DMotionSet(lbl_1_bss_10.pillarModel[i], lbl_1_bss_10.pillarMotion[i][1]);
                }
            }
            lbl_1_bss_10.selectedWord = -1;
        }
        break;
    case 4:
        {
            s8 *pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
            OSReport("PILLARF_LAMP\n");
            for (i = 0; i < 24; pattern++, i++) {
                if (*pattern != lbl_1_bss_10.selectedWord) {
                    Hu3DMotionSet(lbl_1_bss_10.pillarModel[i], lbl_1_bss_10.pillarMotion[i][3]);
                }
            }
            HuAudFXPlay(2199);
        }
        break;
    case 5:
        if (lbl_1_bss_10.speedLevel < 10) {
            lbl_1_bss_10.speedLevel++;
        }
        lbl_1_bss_10.pillarObject->work[0] = 0;
        lbl_1_bss_480 = 100.0f;
        break;
    case 6:
        lbl_1_bss_10.pillarObject->work[0] = 0;
        for (i = 0; i < 24; i++) {
            lbl_1_bss_3B8[i] = lbl_1_bss_480;
            lbl_1_bss_418[i] = -frandmod(60);
        }
        break;
    case 7:
        lbl_1_bss_10.pillarObject->work[0] = 120;
        break;
    case 3:
        lbl_1_bss_10.pillarObject->work[0] = 0;
        for (i = 0; i < 24; i++) {
            lbl_1_bss_3B8[i] = -1500.0f;
            lbl_1_bss_418[i] = 0;
        }
        break;
    }
    lbl_1_bss_10.state = state;
}

int fn_1_2DE8(HuVecF *pos)
{
    int i, nearest;
    HuVecF origin, pillarPos, delta;
    float minDist, dist;
    origin = *pos;
    nearest = -1;
    minDist = 99999.0f;
    origin.y = 0.0f;
    for (i = 0; i < 24; i++) {
        pillarPos = lbl_1_bss_10.pillarPos[i];
        pillarPos.y = 0.0f;
        PSVECSubtract(&pillarPos, &origin, &delta);
        dist = PSVECMag(&delta);
        if (dist < minDist) {
            nearest = i;
            minDist = dist;
        }
    }
    return nearest;
}

float fn_1_2EF4(int pillarNo)
{
    HU3D_MODELID model = lbl_1_bss_10.pillarModel[pillarNo];
    HuVecF pos;
    Hu3DModelPosGet(model, &pos);
    return pos.y;
}

int fn_1_2F44(HuVecF *pos)
{
    int pillarNo;
    HU3D_MODELID model;
    HuVecF pillarPos;
    pillarNo = fn_1_2DE8(pos);
    model = lbl_1_bss_10.pillarModel[pillarNo];
    Hu3DModelPosGet(model, &pillarPos);
    return 100.0f - pillarPos.y < 1.0f;
}

void fn_1_3098(OMOBJ *obj)
{
    int i;
    if (MgSeqModeGet() >= 5 && MgTimerDoneCheck(lbl_1_bss_10.timer)) {
        return;
    }
    switch (lbl_1_bss_10.state) {
    case 1:
    {
        int done, type;
        s8 *pattern;
        HU3D_MODELID model;
        lbl_1_bss_480 += lbl_1_bss_47C;
        if (lbl_1_bss_47C != 0.0f) {
            if (lbl_1_bss_47C > 1.0f) {
                lbl_1_bss_47C -= 0.065f;
            } else {
                lbl_1_bss_47C = 1.0f;
            }
        }
        if (lbl_1_bss_480 > 100.0f) {
            lbl_1_bss_480 = 100.0f;
            if (lbl_1_bss_47C != 0.0f) {
                for (i = 0; i < 4; i++) {
                    omVibrate(i, 20, 4, 4);
                }
                HuAudFXPlay(2201);
            }
            lbl_1_bss_47C = 0.0f;
        }
        for (i = 0; i < 24; i++) {
            fn_1_22F0(i, lbl_1_bss_480);
        }
        done = 1;
        type = obj->work[0];
        pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
        switch ((int)obj->work[1]) {
        case 0:
            for (i = 0; i < 24; pattern++, i++) {
                if (*pattern == type) {
                    model = lbl_1_bss_10.pillarModel[i];
                    if (!Hu3DMotionEndCheck(model)) {
                        done = 0;
                    }
                }
            }
            if (done) {
                pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
                for (i = 0; i < 24; pattern++, i++) {
                    if (*pattern == type) {
                        model = lbl_1_bss_10.pillarModel[i];
                        Hu3DMotionSet(model, lbl_1_bss_10.pillarMotion[i][3]);
                    }
                }
                if (type > 2) {
                    obj->work[1] = 2;
                } else {
                    obj->work[1] = 1;
                }
            }
            break;
        case 1:
            for (i = 0; i < 24; pattern++, i++) {
                if (*pattern == type) {
                    model = lbl_1_bss_10.pillarModel[i];
                    if (!Hu3DMotionEndCheck(model)) {
                        done = 0;
                    }
                }
            }
            if (done) {
                type = (type + 1) % 6;
                pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
                for (i = 0; i < 24; pattern++, i++) {
                    if (*pattern == type) {
                        model = lbl_1_bss_10.pillarModel[i];
                        Hu3DMotionSet(model, lbl_1_bss_10.pillarMotion[i][1]);
                    }
                }
                obj->work[0] = type;
                obj->work[1] = 0;
            }
            break;
        }
        break;
    }
    case 4:
    {
        s8 *pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
        int done = 1;
        for (i = 0; i < 24; pattern++, i++) {
            if (*pattern != lbl_1_bss_10.selectedWord && !Hu3DMotionEndCheck(lbl_1_bss_10.pillarModel[i])) {
                done = 0;
            }
        }
        /* Retail computes this flag but advances without testing it. */
        fn_1_2A24(5);
        break;
    }
    case 5:
    {
        float frame = obj->work[0];
        s8 *pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
        float height = lbl_1_bss_480;
        height -= 4.0f * lbl_1_bss_10.speedLevel;
        if (height < 50.0f) {
            height = 50.0f;
        }
        lbl_1_bss_480 = height;
        for (i = 0; i < 24; pattern++, i++) {
            if (*pattern != lbl_1_bss_10.selectedWord) {
                fn_1_22F0(i, lbl_1_bss_480);
            }
        }
        obj->work[0]++;
        if (obj->work[0] == 40 / lbl_1_bss_10.speedLevel) {
            fn_1_2A24(6);
        }
        break;
    }
    case 6:
    {
        s8 *pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
        int done = 1;
        if (!MgTimerDoneCheck(lbl_1_bss_10.timer)) {
            for (i = 0; i < 24; pattern++, i++) {
                if (*pattern != lbl_1_bss_10.selectedWord) {
                    int oldTime;
                    if (lbl_1_bss_418[i] > 0.0f) {
                        lbl_1_bss_3B8[i] -= 4.0f * lbl_1_bss_10.speedLevel * lbl_1_bss_418[i];
                        fn_1_22F0(i, lbl_1_bss_3B8[i]);
                    }
                    if (lbl_1_bss_3B8[i] >= -1500.0f) {
                        done = 0;
                    }
                    oldTime = lbl_1_bss_418[i];
                    lbl_1_bss_418[i] += 1.0f;
                    if (oldTime < 0 && lbl_1_bss_418[i] >= 0) {
                        fn_1_15B8(2200, &lbl_1_bss_10.pillarPos[i]);
                    }
                }
            }
        }
        if (done) {
            fn_1_2A24(7);
        }
        break;
    }
    case 7:
        if (obj->work[0] == 0) {
            fn_1_2A24(3);
            break;
        }
        if (obj->work[0] == 30) {
            HuAudFXPlay(2198);
            HuAudFXPlay(2203);
        }
        obj->work[0]--;
        break;
    case 3:
    {
        s8 *pattern = lbl_1_data_250[lbl_1_bss_10.pattern];
        int done = 1;
        float height;
        for (i = 0; i < 24; pattern++, i++) {
            if (*pattern != lbl_1_bss_10.selectedWord) {
                if (lbl_1_bss_418[i] > 0.0f) {
                    height = lbl_1_bss_3B8[i];
                    height += 0.82f * lbl_1_bss_418[i];
                    if (height > 100.0f) {
                        height = 100.0f;
                    }
                    lbl_1_bss_3B8[i] = height;
                    fn_1_22F0(i, lbl_1_bss_3B8[i]);
                }
                if (lbl_1_bss_3B8[i] != 100.0f) {
                    done = 0;
                }
            }
            lbl_1_bss_418[i]++;
        }
        if (done) {
            fn_1_2460(lbl_1_bss_10.soloPlayer, 1);
            fn_1_2A24(2);
            HuAudFXPlay(2201);
        } else {
            obj->work[0]++;
        }
        break;
    }
    }
}
