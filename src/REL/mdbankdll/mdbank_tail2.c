#include <dolphin/mtx/GeoTypes.h>

typedef struct MdbankMoveWork {
    s16 active;
    s16 unk_02;
    float time;
    float duration;
    Vec start;
    Vec control;
    Vec end;
} MDBANK_MOVE_WORK;

typedef struct MdbankObject {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    void (*objFunc)(struct MdbankObject *obj);
    Vec trans;
    Vec rot;
    Vec scale;
    u16 modelCount;
    s16 *mdlId;
} MDBANK_OBJECT;

extern const float lbl_1_rodata_68;
extern const float lbl_1_rodata_74;
extern const Vec lbl_1_rodata_200;
extern MDBANK_MOVE_WORK lbl_1_bss_1C0;
extern MDBANK_OBJECT *lbl_1_bss_1C;
extern MDBANK_OBJECT *lbl_1_bss_20;
extern MDBANK_OBJECT *lbl_1_bss_24[2];
extern Vec lbl_1_bss_210[20];

float fn_1_11458(float start, float end, float time, float duration);
void fn_1_11678(Vec *out, const Vec *start, const Vec *control,
    const Vec *end, float weight);
void fn_1_11980(float x, float y, float z);
void fn_1_12A94(float x, float y, float z);

void fn_1_6F00(void)
{
    Vec pos = lbl_1_rodata_200;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_1C0;
    s16 i;

    if (work->active != 0) {
        fn_1_11678(&pos, &work->start, &work->control, &work->end,
            fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74,
                work->time, work->duration));
        Hu3DModelPosSet(lbl_1_bss_1C->mdlId[0], pos.x, pos.y, pos.z);
        for (i = 0; i < 12; i++) {
            Hu3DModelPosSet(lbl_1_bss_24[0]->mdlId[i], pos.x, pos.y, pos.z);
        }
        fn_1_11980(pos.x, pos.y, pos.z);
        fn_1_12A94(pos.x, pos.y, pos.z);
        for (i = 0; i < 20; i++) {
            Hu3DModelPosSet(lbl_1_bss_20->mdlId[i],
                pos.x + lbl_1_bss_210[i].x,
                pos.y + lbl_1_bss_210[i].y,
                pos.z + lbl_1_bss_210[i].z);
        }
        if ((work->time += lbl_1_rodata_74) > work->duration) {
            work->active = 0;
        }
    }
}
