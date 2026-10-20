/* Insectiride: model drawing and motion for the celebration particles. */
#include "REL/m647dll/module_types.h"

GXColor lbl_1_data_5A8[4] = {
    {255, 64, 64, 255}, {255, 255, 0, 255},
    {0, 128, 255, 255}, {64, 255, 64, 255}
};

/* Draws active particles from the model hook, updating shared material colors after each draw. */
void fn_1_AFB0(HU3D_MODEL *hookModel, Mtx *parentMatrix) {
    Mtx matrix;
    HU3D_MODEL *model;
    s32 recordIndex;
    HSF_DATA *modelData;
    HSF_OBJECT *object;
    M647ParticleRecord *record;
    HSF_MATERIAL *material;
    s32 materialIndex;

    Hu3DModelObjDrawInit();
    model = &Hu3DData[lbl_1_bss_358.model0];
    modelData = model->hsf;
    object = Hu3DModelObjPtrGet(lbl_1_bss_358.model0, "m569_ef2");
    for (recordIndex = 0; recordIndex < 50; recordIndex += 1) {
        record = &lbl_1_bss_364[recordIndex];
        if (record->active != 0) {
            mtxRot(matrix, record->rotation.x, record->rotation.y, record->rotation.z);
            mtxTransCat(matrix, record->position.x, record->position.y, record->position.z);
            mtxScaleCat(matrix, 1.5f, 1.5f, 1.5f);
            PSMTXConcat(*parentMatrix, matrix, matrix);
            Hu3DModelObjPtrDraw(lbl_1_bss_358.model0, object, matrix);
            /* These colors affect subsequent draws; the current particle has already been drawn. */
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

/* Object update: moves active particles and removes them after four landed updates. */
void fn_1_B108(void) {
    s32 index;
    M647ParticleRecord *record;

    if (lbl_1_bss_358.particleCount != 0) {
        index = 0;
        while (index < 50) {
            record = &lbl_1_bss_364[index];
            if (record->active != 0) {
                if (record->landed != 0) {
                    (record->landedFrames)++;
                    if (record->landedFrames > 3) {
                        record->landed = record->active = 0;
                        (lbl_1_bss_358.particleCount)--;
                    }
                } else {
                    PSVECAdd(&record->position, &record->velocity,
                             &record->position);
                    PSVECAdd(&record->rotation, &record->angularVelocity,
                             &record->rotation);
                    if (record->position.y <= lbl_1_bss_358.floorHeight) {
                        record->position.y = lbl_1_bss_358.floorHeight;
                        record->rotation.x = -90.0f;
                        record->rotation.z = 0.0f;
                        record->landed = 1;
                    }
                }
            }
            index++;
        }
    }
}

/* Creates the particle model and draw hook during minigame setup, then marks each slot inactive
 * and unlanded. */
void fn_1_B234(void) {
    s32 index;

    lbl_1_bss_358.model0 =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m647, 15), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_358.model0, 2U);
    Hu3DModelAttrSet(lbl_1_bss_358.model0, HU3D_ATTR_DISPOFF);
    lbl_1_bss_358.model1 = Hu3DHookFuncCreate(fn_1_AFB0);
    Hu3DModelCameraSet(lbl_1_bss_358.model1, 2U);
    Hu3DModelLayerSet(lbl_1_bss_358.model1, 1);
    lbl_1_bss_358.floorHeight = (f32) 200.0f;
    lbl_1_bss_358.particleCount = 0;
    index = 0;
    while (index < 50) {
        lbl_1_bss_364[index].active = 0;
        lbl_1_bss_364[index].landed = 0;
        index += 1;
    }
}

/* Releases both models used by the celebration effect. */
void fn_1_B350(void) {
    Hu3DModelKill(lbl_1_bss_358.model0);
    Hu3DModelKill(lbl_1_bss_358.model1);
}

/* Spawns up to count particles for the winner sequence, stopping when no slots remain. */
void fn_1_B38C(const Point3d *origin, s32 count) {
    s32 spawned;
    s32 index;
    f32 pitchAngle;
    f32 radius;
    f32 yawAngle;
    M647ParticleRecord *particle;

    spawned = 0;
    index = 0;
    while (index < 50) {
        if (spawned == count) {
            break;
        }
        particle = &lbl_1_bss_364[index];
        if (particle->active == 0) {
            particle->active = 1;
            spawned++;
            lbl_1_bss_358.particleCount++;
            radius = (f32)frandmod(15000) / 100.0f;
            pitchAngle = (f32)frandmod(360);
            yawAngle = (f32)frandmod(360);
            particle->position.x = (f32)((f64)origin->x +
                ((f64)radius * (sin((3.141592653589793 * (f64)yawAngle) / 180.0) *
                                cos((3.141592653589793 * (f64)pitchAngle) / 180.0))));
            particle->position.y = (f32)((f64)origin->y +
                ((f64)radius * -sin((3.141592653589793 * (f64)pitchAngle) / 180.0)));
            particle->position.z = (f32)((f64)origin->z +
                ((f64)radius * (cos((3.141592653589793 * (f64)yawAngle) / 180.0) *
                                cos((3.141592653589793 * (f64)pitchAngle) / 180.0))));
            particle->velocity.x = (f32)((f64)0.8f *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            particle->velocity.y = -6.5f * ((f32)frandmod(100) / 100.0f);
            /* Keep every particle falling by at least this amount each update. */
            if (particle->velocity.y > -2.6000001f) {
                particle->velocity.y = -2.6000001f;
            }
            particle->velocity.z = (f32)(0.5 *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            particle->rotation.x = particle->rotation.y = particle->rotation.z = 0.0f;
            particle->angularVelocity.x = (f32)(14.0 *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            particle->angularVelocity.y = (f32)(14.0 *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            particle->angularVelocity.z = (f32)(14.0 *
                (((f64)(f32)frandmod(201) - 100.0) / 100.0));
            particle->color = lbl_1_data_5A8[frandmod(4)];
            particle->landedFrames = 0;
        }
        index++;
    }
}

M647ParticleRecord lbl_1_bss_364[50];
M647ModelPair lbl_1_bss_358;
