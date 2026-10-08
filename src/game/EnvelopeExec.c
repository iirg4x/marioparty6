/* Builds and updates HSF envelope skinning matrices, vertices, and normals. */
#define _MATH_H
#include "game/EnvelopeExec.h"
#include "game/hsfex.h"

#include "string.h"

static void SetEnvelopMtx(HSF_OBJECT *objectArrayBase, HSF_OBJECT *object, Mtx *parentMtx);
static void SetEnvelopMain(HSF_DATA *model);
static void SetEnvelop(HSF_CENV *envelope);
static void SetMtx(HSF_OBJECT *object, Mtx parentMtx);
static void SetRevMtx(void);
static HSF_SKELETON *SearchSklenton(char *objectName);

Vec *Vertextop;
Mtx *MtxTop;
static u32 nObj;
static u32 nMesh;
static HSF_OBJECT *objtop;
static HSF_DATA *CurHsf;
static Vec *vtxenv;
static Vec *normenv;
static Vec *normtop;
static s32 Meshcnt;
static s32 Meshno;

/* Called after HSF loading to connect named skeleton transforms and initialize envelope
 * matrices. */
void InitEnvelope(HSF_DATA *model) {
    HSF_BUFFER *vertexBuffer;
    HSF_BUFFER *normalBuffer;
    HSF_MATRIX *matrixTable;
    HSF_OBJECT *object;
    HSF_SKELETON *skeletonEntry;
    Mtx identityMtx;
    s32 objectIndex;
    s32 skeletonIndex;

    if (model->cenvNum != 0) {
        object = model->object;
        for (Meshcnt = objectIndex = 0; objectIndex < model->objectNum; objectIndex++, object++) {
            if (object->type == HSF_OBJ_MESH) {
                if (object->mesh.vtxtop) {
                    /* These buffer pointers are captured but not read later in this function. */
                    vertexBuffer = object->mesh.vertex;
                    normalBuffer = object->mesh.normal;
                    Meshcnt++;
                } else {
                    continue;
                }
            }
            skeletonEntry = model->skeleton;
            for (skeletonIndex = 0; skeletonIndex < model->skeletonNum;
                 skeletonIndex++, skeletonEntry++) {
                if (strcmp(object->name, skeletonEntry->name) == 0) {
                    object->mesh.base = skeletonEntry->transform;
                }
            }
            /* Each object reaching this point copies its base transform into the current
             * transform. */
            object->mesh.curr = object->mesh.base;
        }
        CurHsf = model;
        objtop = model->object;
        matrixTable = CurHsf->matrix;
        if (matrixTable) {
            MtxTop = matrixTable->data;
            nObj = matrixTable->count;
            nMesh = matrixTable->base_idx;
        }
        PSMTXIdentity(identityMtx);
        SetMtx(model->root, identityMtx);
        SetRevMtx();
    }
}

/* Recursively builds object matrices during EnvelopeProc, using an object hook when present. */
static void SetEnvelopMtx(HSF_OBJECT *objectArrayBase, HSF_OBJECT *object, Mtx *parentMtx) {
    Mtx rotationMtx;
    Mtx objectMtx;
    Mtx translationMtx;
    HSF_CONSTDATA *objectConstData;
    s32 objectMtxIndex;
    s32 childIndex;
    
    objectConstData = object->constData;
    if(!objectConstData || !objectConstData->hook) {
        PSMTXTrans(translationMtx, object->mesh.curr.pos.x, object->mesh.curr.pos.y,
                   object->mesh.curr.pos.z);
        PSMTXConcat(*parentMtx, translationMtx, objectMtx);
        if (object->mesh.curr.rot.z) {
            PSMTXRotRad(rotationMtx, 'z', MTXDegToRad(object->mesh.curr.rot.z));
            PSMTXConcat(objectMtx, rotationMtx, objectMtx);
        }
        if (object->mesh.curr.rot.y) {
            PSMTXRotRad(rotationMtx, 'y', MTXDegToRad(object->mesh.curr.rot.y));
            PSMTXConcat(objectMtx, rotationMtx, objectMtx);
        }
        if (object->mesh.curr.rot.x) {
            PSMTXRotRad(rotationMtx, 'x', MTXDegToRad(object->mesh.curr.rot.x));
            PSMTXConcat(objectMtx, rotationMtx, objectMtx);
        }
        if (object->mesh.curr.scale.x != 1.0f) {
            objectMtx[0][0] *= object->mesh.curr.scale.x;
            objectMtx[1][0] *= object->mesh.curr.scale.x;
            objectMtx[2][0] *= object->mesh.curr.scale.x;
        }
        if (object->mesh.curr.scale.y != 1.0f) {
            objectMtx[0][1] *= object->mesh.curr.scale.y;
            objectMtx[1][1] *= object->mesh.curr.scale.y;
            objectMtx[2][1] *= object->mesh.curr.scale.y;
        }
        if (object->mesh.curr.scale.z != 1.0f) {
            objectMtx[0][2] *= object->mesh.curr.scale.z;
            objectMtx[1][2] *= object->mesh.curr.scale.z;
            objectMtx[2][2] *= object->mesh.curr.scale.z;
        }
    } else {
        HU3D_OBJ_HOOK hook = objectConstData->hook;
        hook(object, &object->mesh.curr, parentMtx, &objectMtx);
    }
    
    objectMtxIndex = object - objectArrayBase;
    PSMTXCopy(objectMtx, MtxTop[nMesh + objectMtxIndex]);
    for (childIndex = 0; childIndex < object->mesh.childNum; childIndex++) {
        SetEnvelopMtx(objectArrayBase, object->mesh.child[childIndex], &objectMtx);
    }
}

/* Called from the model update path to rebuild transforms and deform envelope meshes for this
 * frame. */
void EnvelopeProc(HSF_DATA *model) {
    HSF_MATRIX *matrixTable;
    HSF_OBJECT *rootObject;
    Mtx identityMtx;

    CurHsf = model;
    matrixTable = CurHsf->matrix;
    MtxTop = matrixTable->data;
    nObj = matrixTable->count;
    nMesh = matrixTable->base_idx;
    rootObject = model->root;
    PSMTXIdentity(identityMtx);
    SetEnvelopMtx(model->object, rootObject, &identityMtx);
    SetEnvelopMain(model);
}

/* HSF model update routines call this before per-frame mesh work to clear each mesh write
 * counter. */
void InitVtxParm(HSF_DATA *model) {
    HSF_OBJECT *object;
    s32 objectIndex;

    object = model->object;
    for (objectIndex = 0; objectIndex < model->objectNum; objectIndex++, object++) {
        if (object->type == HSF_OBJ_MESH) {
            object->mesh.writeNum = 0;
        }
    }
}

/* EnvelopeProc calls this each frame to process mesh envelopes and flush output buffers to
 * cache. */
static void SetEnvelopMain(HSF_DATA *model) {
    void *unusedVertexData;
    void *unusedVertexTop;
    void *unusedVertexBufferAgain;
    HSF_BUFFER *normalBuffer;
    HSF_BUFFER *vertexBuffer;
    HSF_OBJECT *object;
    s32 objectIndex;
    s32 envelopeIndex;
    HSF_CENV *envelope;

    object = model->object;
    for (Meshno = objectIndex = 0; objectIndex < model->objectNum; objectIndex++, object++) {
        if (object->type == HSF_OBJ_MESH) {
            PSMTXInverse(MtxTop[&object[nMesh] - model->object], MtxTop[Meshno]);
            vertexBuffer = object->mesh.vertex;
            normalBuffer = object->mesh.normal;
            if (object->mesh.writeNum != 0) {
                Vertextop = vertexBuffer->data;
            } else {
                Vertextop = object->mesh.vtxtop;
            }
            vtxenv = vertexBuffer->data;
            normtop = object->mesh.normtop;
            normenv = normalBuffer->data;
            envelope = object->mesh.cenv;
            for (envelopeIndex = 0; envelopeIndex < object->mesh.cenvNum;
                 envelopeIndex++, envelope++) {
                SetEnvelop(envelope);
            }
            /* These values are assigned to locals that are never read. */
            unusedVertexData = vertexBuffer->data;
            unusedVertexTop = object->mesh.vtxtop;
            unusedVertexBufferAgain = vertexBuffer->data;
            DCStoreRangeNoSync(normenv, normalBuffer->count * sizeof(Vec));
            DCStoreRangeNoSync(vtxenv, vertexBuffer->count * sizeof(Vec));
            Meshno++;
        }
    }
}

/* SetEnvelop uses this to add dual-target transforms; a temporary protects aliased output. */
void MTXAdd(Mtx *leftMtx, Mtx *rightMtx, Mtx *resultMtx)
{
    Mtx scratchMtx;
    Mtx *outputMtx;
    if(resultMtx == leftMtx || resultMtx == rightMtx) {
        outputMtx = &scratchMtx;
    } else {
        outputMtx = resultMtx;
    }
    (*outputMtx)[0][0] = (*leftMtx)[0][0]+(*rightMtx)[0][0];
    (*outputMtx)[0][1] = (*leftMtx)[0][1]+(*rightMtx)[0][1];
    (*outputMtx)[0][2] = (*leftMtx)[0][2]+(*rightMtx)[0][2];
    (*outputMtx)[0][3] = (*leftMtx)[0][3]+(*rightMtx)[0][3];
    (*outputMtx)[1][0] = (*leftMtx)[1][0]+(*rightMtx)[1][0];
    (*outputMtx)[1][1] = (*leftMtx)[1][1]+(*rightMtx)[1][1];
    (*outputMtx)[1][2] = (*leftMtx)[1][2]+(*rightMtx)[1][2];
    (*outputMtx)[1][3] = (*leftMtx)[1][3]+(*rightMtx)[1][3];
    (*outputMtx)[2][0] = (*leftMtx)[2][0]+(*rightMtx)[2][0];
    (*outputMtx)[2][1] = (*leftMtx)[2][1]+(*rightMtx)[2][1];
    (*outputMtx)[2][2] = (*leftMtx)[2][2]+(*rightMtx)[2][2];
    (*outputMtx)[2][3] = (*leftMtx)[2][3]+(*rightMtx)[2][3];
    if(outputMtx == &scratchMtx) {
        MTXCopy(scratchMtx, *resultMtx);
    }
}

/* Applies single-, dual-, and multi-target transforms to envelope vertices and normals, then
 * copies trailing vertices. */
static void SetEnvelop(HSF_CENV *envelope) {
    Vec weightedVertexDelta;
    Vec totalVertexDelta;
    Vec weightedNormalDelta;
    Vec totalNormalDelta;
    Vec matrixScale;
    s32 unusedAccumulator;
    u32 targetSkeletonIndex1;
    u32 targetSkeletonIndex2;
    HSF_CENV_DUAL *dualEnvelope;
    HSF_CENV_DUAL_WEIGHT *dualWeight;
    HSF_CENV_MULTI *multiEnvelope;
    HSF_CENV_MULTI_WEIGHT *multiWeight;
    HSF_CENV_SINGLE *singleEnvelope;
    Vec *currentNormal;
    Vec *sourceNormal;
    Vec *currentVertex;
    Vec *sourceVertex;
    float blendWeight;
    s32 normalIndex;
    s32 vertexIndex;
    s32 envelopeIndex;
    s32 weightIndex;
    Mtx skinMtx;
    Mtx normalMtx;
    Mtx scratchMtx;
    Mtx scratchMtx2;
    Mtx inverseScaleMtx;
    Mtx secondTargetMtx;
    Mtx blendedMtx;

    singleEnvelope = envelope->singleData;
    for (envelopeIndex = 0; envelopeIndex < envelope->singleCount;
         envelopeIndex++, singleEnvelope++) {
        normalIndex = singleEnvelope->normal;
        vertexIndex = singleEnvelope->pos;
        currentVertex = &vtxenv[vertexIndex];
        sourceVertex = &Vertextop[vertexIndex];
        currentNormal = &normenv[normalIndex];
        sourceNormal = &normtop[normalIndex];
        PSMTXConcat(MtxTop[nMesh + singleEnvelope->target],
                    MtxTop[nMesh + nObj + nObj * Meshno + singleEnvelope->target], scratchMtx);
        PSMTXConcat(MtxTop[Meshno], scratchMtx, skinMtx);
        Hu3DMtxScaleGet(skinMtx, &matrixScale);
        if (matrixScale.x != 1.0f || matrixScale.y != 1.0f || matrixScale.z != 1.0f) {
            PSMTXScale(inverseScaleMtx, 1.0 / matrixScale.x, 1.0 / matrixScale.y,
                       1.0 / matrixScale.z);
            PSMTXConcat(inverseScaleMtx, skinMtx, normalMtx);
            PSMTXInvXpose(normalMtx, normalMtx);
        } else {
            PSMTXInvXpose(skinMtx, normalMtx);
        }
        if (singleEnvelope->posNum == 1) {
            PSMTXMultVec(skinMtx, sourceVertex, currentVertex);
            PSMTXMultVec(normalMtx, sourceNormal, currentNormal);
        } else if (singleEnvelope->posNum <= 6) {
            PSMTXMultVecArray(skinMtx, sourceVertex, currentVertex, singleEnvelope->posNum);
            PSMTXMultVecArray(normalMtx, sourceNormal, currentNormal, singleEnvelope->normalNum);
        } else {
            PSMTXReorder(skinMtx, (ROMtxPtr) scratchMtx);
            PSMTXReorder(normalMtx, (ROMtxPtr) scratchMtx2);
            PSMTXROMultVecArray((ROMtxPtr) scratchMtx, sourceVertex, currentVertex,
                                singleEnvelope->posNum);
            PSMTXROMultVecArray((ROMtxPtr) scratchMtx2, sourceNormal, currentNormal,
                                singleEnvelope->normalNum);
        }
    }
    dualEnvelope = envelope->dualData;
    for (envelopeIndex = 0; envelopeIndex < envelope->dualCount; envelopeIndex++, dualEnvelope++) {
        targetSkeletonIndex1 = dualEnvelope->target1;
        targetSkeletonIndex2 = dualEnvelope->target2;
        PSMTXConcat(MtxTop[nMesh + targetSkeletonIndex1],
                    MtxTop[nMesh + nObj + nObj * Meshno + targetSkeletonIndex1], scratchMtx);
        PSMTXConcat(MtxTop[Meshno], scratchMtx, skinMtx);
        PSMTXConcat(MtxTop[nMesh + targetSkeletonIndex2],
                    MtxTop[nMesh + nObj + nObj * Meshno + targetSkeletonIndex2], scratchMtx);
        PSMTXConcat(MtxTop[Meshno], scratchMtx, secondTargetMtx);
        dualWeight = dualEnvelope->weight;
        for (weightIndex = 0; weightIndex < dualEnvelope->weightNum; weightIndex++, dualWeight++) {
            normalIndex = dualWeight->normal;
            vertexIndex = dualWeight->pos;
            currentVertex = &vtxenv[vertexIndex];
            sourceVertex = &Vertextop[vertexIndex];
            currentNormal = &normenv[normalIndex];
            sourceNormal = &normtop[normalIndex];
            blendWeight = dualWeight->weight;
            scratchMtx[0][0] = skinMtx[0][0] * blendWeight;
            scratchMtx[1][0] = skinMtx[1][0] * blendWeight;
            scratchMtx[2][0] = skinMtx[2][0] * blendWeight;
            scratchMtx[0][1] = skinMtx[0][1] * blendWeight;
            scratchMtx[1][1] = skinMtx[1][1] * blendWeight;
            scratchMtx[2][1] = skinMtx[2][1] * blendWeight;
            scratchMtx[0][2] = skinMtx[0][2] * blendWeight;
            scratchMtx[1][2] = skinMtx[1][2] * blendWeight;
            scratchMtx[2][2] = skinMtx[2][2] * blendWeight;
            scratchMtx[0][3] = skinMtx[0][3] * blendWeight;
            scratchMtx[1][3] = skinMtx[1][3] * blendWeight;
            scratchMtx[2][3] = skinMtx[2][3] * blendWeight;
            /* The second target receives the remainder of the first target's weight. */
            blendWeight = 1.0f - dualWeight->weight;
            scratchMtx2[0][0] = secondTargetMtx[0][0] * blendWeight;
            scratchMtx2[1][0] = secondTargetMtx[1][0] * blendWeight;
            scratchMtx2[2][0] = secondTargetMtx[2][0] * blendWeight;
            scratchMtx2[0][1] = secondTargetMtx[0][1] * blendWeight;
            scratchMtx2[1][1] = secondTargetMtx[1][1] * blendWeight;
            scratchMtx2[2][1] = secondTargetMtx[2][1] * blendWeight;
            scratchMtx2[0][2] = secondTargetMtx[0][2] * blendWeight;
            scratchMtx2[1][2] = secondTargetMtx[1][2] * blendWeight;
            scratchMtx2[2][2] = secondTargetMtx[2][2] * blendWeight;
            scratchMtx2[0][3] = secondTargetMtx[0][3] * blendWeight;
            scratchMtx2[1][3] = secondTargetMtx[1][3] * blendWeight;
            scratchMtx2[2][3] = secondTargetMtx[2][3] * blendWeight;
            MTXAdd(&scratchMtx2, &scratchMtx, &blendedMtx);
            Hu3DMtxScaleGet(&blendedMtx[0], &matrixScale);
            if (matrixScale.x != 1.0f || matrixScale.y != 1.0f || matrixScale.z != 1.0f) {
                PSMTXScale(inverseScaleMtx, 1.0 / matrixScale.x, 1.0 / matrixScale.y,
                           1.0 / matrixScale.z);
                PSMTXConcat(inverseScaleMtx, blendedMtx, scratchMtx2);
                PSMTXInvXpose(scratchMtx2, scratchMtx2);
            } else {
                PSMTXInvXpose(blendedMtx, scratchMtx2);
            }
            if (dualWeight->posNum == 1) {
                PSMTXMultVec(blendedMtx, sourceVertex, currentVertex);
            } else if (dualWeight->posNum <= 6) {
                PSMTXMultVecArray(blendedMtx, sourceVertex, currentVertex, dualWeight->posNum);
            } else {
                PSMTXReorder(blendedMtx, (ROMtxPtr) scratchMtx);
                PSMTXROMultVecArray((ROMtxPtr) scratchMtx, sourceVertex, currentVertex,
                                    dualWeight->posNum);
            }
            if (dualWeight->normalNum != 0) {
                if (dualWeight->normalNum == 1) {
                    PSMTXMultVec(scratchMtx2, sourceNormal, currentNormal);
                } else if (dualWeight->normalNum <= 6) {
                    PSMTXMultVecArray(scratchMtx2, sourceNormal, currentNormal,
                                      dualWeight->normalNum);
                } else {
                    PSMTXReorder(scratchMtx2, (ROMtxPtr) scratchMtx);
                    PSMTXROMultVecArray((ROMtxPtr) scratchMtx, sourceNormal, currentNormal,
                                        dualWeight->normalNum);
                }
            }
        }
    }
    multiEnvelope = envelope->multiData;
    for (envelopeIndex = 0; envelopeIndex < envelope->multiCount;
         envelopeIndex++, multiEnvelope++) {
        multiWeight = multiEnvelope->weight;
        normalIndex = multiEnvelope->normal;
        vertexIndex = multiEnvelope->pos;
        currentVertex = &vtxenv[vertexIndex];
        sourceVertex = &Vertextop[vertexIndex];
        currentNormal = &normenv[normalIndex];
        sourceNormal = &normtop[normalIndex];
        totalVertexDelta.x = totalVertexDelta.y = totalVertexDelta.z = 0.0f;
        totalNormalDelta.x = totalNormalDelta.y = totalNormalDelta.z = 0.0f;
        /* Each influence adds its weighted transformed-minus-source delta to the vertex and normal
         * totals; the totals are added to the source values below. */
        unusedAccumulator = 0;
        for (weightIndex = 0; weightIndex < multiEnvelope->weightNum;
             weightIndex++, multiWeight++) {
            PSMTXConcat(MtxTop[nMesh + multiWeight->target],
                        MtxTop[nMesh + nObj + nObj * Meshno + multiWeight->target], skinMtx);
            PSMTXConcat(MtxTop[Meshno], skinMtx, skinMtx);
            PSMTXInvXpose(skinMtx, normalMtx);
            PSMTXMultVec(skinMtx, sourceVertex, &weightedVertexDelta);
            PSMTXMultVec(normalMtx, sourceNormal, &weightedNormalDelta);
            weightedVertexDelta.x = multiWeight->value * (weightedVertexDelta.x - sourceVertex->x);
            weightedVertexDelta.y = multiWeight->value * (weightedVertexDelta.y - sourceVertex->y);
            weightedVertexDelta.z = multiWeight->value * (weightedVertexDelta.z - sourceVertex->z);
            VECAdd(&totalVertexDelta, &weightedVertexDelta, &totalVertexDelta);
            weightedNormalDelta.x = multiWeight->value * (weightedNormalDelta.x - sourceNormal->x);
            weightedNormalDelta.y = multiWeight->value * (weightedNormalDelta.y - sourceNormal->y);
            weightedNormalDelta.z = multiWeight->value * (weightedNormalDelta.z - sourceNormal->z);
            VECAdd(&totalNormalDelta, &weightedNormalDelta, &totalNormalDelta);
        }
        currentVertex->x = sourceVertex->x + totalVertexDelta.x;
        currentVertex->y = sourceVertex->y + totalVertexDelta.y;
        currentVertex->z = sourceVertex->z + totalVertexDelta.z;
        currentNormal->x = sourceNormal->x + totalNormalDelta.x;
        currentNormal->y = sourceNormal->y + totalNormalDelta.y;
        currentNormal->z = sourceNormal->z + totalNormalDelta.z;
    }
    vertexIndex = envelope->vtxCount;
    currentVertex = &vtxenv[vertexIndex];
    sourceVertex = &Vertextop[vertexIndex];
    for (envelopeIndex = 0; envelopeIndex < envelope->copyCount;
         envelopeIndex++, currentVertex++, sourceVertex++) {
        currentVertex->x = sourceVertex->x;
        currentVertex->y = sourceVertex->y;
        currentVertex->z = sourceVertex->z;
    }
}

/* InitEnvelope uses this to build world matrices for the object tree; a skeleton entry with the
 * object's name supplies its base transform. */
static void SetMtx(HSF_OBJECT *object, Mtx parentMtx) {
    HSF_SKELETON *skeletonEntry;
    Mtx worldMtx;
    Mtx scaleMtx;
    Mtx rotationMtx;
    s32 objectIndex;
    s32 childIndex;

    skeletonEntry = SearchSklenton(object->name);
    if (skeletonEntry) {
        object->mesh.base = skeletonEntry->transform;
    }
    PSMTXTrans(worldMtx, object->mesh.base.pos.x, object->mesh.base.pos.y, object->mesh.base.pos.z);
    
    PSMTXConcat(parentMtx, worldMtx, worldMtx);
    if(object->mesh.base.rot.z != 0.0f) {
        MTXRotDeg(rotationMtx, 'z', object->mesh.base.rot.z);
        PSMTXConcat(worldMtx, rotationMtx, worldMtx);
    }
    if(object->mesh.base.rot.y != 0.0f) {
        MTXRotDeg(rotationMtx, 'y', object->mesh.base.rot.y);
        PSMTXConcat(worldMtx, rotationMtx, worldMtx);
    }
    if(object->mesh.base.rot.x != 0.0f) {
        MTXRotDeg(rotationMtx, 'x', object->mesh.base.rot.x);
        PSMTXConcat(worldMtx, rotationMtx, worldMtx);
    }
    if (object->mesh.base.scale.x != 1.0 || object->mesh.base.scale.y != 1.0 ||
        object->mesh.base.scale.z != 1.0) {
        PSMTXScale(scaleMtx, object->mesh.base.scale.x, object->mesh.base.scale.y,
                   object->mesh.base.scale.z);
        PSMTXConcat(worldMtx, scaleMtx, worldMtx);
    }

    objectIndex = object - objtop;
    PSMTXCopy(worldMtx, MtxTop[nMesh + objectIndex]);
    for (childIndex = 0; childIndex < object->mesh.childNum; childIndex++) {
        SetMtx(object->mesh.child[childIndex], worldMtx);
    }
}

/* InitEnvelope calls this to build inverse-relative matrices for every mesh. */
static void SetRevMtx(void) {
    HSF_OBJECT *object;
    s32 meshIndex;
    s32 objectIndex;
    s32 targetIndex;
    Mtx inverseMtx;
    Mtx meshMtx;

    object = CurHsf->object;
    for (meshIndex = objectIndex = 0; objectIndex < CurHsf->objectNum; objectIndex++, object++) {
        if (object->type == HSF_OBJ_MESH) {
            PSMTXCopy(MtxTop[nMesh + objectIndex], meshMtx);
            for (targetIndex = 0; targetIndex < CurHsf->objectNum; targetIndex++) {
                PSMTXInverse(MtxTop[nMesh + targetIndex], inverseMtx);
                PSMTXConcat(inverseMtx, meshMtx,
                            MtxTop[nMesh + nObj + nObj * meshIndex + targetIndex]);
            }
            /* This final inverse is written to a local that is not read afterward. */
            PSMTXInverse(MtxTop[nMesh + objectIndex], meshMtx);
            meshIndex++;
        }
    }
}

/* SetMtx uses this during model setup to find the skeleton entry named for an object, or NULL if
 * no entry matches. */
static HSF_SKELETON *SearchSklenton(char *objectName) {
    HSF_SKELETON *skeletonEntry;
    s32 skeletonIndex;

    skeletonEntry = CurHsf->skeleton;
    for (skeletonIndex = 0; skeletonIndex < CurHsf->skeletonNum; skeletonIndex++, skeletonEntry++) {
        if (strcmp(objectName, skeletonEntry->name) == 0) {
            return skeletonEntry;
        }
    }
    return NULL;
}
