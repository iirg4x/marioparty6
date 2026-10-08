// Updates HSF mesh vertices from shape animation during the Hu3D model update.
#define _MATH_H
#include "game/ShapeExec.h"
#include "game/EnvelopeExec.h"

// Called by ShapeProc after it selects the mesh vertex buffer.
// Applies sequential weights for shape type 2; other types truncate baseMorph to select a shape and
// use its fraction to interpolate toward the next. The last shape wraps to itself.
static void SetShapeMain(HSF_OBJECT *obj) {
    HSF_BUFFER *nextShape;
    HSF_BUFFER *shape;
    float totalMorphWeight;
    float blendFactor;
    s32 baseShapeIndex;
    s32 vertexIndex;
    s32 nextShapeIndex;
    s32 shapeOrVertexIndex;

    if (obj->mesh.shapeType == 2) {
        totalMorphWeight = 0.0f;
        for (shapeOrVertexIndex = 0; shapeOrVertexIndex < obj->mesh.shapeNum;
             shapeOrVertexIndex++) {
            totalMorphWeight += obj->mesh.mesh.morphWeight[shapeOrVertexIndex];
        }
        shape = *obj->mesh.shape;
        for (shapeOrVertexIndex = 0; shapeOrVertexIndex < shape->count;
             shapeOrVertexIndex++) {
            Vertextop[shapeOrVertexIndex].x = ((Vec*) shape->data)[shapeOrVertexIndex].x;
            Vertextop[shapeOrVertexIndex].y = ((Vec*) shape->data)[shapeOrVertexIndex].y;
            Vertextop[shapeOrVertexIndex].z = ((Vec*) shape->data)[shapeOrVertexIndex].z;
        }
        // Negative weights are zeroed for blending but remain in the sum; if the sum exceeds 1,
        // each weight is divided by it before sequential blending.
        for (shapeOrVertexIndex = 0; shapeOrVertexIndex < obj->mesh.shapeNum;
             shapeOrVertexIndex++) {
            shape = obj->mesh.shape[shapeOrVertexIndex];
            blendFactor = obj->mesh.mesh.morphWeight[shapeOrVertexIndex];
            if (blendFactor < 0.0f) {
                blendFactor = 0.0f;
            } else if (totalMorphWeight > 1.0f) {
                blendFactor /= totalMorphWeight;
            }
            for (vertexIndex = 0; vertexIndex < shape->count; vertexIndex++) {
                Vertextop[vertexIndex].x +=
                    blendFactor * (((Vec *) shape->data)[vertexIndex].x - Vertextop[vertexIndex].x);
                Vertextop[vertexIndex].y +=
                    blendFactor * (((Vec *) shape->data)[vertexIndex].y - Vertextop[vertexIndex].y);
                Vertextop[vertexIndex].z +=
                    blendFactor * (((Vec *) shape->data)[vertexIndex].z - Vertextop[vertexIndex].z);
            }
        }
    } else {
        baseShapeIndex = obj->mesh.mesh.baseMorph;
        nextShapeIndex = baseShapeIndex + 1;
        if (nextShapeIndex >= obj->mesh.shapeNum) {
            nextShapeIndex = baseShapeIndex;
        }
        blendFactor = obj->mesh.mesh.baseMorph - baseShapeIndex;
        shape = obj->mesh.shape[baseShapeIndex];
        nextShape = obj->mesh.shape[nextShapeIndex];
        for (shapeOrVertexIndex = 0; shapeOrVertexIndex < shape->count;
             shapeOrVertexIndex++) {
            Vertextop[shapeOrVertexIndex].x =
                ((Vec *) shape->data)[shapeOrVertexIndex].x +
                blendFactor * (((Vec *) nextShape->data)[shapeOrVertexIndex].x -
                               ((Vec *) shape->data)[shapeOrVertexIndex].x);
            Vertextop[shapeOrVertexIndex].y =
                ((Vec *) shape->data)[shapeOrVertexIndex].y +
                blendFactor * (((Vec *) nextShape->data)[shapeOrVertexIndex].y -
                               ((Vec *) shape->data)[shapeOrVertexIndex].y);
            Vertextop[shapeOrVertexIndex].z =
                ((Vec *) shape->data)[shapeOrVertexIndex].z +
                blendFactor * (((Vec *) nextShape->data)[shapeOrVertexIndex].z -
                               ((Vec *) shape->data)[shapeOrVertexIndex].z);
        }
    }
}

// Hu3D calls this after shape motion and before cluster and envelope processing.
// It updates each mesh with shape data and writes the vertex range back to memory before GX reads
// it.
void ShapeProc(HSF_DATA *hsf) {
    HSF_OBJECT *obj;
    s32 objectIndex;

    obj = hsf->object;
    for (objectIndex = 0; objectIndex < hsf->objectNum; objectIndex++, obj++) {
        if (obj->type == HSF_OBJ_MESH && obj->mesh.shapeNum != 0) {
            Vertextop = obj->mesh.vertex->data;
            SetShapeMain(obj);
            // For each mesh with shape data, it updates and writes back the vertex range with
            // DCStoreRange.
            DCStoreRange(Vertextop, obj->mesh.vertex->count * sizeof(Vec));
            // Mark the mesh as having updated vertices so envelope processing uses this buffer as
            // its source.
            obj->mesh.writeNum++;
        }
    }
}
