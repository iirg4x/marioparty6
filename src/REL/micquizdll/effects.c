/* Speak Up: animated model pieces emitted around the players. */
#include <math.h>
#include "REL/micquizdll/module_types.h"
#include "REL/micquizdll/model_work.h"
#include "humath.h"

/* Red, yellow, blue, and green variants for emitted pieces. */
GXColor lbl_1_data_E68[4] = {
    {255, 64, 64, 255},
    {255, 255, 0, 255},
    {0, 128, 255, 255},
    {64, 255, 64, 255},
};

MicquizModelRecord lbl_1_bss_5E4[240];
MicquizModelWork lbl_1_bss_5D8;

/* Draw the active falling pieces when the effect hook is rendered. */
void fn_1_31E74(HU3D_MODEL *hookModel, Mtx *parentMatrix) {
    Mtx matrix;
    HU3D_MODEL *model;
    s32 recordIndex;
    HSF_DATA *modelData;
    HSF_OBJECT *object;
    MicquizModelRecord *record;
    HSF_MATERIAL *material;
    s32 materialIndex;

    fn_8002472C();
    model = &lbl_802C01C8[lbl_1_bss_5D8.bodyModel];
    modelData = model->hsf;
    object = fn_80026BF4(lbl_1_bss_5D8.bodyModel, "m569_ef2");
    /* Each record supplies another transform for the shared piece geometry. */
    for (recordIndex = 0; recordIndex < 240; recordIndex += 1) {
        record = &lbl_1_bss_5E4[recordIndex];
        if (record->active != 0) {
            fn_80022E38(matrix, record->rotation[0], record->rotation[1], record->rotation[2]);
            mtxTransCat(matrix, record->position[0], record->position[1], record->position[2]);
            mtxScaleCat(matrix, 1.5f, 1.5f, 1.5f);
            PSMTXConcat(*parentMatrix, matrix, matrix);
            fn_80024850(lbl_1_bss_5D8.bodyModel, object, matrix);
            /* Material colors are updated after this piece has been submitted for drawing. */
            materialIndex = 0;
            material = modelData->material;
            while (materialIndex < modelData->materialNum) {
                material->color[0] = record->color.r;
                material->color[1] = record->color.g;
                material->color[2] = record->color.b;
                materialIndex++;
                material++;
            }
        }
    }
}

/* Move the falling pieces each scene update and remove them after four settled updates. */
void fn_1_31FCC(void) {
    s32 recordIndex;
    MicquizModelRecord *record;

    if (lbl_1_bss_5D8.activeRecordCount != 0) {
        recordIndex = 0;
        while (recordIndex < 240) {
            record = &lbl_1_bss_5E4[recordIndex];
            if (record->active != 0) {
                if (record->settled != 0) {
                    record->settledFrames++;
                    if (record->settledFrames > 3) {
                        record->settled = record->active = 0;
                        lbl_1_bss_5D8.activeRecordCount--;
                    }
                } else {
                    PSVECAdd((Point3d *)record->position, (Point3d *)record->velocity,
                             (Point3d *)record->position);
                    PSVECAdd((Point3d *)record->rotation, (Point3d *)record->angularVelocity,
                             (Point3d *)record->rotation);
                    /* Pieces lie flat briefly after crossing the floor. */
                    if (record->position[1] <= lbl_1_bss_5D8.floorHeight) {
                        record->position[1] = lbl_1_bss_5D8.floorHeight;
                        record->rotation[0] = -90.0f;
                        record->rotation[2] = 0.0f;
                        record->settled = 1;
                    }
                }
            }
            recordIndex++;
        }
    }
}

/* Create the falling-piece models and clear their records during scene setup. */
void fn_1_320F8(void) {
    s32 recordIndex;

    lbl_1_bss_5D8.bodyModel = fn_80025A04(
        fn_80008DD0(DATANUM(0xa20000, 99), 0x10000000, 2));
    fn_80026B64(lbl_1_bss_5D8.bodyModel, (1 << 2));
    fn_800269E8(lbl_1_bss_5D8.bodyModel, (1 << 0));
    lbl_1_bss_5D8.hookModel = fn_800260F4(fn_1_31E74);
    fn_80026B64(lbl_1_bss_5D8.hookModel, (1 << 2));
    fn_80026B8C(lbl_1_bss_5D8.hookModel, 1);
    lbl_1_bss_5D8.floorHeight = 0.0f;
    lbl_1_bss_5D8.activeRecordCount = 0;
    recordIndex = 0;
    while (recordIndex < 240) {
        lbl_1_bss_5E4[recordIndex].active = 0;
        lbl_1_bss_5E4[recordIndex].settled = 0;
        recordIndex += 1;
    }
}

/* Release the piece model and its drawing hook when the scene is closed. */
void fn_1_32214(void) {
    fn_800262D0(lbl_1_bss_5D8.bodyModel);
    fn_800262D0(lbl_1_bss_5D8.hookModel);
}

/* Emit up to pieceCount falling pieces around a player during the results reaction. */
void fn_1_32250(const Vec *position, s32 pieceCount) {
    s32 createdCount;
    s32 recordIndex;
    MicquizModelRecord *record;
    f32 elevationAngle;
    f32 radius;
    f32 azimuthAngle;

    createdCount = 0;
    recordIndex = 0;
    while (recordIndex < 240) {
        if (createdCount == pieceCount) {
            break;
        }
        record = &lbl_1_bss_5E4[recordIndex];
        if (record->active == 0) {
            record->active = 1;
            createdCount++;
            lbl_1_bss_5D8.activeRecordCount++;

            radius = (f32)frandmod(10000) / 100.0f;
            elevationAngle = (f32)frandmod(360);
            azimuthAngle = (f32)frandmod(360);
            record->position[0] =
                (f32) ((f64) position->x +
                       ((f64) radius * (sin((M_PI * (f64) azimuthAngle) / 180.0) *
                                        cos((M_PI * (f64) elevationAngle) / 180.0))));
            record->position[1] = (f32)((f64)position->y +
                ((f64)radius * -sin((M_PI * (f64)elevationAngle) / 180.0)));
            record->position[2] = (f32)((f64)position->z +
                ((f64)radius * (cos((M_PI * (f64)azimuthAngle) / 180.0) *
                cos((M_PI * (f64)elevationAngle) / 180.0))));

            record->velocity[0] = (f32)(0.9f *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            record->velocity[1] = -6.5f * ((f32)frandmod(100) / 100.0f);
            if (record->velocity[1] > -2.6000001f) {
                record->velocity[1] = -2.6000001f;
            }
            record->velocity[2] = (f32)(0.5 *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));

            record->rotation[0] = record->rotation[1] =
                record->rotation[2] = 0.0f;
            record->angularVelocity[0] = (f32)(14.0 *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            record->angularVelocity[1] = (f32)(14.0 *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            record->angularVelocity[2] = (f32)(14.0 *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            record->color = lbl_1_data_E68[frandmod(4)];
            record->settledFrames = 0;
        }
        recordIndex++;
    }
}
