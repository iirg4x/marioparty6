/* Creates and updates the Lunar-tics arena, spaceship, and background scene. */
#include "REL/m657Dll.h"
#include "game/memory.h"
#include "game/audio.h"
#include "string.h"

/* Arena entrance sound-effect resource IDs. */



OMOBJ *lbl_1_bss_28;

/* Twelve zero-filled bytes with no demonstrated gameplay use in this module. */
u8 lbl_1_data_80[12] = {0};

char lbl_1_data_8C[5][10] = {
    "col", "post_R", "post_L", "post_R_C", "post_L_C"
};

/* Creates the arena object with automatic model transform syncing disabled and allocates its scene
 * state. */
void fn_1_15C0(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657ArenaWork *arenaWork;

    obj = fn_8003AB38(objman, 30, 6, 3, -1, fn_1_1B84);
    lbl_1_bss_28 = obj;
    obj->stat |= (1 << 8);
    arenaWork = obj->data = HuMemDirectMallocNum(HEAP_HEAP,
        sizeof(M657ArenaWork), 0x10000000);
    memset(arenaWork, 0, sizeof(M657ArenaWork));
}

void fn_1_1658(void)
{
}

/* Creates the arena background model and applies its initial camera and color. */
void fn_1_165C(OMOBJ *obj)
{
    M657ArenaWork *arenaWork = obj->data;
    HU3D_MODELID model = 0;
    HU3D_MOTIONID motion = 0;

    model = obj->mdlId[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 6), 0x10000000, 2));
    fn_80026B8C(model, 1);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026B64(model, 3);
    fn_8002C2AC(1);
    Hu3DBGColorSet(0, 0, 50);
}

/* Creates the arena floor and the spaceship door model with its motion. */
void fn_1_1720(void)
{
    M657ArenaItem *item;
    OMOBJ *obj = lbl_1_bss_28;
    HU3D_MODELID model;
    M657ArenaWork *arenaWork = obj->data;
    HU3D_MOTIONID motion;

    item = &arenaWork->item;
    item->pos.x = 0.0f;
    item->pos.y = 2573.0f;
    item->pos.z = 0.0f;
    item->unknownValue = 0;
    model = obj->mdlId[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 0), 0x10000000, 2));
    fn_80026760(model, item->pos.x, item->pos.y, item->pos.z);
    fn_80026B8C(model, 1);
    fn_80026B64(model, 3);
    model = obj->mdlId[2] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 2), 0x10000000, 2));
    motion = obj->mtnId[0] = fn_80030458(model,
        fn_80008DD0(DATANUM(0x740000, 3), 0x10000000, 2));
    fn_8002C95C(model, motion);
    fn_8002D86C(model, 0.0f);
    fn_80026B8C(model, 1);
    fn_80026B64(model, 3);
    fn_8002741C(obj->mdlId[1], "spaceship00-door_target", model);
}

/* Creates the looping arena decoration and places it above the playfield. */
void fn_1_1884(OMOBJ *obj)
{
    HU3D_MODELID model = 0;
    HU3D_MOTIONID motion = 0;

    model = obj->mdlId[4] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 7), 0x10000000, 2));
    motion = obj->mtnId[1] = fn_80030458(model,
        fn_80008DD0(DATANUM(0x740000, 8), 0x10000000, 2));
    fn_8002C95C(model, motion);
    fn_8002D86C(model, 1.0f);
    fn_800269E8(model, ((1 << 30) | (1 << 0)));
    fn_80026760(model, 0.0f, 500.0f, 0.0f);
    fn_800268B0(model, 1.0f, 1.0f, 1.0f);
    fn_80026B8C(model, 1);
    fn_80026B64(model, 3);
}

/* Creates the result-scene model used when the sequence changes camera views. */
void fn_1_19B4(OMOBJ *obj)
{
    HU3D_MODELID model = 0;
    HU3D_MOTIONID motion = 0;

    model = obj->mdlId[5] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 20), 0x10000000, 2));
    motion = obj->mtnId[2] = fn_80030458(model,
        fn_80008DD0(DATANUM(0x740000, 21), 0x10000000, 2));
    fn_8002C95C(model, motion);
    fn_8002D86C(model, 1.0f);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026808(model, 0.0f, 0.0f, 0.0f);
    fn_80026B8C(model, 1);
    fn_80026B64(model, 3);
}

/* Starts the spaceship door motion by restoring its playback speed to 1.0. */
void fn_1_1AD4(void)
{
    HU3D_MODELID model = lbl_1_bss_28->mdlId[2];
    fn_8002D86C(model, 1.0f);
}

/* Reports whether the spaceship door motion has ended. */
s32 fn_1_1B20(void)
{
    OMOBJ *obj = lbl_1_bss_28;
    M657ArenaWork *arenaWork = obj->data;
    HU3D_MODELID model = obj->mdlId[2];

    if (fn_8002D75C(model)) {
        return TRUE;
    }
    return FALSE;
}

/* Object-create callback that builds the arena models and installs its update. */
void fn_1_1B84(OMOBJ *obj)
{
    fn_1_165C(obj);
    fn_1_1720();
    fn_1_1884(obj);
    fn_1_19B4(obj);
    obj->objFunc = fn_1_1F6C;
}

/* Waits for the main game sequence and starts the arena entrance sound cues. */
void fn_1_1F6C(OMOBJ *obj)
{
    M657ArenaWork *arenaWork = obj->data;

    switch (arenaWork->state) {
        case 0:
            arenaWork->state++;
            break;
        case 1:
            arenaWork->state++;
            break;
        case 2:
            if (fn_80071A8C() == 3) {
                arenaWork->state++;
                HuAudFXPlayPan(2087, 48);
                HuAudFXPlayPan(2088, 80);
            }
            break;
        case 3:
            break;
    }
}
