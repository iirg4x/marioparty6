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
extern const float lbl_1_rodata_118;
extern MDBANK_MOVE_WORK lbl_1_bss_171C;

float fn_1_11458(float start, float end, float time, float duration);
void fn_1_11678(Vec *out, const Vec *start, const Vec *control,
    const Vec *end, float weight);

void fn_1_226C(MDBANK_OBJECT *obj)
{
    Vec pos;
    MDBANK_MOVE_WORK *work = &lbl_1_bss_171C;
    float rotation;

    fn_1_11678(&pos, &work->start, &work->control, &work->end,
        fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_74,
            work->time, work->duration));
    Hu3DModelPosSetV(obj->mdlId[0], &pos);
    rotation = fn_1_11458(lbl_1_rodata_68, lbl_1_rodata_118,
        work->time, work->duration);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_68,
        rotation, lbl_1_rodata_68);
    if ((work->time += lbl_1_rodata_74) > work->duration) {
        obj->objFunc = 0;
    }
}
