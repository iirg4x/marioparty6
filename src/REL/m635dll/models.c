#include "REL/m635dll.h"
#include "game/audio.h"

extern s16 lbl_1_data_1A8[20][8];

void fn_1_3AC4(s16 team)
{
    M635Model *model;
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    if (work->unk_0E < 10) {
        work->unk_0E++;
    }
    if (work->unk_14.time >= 430) {
        model = &work->unk_14;
        model->time = 401;
        Hu3DMotionTimeSet(model->model, model->time);
        Hu3DMotionSpeedSet(model->model, 1.0f);
    }
}

void fn_1_3B7C(s16 team, s16 member)
{
    M635Model *model;
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    if (member != 0) {
        model = &work->unk_18;
    } else {
        model = &work->unk_1C;
    }
    if (model->time >= 430) {
        model->time = 401;
        Hu3DMotionTimeSet(model->model, model->time);
        Hu3DMotionSpeedSet(model->model, 1.0f);
    }
}

void fn_1_3C34(void)
{
    M635Model *model;
    s16 step;
    int team;
    M635Team *work;

    for (team = 0; team < 2; team++) {
        work = &lbl_1_bss_4.team[team];
        step = work->unk_0E;
        if (step != 0) {
            model = &work->unk_14;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][0]) {
                if (lbl_1_data_1A8[step - 1][0] < 400 || lbl_1_data_1A8[step - 1][0] > 700) {
                    model->time = lbl_1_data_1A8[step - 1][0];
                    Hu3DMotionTimeSet(model->model, model->time);
                    if (team == 0) {
                        HuAudFXPlay(1859);
                    } else {
                        HuAudFXPlay(1860);
                    }
                }
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][1]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            if (model->time == 769) {
                Hu3DMotionSpeedSet(lbl_1_bss_4.team[team].unk_20, 1.0f);
            } else if (model->time == 710) {
                if (team == 0) {
                    HuAudFXPlay(1861);
                } else {
                    HuAudFXPlay(1862);
                }
            }
            model = &work->unk_18;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][2]) {
                model->time = lbl_1_data_1A8[step - 1][2];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][3]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            model = &work->unk_1C;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][4]) {
                model->time = lbl_1_data_1A8[step - 1][4];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][5]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            model = &work->unk_10;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][6]) {
                model->time = lbl_1_data_1A8[step - 1][6];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][7]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
        }
    }
}
