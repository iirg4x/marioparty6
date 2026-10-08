/* Evaluates HSF cluster animation and applies its vertex deformation to models. */
#define _MATH_H
#include "game/ClusterExec.h"
#include "game/EnvelopeExec.h"
#include "game/hu3d.h"
#include "game/sprite.h"

/* Samples the HSF cluster-index curve at the supplied motion time. */
float GetClusterCurve(HSF_TRACK *clusterTrack, float motionTime) {
    float *constantValue;

    switch (clusterTrack->curveType) {
        case HSF_CURVE_LINEAR:
            return GetLinear(clusterTrack->numKeyframes, clusterTrack->data, motionTime);
        case HSF_CURVE_BEZIER:
            return GetBezier(clusterTrack->numKeyframes, clusterTrack, motionTime);
        case HSF_CURVE_CONST:
            constantValue = &clusterTrack->value;
            return *constantValue;
    }
    return 0.0f;
}

/* Samples a cluster-weight curve as motion tracks are evaluated. */
float GetClusterWeightCurve(HSF_TRACK *clusterTrack, float motionTime) {
    float *constantValue;

    switch (clusterTrack->curveType) {
        case HSF_CURVE_LINEAR:
            return GetLinear(clusterTrack->numKeyframes, clusterTrack->data, motionTime);
        case HSF_CURVE_BEZIER:
            return GetBezier(clusterTrack->numKeyframes, clusterTrack, motionTime);
        case HSF_CURVE_CONST:
            constantValue = &clusterTrack->value;
            return *constantValue;
    }
    return 0.0f;
}

/* Applies a cluster's weighted or interpolated vertices during ClusterProc. */
void SetClusterMain(HSF_CLUSTER *cluster) {
    float totalWeight;
    float blendAmount;
    s32 vertexFrameIndex;
    s32 modelVertexIndex;
    u16 *partVertexIndex;
    s32 nextVertexFrameIndex;
    s32 vertexIndex;
    s32 clusterVertexIndex;
    HSF_BUFFER *nextVertexBuffer;
    HSF_PART *clusterPart;
    HSF_BUFFER *vertexBuffer;

    clusterPart = cluster->part;
    if (cluster->vertexNum != 0) {
        if (cluster->type == 2) {
            partVertexIndex = clusterPart->vertex;
            vertexBuffer = *cluster->vertex;
            totalWeight = 0.0f;
            for (vertexIndex = 0; vertexIndex < cluster->vertexNum; vertexIndex++) {
                totalWeight += cluster->weight[vertexIndex];
            }
            for (vertexIndex = 0; vertexIndex < clusterPart->num;
                 vertexIndex++, partVertexIndex++) {
                modelVertexIndex = *partVertexIndex;
                Vertextop[modelVertexIndex].x = ((Vec*) vertexBuffer->data)[vertexIndex].x;
                Vertextop[modelVertexIndex].y = ((Vec*) vertexBuffer->data)[vertexIndex].y;
                Vertextop[modelVertexIndex].z = ((Vec*) vertexBuffer->data)[vertexIndex].z;
            }
            /* Vertex 0 is the starting shape; later weights are clamped at zero and, when total
             * authored weight exceeds one, divided by that total. */
            for (vertexIndex = 1; vertexIndex < cluster->vertexNum; vertexIndex++) {
                vertexBuffer = cluster->vertex[vertexIndex];
                partVertexIndex = clusterPart->vertex;
                blendAmount = cluster->weight[vertexIndex];
                if (blendAmount < 0.0f) {
                    blendAmount = 0.0f;
                } else if (totalWeight > 1.0f) {
                    blendAmount /= totalWeight;
                }
                for (clusterVertexIndex = 0; clusterVertexIndex < clusterPart->num;
                     clusterVertexIndex++, partVertexIndex++) {
                    modelVertexIndex = *partVertexIndex;
                    Vertextop[modelVertexIndex].x +=
                        blendAmount * (((Vec *) vertexBuffer->data)[clusterVertexIndex].x -
                                       Vertextop[modelVertexIndex].x);
                    Vertextop[modelVertexIndex].y +=
                        blendAmount * (((Vec *) vertexBuffer->data)[clusterVertexIndex].y -
                                       Vertextop[modelVertexIndex].y);
                    Vertextop[modelVertexIndex].z +=
                        blendAmount * (((Vec *) vertexBuffer->data)[clusterVertexIndex].z -
                                       Vertextop[modelVertexIndex].z);
                }
            }
            return;
        }
        vertexFrameIndex = cluster->index;
        nextVertexFrameIndex = vertexFrameIndex + 1;
        if (nextVertexFrameIndex >= cluster->vertexNum) {
            /* Hold the final cluster shape after its last frame. */
            nextVertexFrameIndex = vertexFrameIndex;
        }
        blendAmount = cluster->index - vertexFrameIndex;
        vertexBuffer = cluster->vertex[vertexFrameIndex];
        nextVertexBuffer = cluster->vertex[nextVertexFrameIndex];
        partVertexIndex = clusterPart->vertex;
        for (vertexIndex = 0; vertexIndex < clusterPart->num; vertexIndex++, partVertexIndex++) {
            modelVertexIndex = *partVertexIndex;
            Vertextop[modelVertexIndex].x =
                ((Vec *) vertexBuffer->data)[vertexIndex].x +
                blendAmount * (((Vec *) nextVertexBuffer->data)[vertexIndex].x -
                               ((Vec *) vertexBuffer->data)[vertexIndex].x);
            Vertextop[modelVertexIndex].y =
                ((Vec *) vertexBuffer->data)[vertexIndex].y +
                blendAmount * (((Vec *) nextVertexBuffer->data)[vertexIndex].y -
                               ((Vec *) vertexBuffer->data)[vertexIndex].y);
            Vertextop[modelVertexIndex].z =
                ((Vec *) vertexBuffer->data)[vertexIndex].z +
                blendAmount * (((Vec *) nextVertexBuffer->data)[vertexIndex].z -
                               ((Vec *) vertexBuffer->data)[vertexIndex].z);
        }
    }
}

/* Applies active cluster deformation after motion and shape evaluation for a model. */
void ClusterProc(HU3D_MODEL *modelP) {
    int clusterMotionId;
    s32 clusterSlotIndex;
    s32 clusterIndex;
    s32 vertexIndex;
    HSF_DATA *hsfMotion;
    HSF_DATA *hsf;
    HU3D_MOTION *motionP;
    HSF_CLUSTER *cluster;
    HSF_OBJECT *obj;

    for (clusterSlotIndex = 0; clusterSlotIndex < HU3D_CLUSTER_MAX; clusterSlotIndex++) {
        clusterMotionId = modelP->motIdCluster[clusterSlotIndex];
        if (clusterMotionId != HU3D_MOTIONID_NONE) {
            motionP = &Hu3DMotion[clusterMotionId];
            hsfMotion = motionP->hsf;
            hsf = modelP->hsf;
            cluster = hsfMotion->cluster;
            for (clusterIndex = 0; clusterIndex < hsfMotion->clusterNum;
                 clusterIndex++, cluster++) {
                if (cluster->target != -1) {
                    obj = hsf->object;
                    obj += cluster->target;
                    Vertextop = obj->mesh.vertex->data;
                    if (obj->mesh.cenvNum) {
                        /* For enveloped meshes, restore the source top vertices before applying
                         * cluster deformation. */
                        for (vertexIndex = 0; vertexIndex < obj->mesh.vertex->count;
                             vertexIndex++) {
                            Vertextop[vertexIndex].x = ((Vec*) obj->mesh.vtxtop)[vertexIndex].x;
                            Vertextop[vertexIndex].y = ((Vec*) obj->mesh.vtxtop)[vertexIndex].y;
                            Vertextop[vertexIndex].z = ((Vec*) obj->mesh.vtxtop)[vertexIndex].z;
                        }
                    }
                    SetClusterMain(cluster);
                    /* Make the CPU-deformed vertex range visible to the graphics processor. */
                    DCStoreRangeNoSync(Vertextop, obj->mesh.vertex->count * sizeof(Vec));
                    /* Mark the mesh as already deformed so the envelope pass uses these
                     * vertices. */
                    obj->mesh.writeNum++;
                }
            }
        }
    }
}

/* Updates active cluster indices and weights during the model's motion evaluation. */
void ClusterMotionExec(HU3D_MODEL *modelP) {
    float motionTime;
    s32 clusterSlotIndex;
    s32 trackIndex;
    HU3D_MOTIONID clusterMotionId;
    HSF_CLUSTER *cluster;
    HSF_DATA *hsf;
    HSF_MOTION *hsfMotion;
    HSF_TRACK *track;
    HSF_TRACK *weightTrack;
    HU3D_MOTION *motionP;
    hsf = modelP->hsf;
    hsfMotion = hsf->motion;
    track = hsfMotion->track;
    for (clusterSlotIndex = 0; clusterSlotIndex < HU3D_CLUSTER_MAX; clusterSlotIndex++) {
        if (modelP->motIdCluster[clusterSlotIndex] != HU3D_MOTIONID_NONE) {
            clusterMotionId = modelP->motIdCluster[clusterSlotIndex];
            motionP = &Hu3DMotion[clusterMotionId];
            hsf = motionP->hsf;
            hsfMotion = hsf->motion;
            track = hsfMotion->track;
            motionTime = modelP->clusterTime[clusterSlotIndex];
            for (trackIndex = 0; trackIndex < hsfMotion->numTracks; trackIndex++, track++) {
                switch (track->type) {
                    case HSF_TRACK_CLUSTER:
                        cluster = &hsf->cluster[track->cluster];
                        cluster->index = GetClusterCurve(track, motionTime);
                        break;
                    case HSF_TRACK_CLUSTER_WEIGHT:
                        weightTrack = track;
                        cluster = &hsf->cluster[weightTrack->cluster];
                        cluster->weight[weightTrack->clusterWeight] =
                            GetClusterCurve(weightTrack, motionTime);
                        break;
                }
            }
        }
    }
}
