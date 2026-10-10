// Provides collision detection and response for minigame models and bodies.
#define _MATH_H
#include "dolphin/math.h"

extern inline float fabsf(float x)
{
    return fabs(x);
}

#include "game/mg/colman.h"
#include "game/hu3d.h"
#include "game/frand.h"

#define COL_MAT_ATTR_MASK 0x3F0000 // Material bits encoding the surface response.

#define COL_MAT_ATTR_1_BIT_PATTERN 0x010000
#define COL_MAT_ATTR_2_BIT_PATTERN 0x020000
#define COL_MAT_ATTR_3_BIT_PATTERN 0x030000
#define COL_MAT_ATTR_4_BIT_PATTERN 0x040000
#define COL_MAT_ATTR_5_BIT_PATTERN 0x050000
#define COL_MAT_ATTR_6_BIT_PATTERN 0x060000
#define COL_MAT_ATTR_7_BIT_PATTERN 0x070000
#define COL_MAT_ATTR_8_BIT_PATTERN 0x080000
#define COL_MAT_ATTR_9_BIT_PATTERN 0x100000
#define COL_MAT_ATTR_10_BIT_PATTERN 0x180000
#define COL_MAT_ATTR_11_BIT_PATTERN 0x200000
#define COL_MAT_ATTR_12_BIT_PATTERN 0x280000
#define COL_MAT_ATTR_13_BIT_PATTERN 0x300000
#define COL_MAT_ATTR_14_BIT_PATTERN 0x380000

// Skips later mesh tests when a pass leaves the mesh/body contact bits clear.
// Immediate overlap correction can leave those bits clear too.
#define COLBODY_ATTR_NO_CONTACT_FLAG 0x80000000
#define COLBODY_ATTR_COLLISION_MASK 0x0C000000 // Mesh or body contact in the current pass.
#define COLBODY_ATTR_PASS_CONTACT_MASK 0x7C000000 // Contact bits retained across collision passes.
#define COLBODY_ATTR_PASS_RETAIN_MASK 0x83FFFFFF // Attribute bits preserved for the next pass.

#define COLBODY_ATTR_MESH_CONTACT_FLAG 0x04000000 // A triangle mesh has a contact point for this
                                                  // body.
#define COLBODY_ATTR_BODY_CONTACT_FLAG 0x08000000 // A body pair recorded a collision.
#define COLBODY_ATTR_SWEEP_CONTACT_FLAG 0x10000000 // A swept body contact was found.
// An overlap was corrected immediately; retain its contact point after this pass.
#define COLBODY_ATTR_CONTACT_CORRECTED_FLAG 0x20000000
#define COLBODY_ATTR_BODY_PAIR_CONTACT_FLAG 0x40000000 // This body already resolved a pair contact.
#define COLBODY_ATTR_CONTACT_POINT_LIMIT_FLAG 0x02000000 // The collision point list reached its
                                                         // limit.
#define COLBODY_ATTR_MESH_CONTACT_MASK 0x14000000 // Mesh contact state cleared before correction.

typedef struct ColBroad_s {
    float t; // Movement fraction chosen for resolving this body pair; mesh-test tolerance can
             // exceed 1.
    COLBODY *body1; // First body in the candidate pair.
    COLBODY *body2; // Second body in the candidate pair.
    s16 body1Idx; // Index of the first body in the body array.
    s16 body2Idx; // Index of the second body in the body array.
    int colResult; // Contact classification used by the narrow phase.
} COLBROAD;

typedef struct ColNarrow_s {
    float t; // Contact time copied from the candidate pair.
    COLBROAD *broadP; // Candidate pair to resolve in narrow-phase order.
} COLNARROW;

typedef struct ColWork_s {
    HU3D_MODELID *mdlId; // Model IDs whose meshes participate in collision.
    s16 mdlNum; // Number of registered model IDs.
    s16 attr; // Collision-map state and per-frame mode bits.
    COLBROAD *broadCol; // Broad-phase candidate-pair storage.
    int broadColNum; // Number of candidate pairs found in the current pass.
    COL_ATTRPARAM attrParam[7]; // Surface response parameters for material codes 0-6.
    COL_ATTRPARAM attrParamHi[7]; // Surface response parameters for material codes 7-13.
    COLBODY *body; // Collision bodies controlled by actors.
    int bodyNum; // Number of body slots reserved for this map.
    COLNARROW *narrowCol; // Narrow-phase pair storage ordered by contact time.
    s16 *colOrder1; // Left bounds used while sorting collision times.
    s16 *colOrder2; // Right bounds used while sorting collision times.
    u32 reserved; // No collision routine reads or writes this storage.
} COLWORK;

typedef struct ColTri_s {
    HuVecF norm; // Unit normal of the triangle plane.
    HuVecF edgeNorm1; // Inward-facing normal for the first triangle edge.
    HuVecF edgeNorm2; // Inward-facing normal for the second triangle edge.
    HuVecF edgeNorm3; // Inward-facing normal for the third triangle edge.
    float d; // Plane constant in dot(norm, point) + d = 0.
    HuVecF center; // Triangle centroid in mesh-local coordinates.
    float dist; // Squared radius enclosing all three vertices around center.
} COLTRI;

typedef struct ColMesh_s {
    HSF_OBJECT *obj; // HSF mesh object represented by this collision record.
    COLTRI *tri; // Triangle plane and edge data built from the mesh faces.
    HuVecF *bodyPos; // Per-body positions transformed into this mesh's local space.
    HuVecF *bodyMove; // Per-body world-space displacement induced by this mesh's transform change.
    u8 mdlNo; // Index of the owning model in the registered model list.
    u32 mask; // Model collision mask tested against query and body masks.
    HuVecF boundsCenter; // Mesh-local center of the broad-phase bounding sphere.
    float boundsRadius; // Radius of the mesh-local broad-phase bounding sphere.
    Mtx mtx; // Current local-to-world transform.
    Mtx mtxOld; // Previous local-to-world transform.
    Mtx mtxInv; // Current world-to-local transform.
    Mtx mtxInvOld; // Previous world-to-local transform.
} COLMESH;

static COLWORK colWork; // Collision-map registrations, bodies, and response parameters.

static BOOL colMapInitF; // True once model and triangle setup is complete.
static BOOL CancelTRXF; // Selects current object transforms for the model with a motion.
static int colMeshCount; // Number of mesh objects in all registered models.
static COLMESH *colMesh; // Mesh transforms, bounds, triangles, and per-body movement.

#define SWAP(a, b, type) \
do { \
    type temp; \
    temp = a; \
    a = b; \
    b = temp; \
} while(0)

// Sorts narrow-phase collision records by contact time for ordered resolution.
// Called by _BodyColNarrow before resolving body pairs.
static void SortCollisions(COLNARROW *collisions, int collisionCount)
{
    s16 rightIdx;
    s16 *leftBounds;
    s16 leftIdx;
    s16 *rightBounds;
    int partitionCount;
    int partitionIdx;

    float pivotTime;

    leftBounds = colWork.colOrder1;
    rightBounds = colWork.colOrder2;
    leftBounds[0] = 0;
    rightBounds[0] = collisionCount-1;
    for(partitionIdx=0, partitionCount=1; partitionIdx<partitionCount; partitionIdx++) {
        pivotTime = collisions[leftBounds[partitionIdx]].t;
        leftIdx = leftBounds[partitionIdx]+1;
        rightIdx = rightBounds[partitionIdx];
        if(rightBounds[partitionIdx] > leftBounds[partitionIdx]) {
            while(leftIdx < rightIdx) {
                if(collisions[leftIdx].t < pivotTime) {
                    if(pivotTime <= collisions[rightIdx].t) {
                        leftIdx++;
                        rightIdx--;
                    } else {
                        SWAP(collisions[leftIdx+1], collisions[rightIdx], COLNARROW);
                        leftIdx += 2;
                    }
                } else {
                    if(pivotTime <= collisions[rightIdx].t) {
                        SWAP(collisions[rightIdx-1], collisions[leftIdx], COLNARROW);
                        rightIdx -= 2;
                    } else {
                        SWAP(collisions[rightIdx], collisions[leftIdx], COLNARROW);
                        leftIdx++;
                        rightIdx--;
                    }
                }
            }
            if(leftIdx == rightIdx) {
                if(pivotTime <= collisions[leftIdx].t) {
                    rightIdx--;
                } else {
                    leftIdx++;
                }
            }
            if(rightIdx == leftBounds[partitionIdx]) {
                leftBounds[partitionCount] = leftIdx;
                rightBounds[partitionCount] = rightBounds[partitionIdx];
                partitionCount++;
            } else {
                COLNARROW *firstCollision = &collisions[leftBounds[partitionIdx]];
                SWAP(collisions[rightIdx], *firstCollision, COLNARROW);
                leftBounds[partitionCount] = leftBounds[partitionIdx];
                rightBounds[partitionCount] = rightIdx-1;
                partitionCount++;
                leftBounds[partitionCount] = leftIdx;
                rightBounds[partitionCount] = rightBounds[partitionIdx];
                partitionCount++;
            }
        }
    }
}

// Copies a matrix's rotation and scale rows while clearing translation.
// Used by _BodyMeshCol when converting a body radius into mesh-local space.
static inline void RemoveMtxTrans(Mtx outputMatrix, Mtx inputMatrix)
{
    HuVecF firstRow, secondRow, thirdRow;
    firstRow.x = inputMatrix[0][0];
    firstRow.y = inputMatrix[0][1];
    firstRow.z = inputMatrix[0][2];
    secondRow.x = inputMatrix[1][0];
    secondRow.y = inputMatrix[1][1];
    secondRow.z = inputMatrix[1][2];
    thirdRow.x = inputMatrix[2][0];
    thirdRow.y = inputMatrix[2][1];
    thirdRow.z = inputMatrix[2][2];

    outputMatrix[0][0] = firstRow.x;
    outputMatrix[0][1] = firstRow.y;
    outputMatrix[0][2] = firstRow.z;
    outputMatrix[0][3] = 0;
    outputMatrix[1][0] = secondRow.x;
    outputMatrix[1][1] = secondRow.y;
    outputMatrix[1][2] = secondRow.z;
    outputMatrix[1][3] = 0;
    outputMatrix[2][0] = thirdRow.x;
    outputMatrix[2][1] = thirdRow.y;
    outputMatrix[2][2] = thirdRow.z;
    outputMatrix[2][3] = 0;
}

// Replaces a matrix with diagonal scale values measured from each basis row.
// Used by CheckPoint to scale the overlap correction.
static inline void MakeScaleMtx(Mtx outputMatrix, Mtx inputMatrix)
{
    HuVecF firstRow, secondRow, thirdRow;
    firstRow.x = inputMatrix[0][0];
    firstRow.y = inputMatrix[0][1];
    firstRow.z = inputMatrix[0][2];
    secondRow.x = inputMatrix[1][0];
    secondRow.y = inputMatrix[1][1];
    secondRow.z = inputMatrix[1][2];
    thirdRow.x = inputMatrix[2][0];
    thirdRow.y = inputMatrix[2][1];
    thirdRow.z = inputMatrix[2][2];
    outputMatrix[0][0] = VECMag(&firstRow);
    outputMatrix[0][1] = 0;
    outputMatrix[0][2] = 0;
    outputMatrix[0][3] = 0;
    outputMatrix[1][0] = 0;
    outputMatrix[1][1] = VECMag(&secondRow);
    outputMatrix[1][2] = 0;
    outputMatrix[1][3] = 0;
    outputMatrix[2][0] = 0;
    outputMatrix[2][1] = 0;
    outputMatrix[2][2] = VECMag(&thirdRow);
    outputMatrix[2][3] = 0;
}

// Builds a body support offset from a triangle normal and body radius for _BodyMeshCol.
// Y is zero when abs(normal.y) < 0.001, otherwise the signed half-radius.
static inline void MakeMeshEject(COLBODY *body, COLTRI *triangle, HuVecF *eject,
                                 HuVecF *planeNormal)
{
    float halfRadius;
    float radius;

    halfRadius = body->param.radius;
    radius = body->param.radius;
    *planeNormal = triangle->norm;
    eject->x = planeNormal->x;
    eject->y = 0;
    eject->z = planeNormal->z;
    if(fabsf(eject->x) > 0.001f || fabsf(eject->z) > 0.001f) {
        VECNormalize(eject, eject);
    }
    halfRadius /= 2;
    eject->x *= radius;
    if(fabsf(planeNormal->y) < 0.001f) {
        eject->y = 0;
    } else {
        eject->y = planeNormal->y > 0 ? halfRadius : -halfRadius;
    }
    eject->z *= radius;
}

// Returns -1, 0, or 1 according to the sign of a value.
// Used by SameSign when classifying triangle-plane distances.
static inline float GetSign(float value)
{
    if(value < 0) {
        return -1;
    } else if(value > 0) {
        return 1;
    } else {
        return 0;
    }
}

// Unused helper; its discarded constants have no runtime effect.
static void UseFloat(void)
{
    (void)0.0f;
    (void)0.001f;
    (void)0.5f;
    (void)-1.0f;
    (void)1.0f;
}

// Finds the collision mesh record associated with an HSF object.
// Used while ColMtxCalcHsfObject walks a model's object hierarchy.
static COLMESH *SearchHsfObjectColMesh(HSF_OBJECT *object)
{
    int meshIdx;
    for(meshIdx=colMeshCount; meshIdx--;) {
        if(colMesh[meshIdx].obj == object) {
            return &colMesh[meshIdx];
        }
    }
    return NULL;
}

// Adds a translation to the fourth column of a collision transform.
// Used while composing model and object collision transforms.
static void ColMtxTransApply(Mtx matrix, float translateX, float translateY, float translateZ)
{
    matrix[0][3] += translateX;
    matrix[1][3] += translateY;
    matrix[2][3] += translateZ;
}

// Builds an X/Y/Z rotation matrix, concatenating nonzero rotations in order.
// Used while composing model and object collision transforms; angles are degrees.
static void ColMtxRot(Mtx matrix, float rotateX, float rotateY, float rotateZ)
{
    if(rotateX != 0) {
        MTXRotDeg(matrix, 'X', rotateX);
    } else {
        MTXIdentity(matrix);
    }
    if(rotateY != 0) {
        Mtx temp;
        MTXRotDeg(temp, 'Y', rotateY);
        MTXConcat(temp, matrix, matrix);
    }
    if(rotateZ != 0) {
        Mtx temp;
        MTXRotDeg(temp, 'Z', rotateZ);
        MTXConcat(temp, matrix, matrix);
    }
}

// Unused helper; its discarded constants have no runtime effect.
static void UseFloat2(void)
{
    (void)0.0001f;
    (void)2.0f;
    (void)4.0f;
}

// Computes a vertex AABB center and the radius enclosing all supplied vertices.
// Called by ColMapInit for each collision mesh; leaves outputs untouched for no vertices.
static void ColBoundsGet(HuVecF *center, float *boundRadius, HuVecF *vertices, int vertexCount)
{
    HuVecF maxPoint;
    HuVecF minPoint;
    HuVecF fromCenter;
    HuVecF *vertex;
    float maxDistanceSquared;
    float distanceSquared;
    int vertexIdx;

    if(vertexCount == 0) {
        return;
    }
    vertex = vertices;
    maxPoint = *vertex;
    minPoint = *vertex;
    vertex++;
    for(vertexIdx=1; vertexIdx<vertexCount; vertexIdx++, vertex++) {
        if(maxPoint.x < vertex->x) {
            maxPoint.x = vertex->x;
        } else if(minPoint.x > vertex->x) {
            minPoint.x = vertex->x;
        }
        if(maxPoint.y < vertex->y) {
            maxPoint.y = vertex->y;
        } else if(minPoint.y > vertex->y) {
            minPoint.y = vertex->y;
        }
        if(maxPoint.z < vertex->z) {
            maxPoint.z = vertex->z;
        } else if(minPoint.z > vertex->z) {
            minPoint.z = vertex->z;
        }
    }
    center->x = (minPoint.x+maxPoint.x)/2.0f;
    center->y = (minPoint.y+maxPoint.y)/2.0f;
    center->z = (minPoint.z+maxPoint.z)/2.0f;
    maxDistanceSquared = 0;
    for(vertexIdx=vertexCount; vertexIdx--;) {
        VECSubtract(&vertices[vertexIdx], center, &fromCenter);
        distanceSquared = VECSquareMag(&fromCenter);
        if(maxDistanceSquared < distanceSquared) {
            maxDistanceSquared = distanceSquared;
        }
    }
    *boundRadius = sqrtf(maxDistanceSquared);
}

// Expands a mesh face into one triangle's vertex indices for collision setup.
// Used for triangle setup and each mesh collision or segment query.
static void MakeIndexBuf(int *out, HSF_FACE *hsfFaceP, int idx)
{
    switch(hsfFaceP->type) {
        case HSF_FACE_TRI:
            out[0] = hsfFaceP->index[0].vertex;
            out[1] = hsfFaceP->index[1].vertex;
            out[2] = hsfFaceP->index[2].vertex;
            break;

        case HSF_FACE_QUAD:
            if(idx & 0x1) {
                out[0] = hsfFaceP->index[0].vertex;
                out[1] = hsfFaceP->index[1].vertex;
                out[2] = hsfFaceP->index[3].vertex;
            } else {
                out[0] = hsfFaceP->index[0].vertex;
                out[1] = hsfFaceP->index[3].vertex;
                out[2] = hsfFaceP->index[2].vertex;
            }

            break;

        case HSF_FACE_TRISTRIP:
            switch(idx) {
                case 0:
                    out[0] = hsfFaceP->index[0].vertex;
                    out[2] = hsfFaceP->index[1].vertex;
                    out[1] = hsfFaceP->index[2].vertex;
                    break;

                case 1:
                    out[0] = hsfFaceP->index[1].vertex;
                    out[1] = hsfFaceP->index[2].vertex;
                    out[2] = hsfFaceP->strip.data[0].vertex;
                    break;

                case 2:
                    out[0] = hsfFaceP->index[2].vertex;
                    out[2] = hsfFaceP->strip.data[0].vertex;
                    out[1] = hsfFaceP->strip.data[1].vertex;
                    break;

                default:
                    if(idx & 0x1) {
                        out[0] = hsfFaceP->strip.data[idx-3].vertex;
                        out[1] = hsfFaceP->strip.data[idx-2].vertex;
                        out[2] = hsfFaceP->strip.data[idx-1].vertex;
                    } else {
                        out[0] = hsfFaceP->strip.data[idx-3].vertex;
                        out[2] = hsfFaceP->strip.data[idx-2].vertex;
                        out[1] = hsfFaceP->strip.data[idx-1].vertex;
                    }
                    break;
            }
            break;
    }
}

// Converts an HSF material collision attribute into the collision-system bit.
// Used when recording mesh contacts and returning polygon-query material codes.
static inline u32 ColMatCodeGet(u32 matAttr)
{
    switch(matAttr & COL_MAT_ATTR_MASK) {
        case COL_MAT_ATTR_1_BIT_PATTERN:
            return (1 << 0);

        case COL_MAT_ATTR_2_BIT_PATTERN:
            return (1 << 1);

        case COL_MAT_ATTR_3_BIT_PATTERN:
            return (1 << 2);

        case COL_MAT_ATTR_4_BIT_PATTERN:
            return (1 << 3);

        case COL_MAT_ATTR_5_BIT_PATTERN:
            return (1 << 4);

        case COL_MAT_ATTR_6_BIT_PATTERN:
            return (1 << 5);

        case COL_MAT_ATTR_7_BIT_PATTERN:
            return (1 << 6);

        case COL_MAT_ATTR_8_BIT_PATTERN:
            return (1 << 7);

        case COL_MAT_ATTR_9_BIT_PATTERN:
            return (1 << 8);

        case COL_MAT_ATTR_10_BIT_PATTERN:
            return (1 << 9);

        case COL_MAT_ATTR_11_BIT_PATTERN:
            return (1 << 10);

        case COL_MAT_ATTR_12_BIT_PATTERN:
            return (1 << 11);

        case COL_MAT_ATTR_13_BIT_PATTERN:
            return (1 << 12);

        case COL_MAT_ATTR_14_BIT_PATTERN:
            return (1 << 13);

        default:
            return 0;
    }
}

// Maps a collision material bit to its parameter slot, or -1 when unsupported.
// Used for contact response and the public surface-parameter accessors.
static int ColCodeGet(u32 code)
{
    // The low group takes priority; bit 0x4000 selects this group but has no parameter slot.
    // With only 0x4000 in this group, valid high-group surface bits are ignored.
    if(code & 0x407F) {
        if(code & 0x1) {
            return 0;
        } else if(code & 0x2) {
            return 1;
        } else if(code & 0x4) {
            return 2;
        } else if(code & 0x8) {
            return 3;
        } else if(code & 0x10) {
            return 4;
        } else if(code & 0x20) {
            return 5;
        } else if(code & 0x40) {
            return 6;
        }
    } else if(code & 0xBF80) {
        if(code & 0x80) {
            return 7;
        } else if(code & 0x100) {
            return 8;
        } else if(code & 0x200) {
            return 9;
        } else if(code & 0x400) {
            return 10;
        } else if(code & 0x800) {
            return 11;
        } else if(code & 0x1000) {
            return 12;
        } else if(code & 0x2000) {
            return 13;
        }
    }
    return -1;
}

// Builds collision transforms recursively for supported HSF objects.
// ColMtxCalcModelAll calls it during setup and refresh; it then visits child objects recursively.
static void ColMtxCalcHsfObject(HSF_OBJECT *obj, Mtx mtx)
{
    HSF_TRANSFORM *trxP;
    int no;
    BOOL processF = FALSE;
    COLMESH *meshP;
    Mtx objMtx;
    Mtx rot;
    Mtx final;
    switch(obj->type) {
        case HSF_OBJ_NULL1:
        case HSF_OBJ_MESH:
        case HSF_OBJ_ROOT:
        case HSF_OBJ_JOINT:
        case HSF_OBJ_NULL2:
        case HSF_OBJ_NULL3:
        case HSF_OBJ_MAP:
            processF = TRUE;
            break;

        case HSF_OBJ_REPLICA:
        case HSF_OBJ_CAMERA:
        case HSF_OBJ_LIGHT:
            break;
    }
    if(!processF) {
        return;
    }
    if(!CancelTRXF) {
        trxP = &obj->mesh.base;
    } else {
        trxP = &obj->mesh.curr;
    }
    MTXScale(final, trxP->scale.x, trxP->scale.y, trxP->scale.z);
    ColMtxRot(rot, trxP->rot.x, trxP->rot.y, trxP->rot.z);
    MTXConcat(rot, final, final);
    ColMtxTransApply(final, trxP->pos.x, trxP->pos.y, trxP->pos.z);
    MTXConcat(mtx, final, objMtx);
    meshP = SearchHsfObjectColMesh(obj);
    if(meshP) {
        memcpy(meshP->mtx, objMtx, sizeof(Mtx));
    }
    for(no=obj->mesh.childNum; no--;) {
        ColMtxCalcHsfObject(obj->mesh.child[no], objMtx);
    }
}

// Refreshes collision transforms for every model registered with the map.
// Called by ColMapInit and UpdateMeshMtx; animated models use current object transforms.
static void ColMtxCalcModelAll(void)
{
    HU3D_MODEL *modelP;
    Mtx final;
    Mtx rot;
    int no;
    for(no=colWork.mdlNum; no--;) {
        modelP = &Hu3DData[colWork.mdlId[no]];
        ColMtxRot(rot, modelP->rot.x, modelP->rot.y, modelP->rot.z);
        MTXScale(final, modelP->scale.x, modelP->scale.y, modelP->scale.z);
        MTXConcat(modelP->mtx, rot, rot);
        MTXConcat(rot, final, final);
        ColMtxTransApply(final, modelP->pos.x, modelP->pos.y, modelP->pos.z);
        CancelTRXF = modelP->motId != HU3D_MOTIONID_NONE;
        ColMtxCalcHsfObject(modelP->hsf->root, final);
    }
}

#define CLAMP_EPSILON(x) ((fabs(x) < 0.001f) ? 0 : (x))

// Reports whether two distances lie on the same side of a collision plane.
// Used by ColEjectDistGet and ColPlaneDistGet; two zero distances also have the same sign.
static inline BOOL SameSign(float x, float y)
{
    int signA = GetSign(x);
    int signB = GetSign(y);
    BOOL result = signA == signB;
    (void)signA;
    (void)signA;
    (void)signB;
    (void)signB;
    (void)result;

    return result;
}

// Checks that a projected point is inside all three oriented triangle edges.
// Used by plane and mesh collision tests; extFlag does not change the returned result.
static inline BOOL ColPlaneEdgeCheck(HuVecF a, HuVecF b, HuVecF c, COLTRI *tri, BOOL extFlag)
{
    HuVecF ab;
    HuVecF ac;
    VECSubtract(&a, &b, &ab);
    VECSubtract(&a, &c, &ac);

    if (VECDotProduct(&ab, &tri->edgeNorm1) >= 0 && VECDotProduct(&ac, &tri->edgeNorm2) >= 0 &&
        VECDotProduct(&ab, &tri->edgeNorm3) >= 0) {
        return TRUE;
    } else {
        // The extended test still returns false when a point is outside an edge.
        if(extFlag) {
           int a;
           (void)a;
           (void)a;
           (void)0.0001f;
        }
        return FALSE;
    }
}

// Tests one or both vertical ends of a body against a triangle plane and returns the shallowest
// overlap.
// Used by _BodyMeshCylCol; its signed support offset makes the non-flat test use the lower Y end
// for either face-normal direction, or the center when the Y offset is zero.
static inline BOOL ColPlaneYCheck(HuVecF *center, HuVecF *dir, COLTRI *tri, HuVecF *out, BOOL flatF)
{
    HuVecF temp;
    HuVecF vtx[4];

    int i;
    float d;
    float mind;
    BOOL result;
    int num;
    result = FALSE;

    if(flatF) {
        vtx[0].x = 0;
        vtx[0].z = 0;
        vtx[1].x = 0;
        vtx[1].z = 0;
        vtx[0].y = dir->y;
        vtx[1].y = -dir->y;
        num = 2;
    } else {
        vtx[0].x = 0;
        vtx[0].z = 0;
        if(tri->norm.y >= 0) {
            vtx[0].y = -dir->y;
        } else {
            vtx[0].y = dir->y;
        }
        num=1;
    }
    for(i=0; i<num; i++) {
        VECAdd(center, &vtx[i], &temp);
        d = tri->d+VECDotProduct(&tri->norm, &temp);
        if(d >= -0.001f) {
            if(result == FALSE || d < mind) {
                result = TRUE;
                mind = d;
                *out = vtx[i];
            }
        }
    }

    return result;
}

// Tests body corner points against a triangle plane and selects its shallowest overlap.
// Used by ColEjectDistGet and mesh collision passes to select a support corner.
static BOOL ColPlaneCheck(HuVecF *center, HuVecF *size, COLTRI *tri, HuVecF *eject, BOOL flatF)
{
    HuVecF vtx[4];
    HuVecF temp;
    int i;
    BOOL result;
    int num;

    float d;
    float mind;
    result = FALSE;

    // Only the (-X,-Z) and (+X,+Z) corners are sampled; flatF repeats them at both Y ends.
    if(flatF) {
        vtx[0].x = -size->x;
        vtx[0].z = -size->z;
        vtx[1].x = size->x;
        vtx[1].z = size->z;
        vtx[2].x = -size->x;
        vtx[2].z =  -size->z;
        vtx[3].x = size->x;
        vtx[3].z = size->z;
        vtx[0].y = -size->y;
        vtx[1].y = -size->y;
        vtx[2].y = size->y;
        vtx[3].y = size->y;
        num = 4;
    } else {
        vtx[0].x = -size->x;
        vtx[0].z = -size->z;
        vtx[1].x = size->x;
        vtx[1].z = size->z;
        if(tri->norm.y >= 0) {
            vtx[0].y = -size->y;
            vtx[1].y = -size->y;
        } else {
            vtx[0].y = size->y;
            vtx[1].y = size->y;
        }
        num = 2;
    }
    for(i=0; i<num; i++) {
        VECAdd(center, &vtx[i], &temp);
        d = tri->d+VECDotProduct(&tri->norm, &temp);
        if(d >= -0.001f) {
            if(result == FALSE || d < mind) {
                result = TRUE;
                mind = d;
                *eject = vtx[i];
            }
        }
    }
    return result;
}

// Finds a body's plane-crossing movement fraction or signed initial plane distance.
// Used by _BodyMeshCylCol; negative results distinguish rejected and overlap cases.
static int ColEjectDistGet(HuVecF *start, HuVecF *end, HuVecF *dir, HuVecF *size, float height,
                           float maxDist, float minDist, HuVecF *vtx, int *vtxIdx, COLTRI *tri,
                           float *outDist)
{
    float dist;

    float startD;
    float roundStartD;
    float roundEndD;

    float endD;

    HuVecF a;
    HuVecF delta;
    HuVecF eject;
    HuVecF perp;

    startD = tri->d+VECDotProduct(&tri->norm, start)-height;
    endD = (tri->d+VECDotProduct(&tri->norm, end))-height;
    roundStartD = CLAMP_EPSILON(startD);
    roundEndD = CLAMP_EPSILON(endD);
    if(roundStartD < 0 || SameSign(roundStartD, roundEndD)) {
        float dot = (tri->d+VECDotProduct(&tri->norm, start))+height;
        if(dot > 0 && roundEndD < 0) {
            VECScale(&tri->norm, &perp, startD);
            VECSubtract(start, &perp, &a);
            if(!ColPlaneCheck(&a, size, tri, &eject, FALSE)) {
                *outDist = startD;
                return -5;
            }
            VECAdd(&a, &eject, &a);
            if(!ColPlaneEdgeCheck(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
                // Leave outDist untouched when the projected initial overlap lies outside the
                // triangle.
                return -6;
            } else {
                *outDist = startD;
                return -5;
            }
        } else {
            return -1;
        }
    } else {
        float dot = VECDotProduct(&tri->norm, dir);
        if(fabs(dot) < 0.001f) {
            return -3;
        }
        dist = -startD/dot;
        if ((dist < minDist && (minDist - dist) > 0.001f) ||
            (maxDist < dist && (dist - maxDist) > 0.001f)) {
            return -2;
        }
        if(dist < minDist) {
            dist = minDist;
        }
        VECScale(dir, &delta, dist);
        VECAdd(start, &delta, &a);
        // Use the selected support corner without checking ColPlaneCheck's return value.
        ColPlaneCheck(start, size, tri, &eject, FALSE);
        VECAdd(&a, &eject, &a);
        if(!ColPlaneEdgeCheck(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
            return -4;
        } else {
            *outDist = dist;
            return 0;
        }
    }
}

// Finds a swept point's contact with a triangle plane and its edges.
// outDist is a movement fraction on a crossing and the signed start distance for an initial
// overlap.
// Used by _BodyMeshCol and ColMapPolyGet.
static int ColPlaneDistGet(HuVecF *start, HuVecF *end, HuVecF *dir, float height, float maxDist,
                           float minDist, HuVecF *vtx, int *vtxIdx, COLTRI *tri, float *outDist)
{
    float dist;
    float startD;
    float roundStartD;
    float roundEndD;

    float endD;

    HuVecF a;
    HuVecF delta;
    HuVecF perp;

    startD = (tri->d+VECDotProduct(&tri->norm, start))-height;
    endD = (tri->d+VECDotProduct(&tri->norm, end))-height;
    roundStartD = CLAMP_EPSILON(startD);
    roundEndD = CLAMP_EPSILON(endD);
    if(roundStartD < 0 || SameSign(roundStartD, roundEndD)) {
        float dot = (tri->d+VECDotProduct(&tri->norm, start))+height;
        if(dot > 0 && roundEndD < 0) {
            *outDist = startD;
            VECScale(&tri->norm, &perp, startD);
            VECSubtract(start, &perp, &a);
            if(!ColPlaneEdgeCheck(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
                return -6;
            } else {
                return -5;
            }
        } else {
            return -1;
        }
    } else {
        float dot = VECDotProduct(&tri->norm, dir);
        if(fabs(dot) < 0.001f) {
            return -3;
        }
        dist = -startD/dot;
        if ((dist < minDist && (minDist - dist) > 0.001f) ||
            (maxDist < dist && (dist - maxDist) > 0.001f)) {
            return -2;
        }
        if(dist < minDist) {
            dist = minDist;
        }
        VECScale(dir, &delta, dist);
        VECAdd(start, &delta, &a);
        if(!ColPlaneEdgeCheck(a, vtx[vtxIdx[0]], vtx[vtxIdx[1]], tri, FALSE)) {
            return -4;
        } else {
            *outDist = dist;
            return 0;
        }
    }
}

typedef struct ColLine_s {
    float endA; // Upper vertical bound at the start of movement.
    float startA; // Lower vertical bound at the start of movement.
    float endB; // Upper bound for the overlap precheck; edge tests reuse their starting bounds.
    float startB; // Lower bound for the overlap precheck; edge tests reuse their starting bounds.
} COLLINE;

// Calculates a vertical contact time for cylinder edge and body-pair tests.
// Sets result and returns zero when the supplied overlap-precheck bounds overlap; otherwise
// solves from the start bounds. Edge tests reuse their starting bounds for the precheck.
static inline float _GetColLineTime(COLLINE *a, COLLINE *b, float t, BOOL *result)
{
    COLLINE *maxP;
    COLLINE *minP;
    float ret;
    float delta;

    if((a->endB-a->startB) > (b->endB-b->startB)) {
        maxP = a;
        minP = b;
    } else {
        maxP = b;
        minP = a;
    }
    *result = FALSE;
    if(maxP->startB <= minP->endB && minP->endB <= maxP->endB) {
        *result = TRUE;
    } else if(maxP->startB <= minP->startB && minP->startB <= maxP->endB) {
        *result = TRUE;
    }
    if(*result) {
        return 0;
    }
    if(fabsf(t) < 0.0001f)  {
        return -1;
    }
    if(a->endA > b->endA) {
        if(t > 0) {
            return -1;
        } else {
            delta = a->startA-b->endA;
            ret = -(delta/t);
        }
    } else {
        if(t < 0) {
            return -1;
        }
        delta = a->endA-b->startA;
        ret = -(delta/t);
    }

    return ret;
}

// Finds closest points on the movement and edge lines for _EdgeCylCol's distance test.
static inline BOOL _EdgeCylColInline(HuVecF *startPos, HuVecF *movement, HuVecF *edgeStart,
                                     HuVecF *edgeEnd, HuVecF *edgeDelta,
                                     float *movementClosestParam, float *edgeClosestParam)
{
    float determinant;
    float movementMagSquared;
    float movementEdgeDot;
    float edgeMagSquared;
    float startMovementDot;
    float startEdgeDot;

    HuVecF edgeFromStart;

    // edgeEnd is unused; edgeDelta supplies the direction of the edge line.
    VECSubtract(edgeStart, startPos, &edgeFromStart);

    movementMagSquared = VECDotProduct(movement, movement);
    movementEdgeDot = VECDotProduct(movement, edgeDelta);
    edgeMagSquared = VECDotProduct(edgeDelta, edgeDelta);
    startMovementDot = VECDotProduct(&edgeFromStart, movement);
    startEdgeDot = VECDotProduct(&edgeFromStart, edgeDelta);
    determinant = (movementMagSquared*edgeMagSquared)-(movementEdgeDot*movementEdgeDot);
    if(fabsf(determinant) < 0.0001f) {
        return FALSE;
    }
    *movementClosestParam =
        ((edgeMagSquared * startMovementDot) - (movementEdgeDot * startEdgeDot)) / determinant;
    *edgeClosestParam =
        ((movementEdgeDot * startMovementDot) - (movementMagSquared * startEdgeDot)) / determinant;
    return TRUE;
}

// Computes the dot product in double precision for cylinder collision math.
// Used by cylinder and capsule edge tests to form their quadratic coefficients.
static inline double GetVecDot(HuVecF a, HuVecF b)
{
    return ((double)a.x*b.x)+((double)a.y*b.y)+((double)a.z*b.z);

}

// Finds horizontal cylinder contact with an edge for _BodyEdgeCylCol; reports initial overlap.
static BOOL _EdgeCylCol(HuVecF *start, HuVecF *movement, float radius, HuVecF edgeStart,
                        HuVecF edgeEnd, float *contactTime, BOOL *overlapAtStart)
{
    BOOL movementZero;
    BOOL edgeZero;

    double discriminant;
    double quadraticA;
    double quadraticB;
    double firstRoot;

    float projectionParam;
    float radiusSquared;
    float projectedEndTime;
    double quadraticC;

    double edgeUnit[3];
    HuVecF edgeDelta;
    HuVecF closestMovementPoint;
    HuVecF closestEdgePoint;
    HuVecF closestSeparation;
    HuVecF degenerateSeparation;
    HuVecF pointSeparation;
    HuVecF startFromEdge;
    HuVecF contactPosition;

    double movementMagSquared;
    double startAlongEdge;
    double movementAlongEdge;
    double edgeLength;

    float movementClosestParam;
    float edgeClosestParam;
    float pointExitTime;
    float edgeContactParam;
    float edgeExitTime;

    edgeStart.y = 0;
    edgeEnd.y = 0;
    *overlapAtStart = FALSE;
    VECSubtract(&edgeEnd, &edgeStart, &edgeDelta);
    radiusSquared = radius*radius;

    if (_EdgeCylColInline(start, movement, &edgeStart, &edgeEnd, &edgeDelta, &movementClosestParam,
                          &edgeClosestParam)) {
        VECScale(movement, &closestMovementPoint, movementClosestParam);
        VECAdd(start, &closestMovementPoint, &closestMovementPoint);
        VECScale(&edgeDelta, &closestEdgePoint, edgeClosestParam);
        VECAdd(&edgeStart, &closestEdgePoint, &closestEdgePoint);
        VECSubtract(&closestMovementPoint, &closestEdgePoint, &closestSeparation);
        if(VECSquareMag(&closestSeparation) > radiusSquared) {
            return FALSE;
        }
    } else {
        movementZero = VECSquareMag(movement) < 0.0001f;
        edgeZero = VECSquareMag(&edgeDelta) < 0.0001f;
        if(movementZero && edgeZero) {
            VECSubtract(&edgeStart, start, &degenerateSeparation);
            if(VECSquareMag(&degenerateSeparation) > radiusSquared) {
                return FALSE;
            } else {
                *contactTime = 0;
                *overlapAtStart = TRUE;
                return TRUE;
            }
        }
        if(!movementZero && !edgeZero) {
            edgeClosestParam =
                (VECDotProduct(start, &edgeDelta) - VECDotProduct(&edgeStart, &edgeDelta)) /
                VECSquareMag(&edgeDelta);
            VECScale(&edgeDelta, &degenerateSeparation, edgeClosestParam);
            VECAdd(&edgeStart, &degenerateSeparation, &degenerateSeparation);
            VECSubtract(&degenerateSeparation, start, &degenerateSeparation);
            if(VECSquareMag(&degenerateSeparation) > radiusSquared) {
                return FALSE;
            }
            projectionParam =
                (VECDotProduct(&edgeStart, movement) - VECDotProduct(start, movement)) /
                VECSquareMag(movement);
            projectedEndTime =
                (VECDotProduct(&edgeEnd, movement) - VECDotProduct(start, movement)) /
                VECSquareMag(movement);
            if(VECDotProduct(&edgeDelta, movement) > 0) {
                if(0 <= projectionParam && projectionParam <= 1.0f) {
                    *contactTime = projectionParam;
                     return TRUE;
                } else if(0.0f <= projectedEndTime && projectedEndTime <= 1.0f) {
                    *contactTime = 0;
                    *overlapAtStart = TRUE;
                    return TRUE;
                }
            } else {
                if(0 <= projectedEndTime && projectedEndTime <= 1.0f) {
                    *contactTime = projectedEndTime;
                     return TRUE;
                } else if(0 <= projectionParam && projectionParam <= 1.0f) {
                    *contactTime = 0;
                    *overlapAtStart = TRUE;
                    return TRUE;
                }
            }
            projectionParam =
                (VECDotProduct(start, &edgeDelta) - VECDotProduct(&edgeStart, &edgeDelta)) /
                VECSquareMag(&edgeDelta);
            if(0 <= projectionParam && projectionParam <= 1.0f) {
                *contactTime = 0;
                *overlapAtStart = TRUE;
                return TRUE;
            } else {
                return FALSE;
            }
        } else if(movementZero) {
            VECSubtract(&edgeStart, start, &pointSeparation);
            quadraticA = GetVecDot(edgeDelta, edgeDelta);
            quadraticB = GetVecDot(edgeDelta, pointSeparation);
            quadraticC = GetVecDot(pointSeparation, pointSeparation)-radiusSquared;

        } else if(edgeZero) {
            VECSubtract(start, &edgeStart, &pointSeparation);
            quadraticA = GetVecDot(*movement, *movement);
            quadraticB = GetVecDot(*movement, pointSeparation);
            quadraticC = GetVecDot(pointSeparation, pointSeparation)-radiusSquared;
        }
        discriminant = (quadraticB*quadraticB)-(quadraticA*quadraticC);
        if(discriminant < 0) {
            if(discriminant < -fabsf(discriminant*0.0001f)) {
                return FALSE;
            } else {
                discriminant = 0;
            }
        } else {
            discriminant = sqrt(discriminant);
        }
        if(fabsf(quadraticA) < 0.0001f) {
            return FALSE;
        }
        firstRoot = (-quadraticB-discriminant)/quadraticA;
        if(firstRoot < 0) {
            pointExitTime = (-quadraticB+discriminant)/quadraticA;
            if(pointExitTime < 0) {
                return FALSE;
            } else {
                firstRoot = 0;
                *overlapAtStart = TRUE;
            }
        }
        if(firstRoot < -0.001f || 1.001f < firstRoot) {
            return FALSE;
        }
        if(movementZero) {
            *contactTime = 0;
            *overlapAtStart = TRUE;
        } else {
            *contactTime = firstRoot;
        }
        return TRUE;
    }
    {
        VECSubtract(start, &edgeStart, &startFromEdge);
        movementMagSquared = GetVecDot(*movement, *movement);
        edgeLength = sqrt(((double)edgeDelta.x*edgeDelta.x)+((double)edgeDelta.z*edgeDelta.z));
        edgeUnit[0] = edgeDelta.x/edgeLength;
        edgeUnit[1] = 0;
        edgeUnit[2] = edgeDelta.z/edgeLength;
        movementAlongEdge = (edgeUnit[0]*movement->x)+(edgeUnit[2]*movement->z);
        startAlongEdge = (edgeUnit[0]*startFromEdge.x)+(edgeUnit[2]*startFromEdge.z);
        quadraticA = movementMagSquared-(movementAlongEdge*movementAlongEdge);
        quadraticB = GetVecDot(startFromEdge, *movement)-(movementAlongEdge*startAlongEdge);
        quadraticC = (GetVecDot(startFromEdge, startFromEdge) - (startAlongEdge * startAlongEdge)) -
                     radiusSquared;
        discriminant = (quadraticB*quadraticB)-(quadraticA*quadraticC);
        if(discriminant < 0) {
            if(discriminant < -fabsf(discriminant*0.0001f)) {
                return FALSE;
            } else {
                discriminant = 0;
            }
        } else {
            discriminant = sqrt(discriminant);
        }
        if(fabsf(quadraticA) < 0.0001f) {
            return FALSE;
        }
        firstRoot = (-quadraticB-discriminant)/quadraticA;
        if(firstRoot < 0) {
            edgeExitTime = (-quadraticB+discriminant)/quadraticA;
            if(edgeExitTime < 0) {
                return FALSE;
            } else {
                firstRoot = 0;
                *overlapAtStart = TRUE;
            }
        }
        VECScale(movement, &contactPosition, firstRoot);
        VECAdd(start, &contactPosition, &contactPosition);
        edgeContactParam =
            (VECDotProduct(&contactPosition, &edgeDelta) - VECDotProduct(&edgeStart, &edgeDelta)) /
            VECSquareMag(&edgeDelta);
        if(edgeContactParam < -0.001f || edgeContactParam > 1.001f) {
            return FALSE;
        }
        *contactTime = firstRoot;
        return TRUE;
    }
}

// Finds contact between a moving collision body and one edge of a mesh triangle.
// Used by _BodyTriCylCol to combine horizontal contact with vertical interval overlap.
static BOOL _BodyEdgeCylCol(HuVecF *startPos, HuVecF *movement, COLBODY *bodyP, HuVecF *vtxStart,
                            HuVecF *vtxEnd, float *colT, BOOL *colValidF, HuVecF *out)
{
    BOOL lineValid;

    float t;
    float dot;
    float halfH;
    float t2;
    float edgeColT;
    BOOL edgeColF;

    HuVecF vtxDelta;
    HuVecF ejectPos;
    HuVecF horizontalMovement;
    HuVecF dir;
    HuVecF ejectVec;
    HuVecF start;
    HuVecF delta;
    HuVecF lineStart;
    HuVecF lineEnd;
    COLLINE lineA;
    COLLINE lineB;
    HuVecF endColPos;
    HuVecF startColPos;

    VECSubtract(vtxEnd, vtxStart, &vtxDelta);
    if(VECSquareMag(&vtxDelta) < 0.0001f) {
        return FALSE;
    }
    halfH = bodyP->param.height/2;
    lineA.endA = startPos->y+halfH;
    lineA.startA = startPos->y-halfH;
    lineA.endB = startPos->y+halfH;
    lineA.startB = startPos->y-halfH;
    dir = vtxDelta;
    dir.y = 0;

    if(VECSquareMag(&dir) < 0.001f) {
        if(vtxStart->y > vtxEnd->y) {
            lineB.endA = vtxStart->y;
            lineB.startA = vtxEnd->y;
            lineB.endB = vtxStart->y;
            lineB.startB = vtxEnd->y;
        } else {
            lineB.endA = vtxEnd->y;
            lineB.startA = vtxStart->y;
            lineB.endB = vtxEnd->y;
            lineB.startB = vtxStart->y;
        }
        t = _GetColLineTime(&lineA, &lineB, movement->y, &lineValid);
        if(t < 0 || 1 < t) {
            return FALSE;
        }
    } else {
        VECNormalize(&dir, &dir);
        VECScale(&dir, &ejectVec, bodyP->param.radius);
        VECAdd(startPos, &ejectVec, &ejectPos);
        start = *vtxStart;
        delta = vtxDelta;
        start.y = 0;
        ejectPos.y = 0;
        delta.y = 0;
        dot = (VECDotProduct(&ejectPos, &delta)-VECDotProduct(&start, &delta))/VECSquareMag(&delta);
        VECScale(&vtxDelta, &lineStart, dot);
        VECAdd(vtxStart, &lineStart, &lineStart);
        VECAdd(startPos, &ejectVec, &ejectPos);
        VECAdd(&ejectPos, movement, &ejectPos);
        ejectPos.y = 0;
        dot = (VECDotProduct(&ejectPos, &delta)-VECDotProduct(&start, &delta))/VECSquareMag(&delta);
        VECScale(&vtxDelta, &lineEnd, dot);
        VECAdd(vtxStart, &lineEnd, &lineEnd);
        if(lineStart.y > lineEnd.y) {
            lineB.endA = lineStart.y;
            lineB.startA = lineEnd.y;
            lineB.endB = lineStart.y;
            lineB.startB = lineEnd.y;
        } else {
            lineB.endA = lineEnd.y;
            lineB.startA = lineStart.y;
            lineB.endB = lineEnd.y;
            lineB.startB = lineStart.y;
        }
        t = _GetColLineTime(&lineA, &lineB, movement->y, &lineValid);
        if(t < 0 || 1 < t) {
            t = -1;
        }
        VECSubtract(startPos, &ejectVec, &ejectPos);
        start = *vtxStart;
        delta = vtxDelta;
        start.y = 0;
        ejectPos.y = 0;
        delta.y = 0;
        dot = (VECDotProduct(&ejectPos, &delta)-VECDotProduct(&start, &delta))/VECSquareMag(&delta);
        VECScale(&vtxDelta, &lineStart, dot);
        VECAdd(vtxStart, &lineStart, &lineStart);
        PSVECSubtract(startPos, &ejectVec, &ejectPos);
        VECAdd(&ejectPos, movement, &ejectPos);
        ejectPos.y = 0;
        dot = (VECDotProduct(&ejectPos, &delta)-VECDotProduct(&start, &delta))/VECSquareMag(&delta);
        VECScale(&vtxDelta, &lineEnd, dot);
        VECAdd(vtxStart, &lineEnd, &lineEnd);
        if(lineStart.y > lineEnd.y) {
            lineB.endA = lineStart.y;
            lineB.startA = lineEnd.y;
            lineB.endB = lineStart.y;
            lineB.startB = lineEnd.y;
        } else {
            lineB.endA = lineEnd.y;
            lineB.startA = lineStart.y;
            lineB.endB = lineEnd.y;
            lineB.startB = lineStart.y;
        }
        // This second test overwrites lineValid even if the first test's time remains selected.
        t2 = _GetColLineTime(&lineA, &lineB, movement->y, &lineValid);
        if(0.0f <= t2 && t2 <= 1.0f && (t < 0 || t2 < t)) {
            t = t2;
        }
        if(t < 0 || 1 < t) {
            return FALSE;
        }
    }
    ejectPos = *startPos;
    horizontalMovement = *movement;
    ejectPos.y = 0;
    horizontalMovement.y = 0;
    if (!_EdgeCylCol(&ejectPos, &horizontalMovement, bodyP->param.radius, *vtxStart, *vtxEnd,
                     &edgeColT, &edgeColF)) {
        return FALSE;
    }
    *colT = (edgeColT >= t) ? edgeColT : t;
    *colValidF = lineValid && edgeColF;
    if(out) {
        VECScale(movement, &endColPos, *colT);
        VECAdd(startPos, &endColPos, &endColPos);
        dot = (VECDotProduct(&endColPos, &vtxDelta) - VECDotProduct(vtxStart, &vtxDelta)) /
              VECSquareMag(&vtxDelta);
        if(dot < 0) {
            dot = 0;
        } else if(dot > 1) {
            dot = 1;
        }
        VECScale(&vtxDelta, &startColPos, dot);
        VECAdd(vtxStart, &startColPos, &startColPos);
        VECSubtract(&endColPos, &startColPos, out);
    }
    return TRUE;
}

// Selects a triangle edge hit for _BodyMeshCylCol; returns its index, or -1 for no hit.
// A later initial overlap always replaces the hit; a later earlier-time hit can replace it too.
static inline BOOL _BodyTriCylCol(HuVecF *start, HuVecF *movement, COLBODY *body, HuVecF *vertices,
                                  int *vertexIndices, float *contactTime, HuVecF *separation)
{
    HuVecF candidateSeparation;
    HuVecF *separationOutput;
    int hitEdge;
    BOOL edgeHit;
    int initialOverlap;
    float candidateTime;

    int edgeIdx;

    edgeHit =
        _BodyEdgeCylCol(start, movement, body, &vertices[vertexIndices[2]],
                        &vertices[vertexIndices[0]], contactTime, &initialOverlap, separation);
    if(edgeHit) {
        hitEdge = 2;
    } else {
        hitEdge = -1;
    }
    separationOutput = (separation) ? &candidateSeparation : NULL;
    for(edgeIdx=0; edgeIdx<2; edgeIdx++) {
        edgeHit = _BodyEdgeCylCol(start, movement, body, &vertices[vertexIndices[edgeIdx]],
                                  &vertices[vertexIndices[edgeIdx + 1]], &candidateTime,
                                  &initialOverlap, separationOutput);
        if(edgeHit) {
            if (hitEdge < 0 || initialOverlap ||
                (candidateTime >= 0 && *contactTime > candidateTime)) {
                if(separation) {
                    *separation = candidateSeparation;
                }
                *contactTime = candidateTime;
                hitEdge = edgeIdx;
            }
        }
    }
    return hitEdge;
}

// Tests all three triangle edges during _BodyMeshCol and returns an earlier capsule contact.
static BOOL _ColCapsuleEdgeCalc(HuVecF *startPosition, HuVecF *endPosition, HuVecF *movement,
                                float radius, float bestTime, float minimumTime, HuVecF *vertices,
                                int *vertexIndices, float *contactTime, HuVecF *separation)
{
    HuVecF *edgeStart;
    int edgeIdx;
    HuVecF *edgeEnd;

    double discriminant;
    float candidateTime;
    double quadraticA;
    double quadraticB;
    double startAlongEdge;
    double movementAlongEdge;
    double edgeLength;
    float edgeContactParam;

    double edgeUnit[3];
    HuVecF edgeDelta;
    HuVecF contactPosition;
    HuVecF edgeProjection;
    HuVecF parallelSeparation;

    double movementMagSquared;
    double radiusSquared;
    double quadraticC;
    float exitTime;

    // endPosition is unused; movement contains the displacement for this test.
    *contactTime = bestTime;
    separation->x = 0;
    separation->y = 0;
    separation->z = 0;
    movementMagSquared = GetVecDot(*movement, *movement);
    radiusSquared = radius*radius;
    for(edgeIdx=3; edgeIdx--;) {
        edgeStart = &vertices[vertexIndices[edgeIdx]];
        edgeEnd = &vertices[vertexIndices[(edgeIdx+1)%3]];
        VECSubtract(edgeEnd, edgeStart, &edgeDelta);
        VECSubtract(startPosition, edgeStart, &edgeProjection);
        if(VECSquareMag(&edgeDelta) < 0.0001f) {
            continue;
        }
        edgeLength =
            sqrt(((double) edgeDelta.x * edgeDelta.x) + ((double) edgeDelta.y * edgeDelta.y) +
                 ((double) edgeDelta.z * edgeDelta.z));
        edgeUnit[0] = edgeDelta.x/edgeLength;
        edgeUnit[1] = edgeDelta.y/edgeLength;
        edgeUnit[2] = edgeDelta.z/edgeLength;
        movementAlongEdge =
            (edgeUnit[0] * movement->x) + (edgeUnit[1] * movement->y) + (edgeUnit[2] * movement->z);
        startAlongEdge = (edgeUnit[0] * edgeProjection.x) + (edgeUnit[1] * edgeProjection.y) +
                         (edgeUnit[2] * edgeProjection.z);
        quadraticA = movementMagSquared-(movementAlongEdge*movementAlongEdge);
        quadraticB = GetVecDot(edgeProjection, *movement)-(movementAlongEdge*startAlongEdge);
        quadraticC =
            (GetVecDot(edgeProjection, edgeProjection) - (startAlongEdge * startAlongEdge)) -
            radiusSquared;
        // Parallel movement can use the minimum time for an existing edge overlap.
        // This branch leaves separation unchanged.
        if(0.0f == quadraticA) {
            if(VECSquareMag(&edgeDelta) > movementMagSquared) {
                candidateTime = (VECDotProduct(startPosition, &edgeDelta) -
                                 VECDotProduct(edgeStart, &edgeDelta)) /
                                VECSquareMag(&edgeDelta);
                if(candidateTime >= 0 && candidateTime <= 1) {
                    VECScale(&edgeDelta, &parallelSeparation, candidateTime);
                    VECAdd(edgeStart, &parallelSeparation, &parallelSeparation);
                    VECSubtract(&parallelSeparation, startPosition, &parallelSeparation);
                    if(VECSquareMag(&parallelSeparation) > radiusSquared) {
                        continue;
                    }
                    *contactTime = minimumTime;
                }
            }
        } else {
            discriminant = (quadraticB*quadraticB)-(quadraticA*quadraticC);
            if(discriminant < 0) {
                if(discriminant < -fabsf(discriminant*0.0001f)) {
                    continue;
                }
                discriminant = 0;
            } else {
                discriminant = sqrt(discriminant);
            }
            if(fabsf(quadraticA) < 0.0001f) {
                continue;
            }
            candidateTime = (-quadraticB-discriminant)/quadraticA;
            if(candidateTime < 0) {
                exitTime = (-quadraticB+discriminant)/quadraticA;
                if(exitTime < 0) {
                    continue;
                }
                if(1 < exitTime) {
                    continue;
                }
                candidateTime = minimumTime;
            } else {
                if(*contactTime < candidateTime) {
                    continue;
                }
                if(minimumTime > candidateTime) {
                    continue;
                }
            }
            VECScale(movement, &contactPosition, candidateTime);
            VECAdd(&contactPosition, startPosition, &contactPosition);
            edgeContactParam = (VECDotProduct(&contactPosition, &edgeDelta) -
                                VECDotProduct(edgeStart, &edgeDelta)) /
                               VECSquareMag(&edgeDelta);
            if(edgeContactParam < -0.001f) {
                continue;
            }
            if(edgeContactParam > 1.001f) {
                continue;
            }
            *contactTime = candidateTime;
            VECScale(&edgeDelta, &edgeProjection, edgeContactParam);
            VECAdd(&edgeProjection, edgeStart, &edgeProjection);
            VECSubtract(&contactPosition, &edgeProjection, separation);
        }
    }
    return *contactTime < bestTime;
}

// Finds an earlier capsule contact with any triangle vertex and returns its separation vector.
// Called by _BodyMeshCol after the edge test fails; checks all three vertices.
static BOOL _ColCapsuleVtxCalc(HuVecF *pos, HuVecF *posDelta, float radius, float startT,
                               float colT, HuVecF *vtxBuf, int *index, float *outT, HuVecF *adjust)
{
    HuVecF *vtx;
    int no;

    float outR;
    float dtMag;
    float t;
    float deltaMag;
    float r2;
    float temp;

    HuVecF dt;
    HuVecF vtxDir;

    r2 = radius*radius;
    deltaMag = VECMag(posDelta);
    *outT = startT;
    if(deltaMag < 0.0001f) {
        return 0;
    }
    for(no=3; no--;) {
        vtx = &vtxBuf[index[no]];
        t = (VECDotProduct(vtx, posDelta)-VECDotProduct(pos, posDelta))/VECSquareMag(posDelta);
        VECScale(posDelta, &dt, t);
        dtMag = VECSquareMag(&dt);
        VECSubtract(pos, vtx, &vtxDir);
        outR = dtMag-(VECSquareMag(&vtxDir)-r2);
        if(outR < 0) {
            if(outR < -fabsf(outR*0.0001f)) {
                continue;
            }
            outR = 0;
        } else {
            outR = sqrtf(outR);
        }
        // Use the projection's magnitude, discarding its sign even for a vertex behind the start.
        dtMag = sqrtf(dtMag);
        temp = dtMag-outR;
        t = temp/deltaMag;
        // If the smaller root is negative, try the larger root instead of a start-overlap contact.
        if(t < 0) {
            t = dtMag+outR;
            t /= deltaMag;
        }
        if(colT < t && t < *outT) {
            VECScale(posDelta, adjust, t);
            VECAdd(adjust, pos, adjust);
            VECSubtract(adjust, vtx, adjust);
            *outT = t;
        }
    }
    return *outT != startT;
}

// Returns the horizontal entry time for two moving cylinder shapes.
// Used by _ColCylTest and _ColCapsuleTest with Y components cleared; negligible relative movement
// and negative entry times return -1.
static inline float _ColCylAxisCheck(HuVecF *startA, HuVecF *velA, float radiusA, HuVecF *startB,
                                     HuVecF *velB, float radiusB)
{
    HuVecF deltaStart;
    HuVecF deltaVel;

    float startVelDot;
    float r;
    float colT;
    float ret;

    float velMag2;
    float maxR;
    float startMag2;

    VECSubtract(startA, startB, &deltaStart);
    VECSubtract(velA, velB, &deltaVel);
    r = radiusA+radiusB;
    r = r*r;
    startVelDot = VECDotProduct(&deltaStart, &deltaVel);
    velMag2 = VECDotProduct(&deltaVel, &deltaVel);
    if(velMag2 < 0.001f) {
        return -1;
    }
    ret = -startVelDot/velMag2;
    startMag2 = VECSquareMag(&deltaStart);
    maxR = startMag2+(startVelDot*ret);
    if(r < maxR) {
        return -1;
    }
    colT = (startVelDot*startVelDot)-(velMag2*(startMag2-r));
    if(colT < 0) {
        if(colT < -0.001f) {
            return -1;
        } else {
            colT = 0;
        }
    } else {
        colT = sqrt(colT);
    }
    ret = (-startVelDot-colT)/velMag2;
    if(ret < 0) {
        return -1;
    }
    if(ret > 1) {
        if(ret < 1.001f) {
            ret = 1;
        } else {
            return -1;
        }
    }
    return ret;
}

// Updates each registered mesh transform and preserves its previous transform.
// Called by ColBodyExec; the transforms update only once until ColDirtyClear.
static void UpdateMeshMtx(void)
{
    COLMESH *meshP;
    int no;
    if(colWork.attr & 0x10) {
        return;
    }
    colWork.attr |= 0x10;
    meshP = colMesh;
    for(no=colMeshCount; no--; meshP++) {
        memcpy(meshP->mtxOld, meshP->mtx, sizeof(Mtx));
        memcpy(meshP->mtxInvOld, meshP->mtxInv, sizeof(Mtx));
    }
    ColMtxCalcModelAll();
    meshP = colMesh;
    for(no=colMeshCount; no--; meshP++) {
        // Ignore inversion failure; a singular transform leaves this mesh's previous inverse
        // unchanged.
        MTXInverse(meshP->mtx, meshP->mtxInv);
    }
}

// Clears the current contact point slot before another contact is recorded.
// Used during collision passes to reset the current point slot, not the whole point list.
static inline void ClearColPoint(COLBODY *bodyP)
{
    COLBODY_POINT *colPointP = &bodyP->colPoint[bodyP->colPointNum];

    memset(&colPointP->colOfs, 0, sizeof(colPointP->colOfs));
    memset(&colPointP->normal, 0, sizeof(colPointP->normal));
    colPointP->polyAttr = 0;
    colPointP->meshNo = -1;
    colPointP->obj = NULL;
    colPointP->faceNo = -1;
    bodyP->param.attr &= ~COLBODY_ATTR_MESH_CONTACT_FLAG;
}

// Records mesh contact data in the body's current contact-point slot.
#define InitColPoint(bodyP, bodyNo, norm, hsfFaceP, meshP, face) \
do { \
    COLBODY_POINT *colPointP = &bodyP->colPoint[bodyP->colPointNum]; \
    colPointP->colOfs = meshP->bodyMove[bodyNo]; \
    colPointP->normal = norm; \
    colPointP->polyAttr = ColMatCodeGet(meshP->obj->mesh.material[hsfFaceP->mat & 0xFFF].flags); \
    colPointP->meshNo = meshP->mdlNo; \
    colPointP->obj = meshP->obj; \
    colPointP->faceNo = face; \
} while(0)

static BOOL _BodyApplyColAttr(COLBODY *bodyP, COL_ATTRPARAM *attrParam, int code);

// Applies a contact's surface response and reports whether collision processing should continue.
// Used by mesh collision and response passes; invokes the body's correction hook on success.
static inline BOOL _ColCorrection(COLBODY *bodyP)
{
    COLBODY_POINT *colPointP = &bodyP->colPoint[bodyP->colPointNum];
    COL_ATTRPARAM *attrParam;
    int codeNo;
    BOOL ret;

    codeNo = ColCodeGet(colPointP->polyAttr);
    if(codeNo < 0) {
        OSReport("( colman.c : _ColCorrection ) | マップとあたってるはずなのにアトリビュートがありません\n");
        return FALSE;
    }
    attrParam = (codeNo >= 7) ? (&colWork.attrParamHi[codeNo-7]) : (&colWork.attrParam[codeNo]);
    bodyP->groundAttr |= colPointP->polyAttr;
    ret = _BodyApplyColAttr(bodyP, attrParam, codeNo);
    if(ret && bodyP->param.colCorrectHook) {
        bodyP->param.colCorrectHook(bodyP, bodyP->param.user);
    }
    return ret;
}

typedef struct ColCylinder_s {
    HuVecF startPos; // Body position at the start of the collision pass.
    HuVecF vel; // Movement vector over the frame's collision interval.
    float t; // Movement fraction used to evaluate the current contact position.
    float radius; // Horizontal collision radius in world units.
    float height; // Vertical collision extent in world units.
    HuVecF outPos; // Unused vector storage; the body-pair tests do not access it.
} COLCYLINDER;

// Computes overlap and contact time between two moving cylinder shapes.
// Used by _BodyColCylBroad to classify side, vertical, and existing-overlap contacts.
static float _ColCylTest(COLCYLINDER *a, COLCYLINDER *b, int *result)
{
    HuVecF endVec;
    HuVecF relVel;
    HuVecF endPosA;
    HuVecF endPosB;
    COLLINE lineA;
    COLLINE lineB;
    HuVecF startA;
    HuVecF startB;
    HuVecF velA;
    HuVecF velB;

    float endMag2;
    float colTime;
    float aHalfHeight;
    float bHalfHeight;

    float colLineTime;
    float ret;
    float endRadius;
    float endDist;
    float maxRadius2;

    BOOL lineColResult;
    BOOL endColF;

    VECSubtract(&a->vel, &b->vel, &relVel);
    *result = 4;
    VECScale(&a->vel, &endPosA, a->t);
    VECAdd(&a->startPos, &endPosA, &endPosA);
    VECScale(&b->vel, &endPosB, b->t);
    VECAdd(&b->startPos, &endPosB, &endPosB);
    aHalfHeight = a->height/2;
    bHalfHeight = b->height/2;

    lineA.endA = a->startPos.y+aHalfHeight;
    lineA.startA = a->startPos.y-aHalfHeight;
    lineA.endB = endPosA.y+aHalfHeight;
    lineA.startB = endPosA.y-aHalfHeight;

    lineB.endA = b->startPos.y+bHalfHeight;
    lineB.startA = b->startPos.y-bHalfHeight;
    lineB.endB = endPosB.y+bHalfHeight;
    lineB.startB = endPosB.y-bHalfHeight;
    colLineTime = _GetColLineTime(&lineA, &lineB, relVel.y, &lineColResult);
    if(lineColResult) {
        // Use the starting vertical separation to classify overlap at the current contact
        // positions.
        endDist = (bHalfHeight+aHalfHeight)-fabsf(b->startPos.y-a->startPos.y);
    }
    startA = a->startPos;
    startA.y = 0;
    startB = b->startPos;
    startB.y = 0;
    velA = a->vel;
    velA.y = 0;
    velB = b->vel;
    velB.y = 0;
    endPosA.y = 0;
    endPosB.y = 0;
    VECSubtract(&endPosA, &endPosB, &endVec);
    maxRadius2 = (a->radius+b->radius);
    maxRadius2 = maxRadius2*maxRadius2;
    endMag2 = VECSquareMag(&endVec);
    if(endMag2 < maxRadius2) {
        endRadius = (a->radius+b->radius)-sqrtf(endMag2);
        endColF = TRUE;
        colTime = 0;
    } else {
        colTime = _ColCylAxisCheck(&startA, &velA, a->radius, &startB, &velB, b->radius);
        endColF = FALSE;
    }
    if(colLineTime < 0 || colTime < 0) {
        return -1;
    }
    if(endColF && lineColResult) {
        if(endDist > endRadius) {
            *result = 2;
        } else {
            *result = 3;
        }
    }
    if(colTime >= colLineTime) {
        if(*result == 4) {
            *result = 0;
        }
        ret = colTime;
    } else {
        if(*result == 4) {
            *result = 1;
        }
        ret = colLineTime;
    }
    if(ret < 0 || 1 < ret) {
        *result = 4;
        return -1;
    }
    return ret;
}

// Computes overlap and contact time between two capsule-shaped bodies.
// Used by _BodyColBroad with its radius-shifted vertical bounds.
static float _ColCapsuleTest(COLCYLINDER *a, COLCYLINDER *b, int *result)
{
    HuVecF endVec;
    HuVecF relVel;
    HuVecF endPosA;
    HuVecF endPosB;
    COLLINE lineA;
    COLLINE lineB;
    HuVecF startA;
    HuVecF startB;
    HuVecF velA;
    HuVecF velB;

    float endMag2;
    float colTime;
    float ret;
    float maxRadius2;

    float colLineTime;
    float endRadius;
    float endDist;

    int lineColResult;
    int endColF;

    VECSubtract(&a->vel, &b->vel, &relVel);
    *result = 4;
    VECScale(&a->vel, &endPosA, a->t);
    VECAdd(&a->startPos, &endPosA, &endPosA);
    VECScale(&b->vel, &endPosB, b->t);
    VECAdd(&b->startPos, &endPosB, &endPosB);
    endPosA.y -= a->radius;
    endPosB.y -= b->radius;

    lineA.endA = a->height+(a->startPos.y-a->radius);
    lineA.startA = a->startPos.y-a->radius;
    lineA.endB = endPosA.y+a->height;
    lineA.startB = endPosA.y;

    lineB.endA = b->height+(b->startPos.y-b->radius);
    lineB.startA = b->startPos.y-b->radius;
    lineB.endB = endPosB.y+b->height;
    lineB.startB = endPosB.y;
    colLineTime = _GetColLineTime(&lineA, &lineB, relVel.y, &lineColResult);
    if(lineColResult) {
        // Classify overlap using the radius-shifted lower bounds, without adjusting for
        // different heights.
        endDist = (0.5f*(a->height+b->height))-fabsf(endPosA.y-endPosB.y);
    }
    startA = a->startPos;
    startA.y = 0;
    startB = b->startPos;
    startB.y = 0;
    velA = a->vel;
    velA.y = 0;
    velB = b->vel;
    velB.y = 0;
    endPosA.y = 0;
    endPosB.y = 0;
    VECSubtract(&endPosA, &endPosB, &endVec);
    maxRadius2 = (a->radius+b->radius);
    maxRadius2 = maxRadius2*maxRadius2;
    endMag2 = VECSquareMag(&endVec);
    if(endMag2 < maxRadius2) {
        endRadius = (a->radius+b->radius)-sqrtf(endMag2);
        endColF = 1;
        colTime = 0;
    } else {
        colTime = _ColCylAxisCheck(&startA, &velA, a->radius, &startB, &velB, b->radius);
        endColF = 0;
    }
    if(colLineTime < 0 || colTime < 0) {
        return -1;
    }
    if(endColF && lineColResult) {
        if(endDist > endRadius) {
            *result = 2;
        } else {
            *result = 3;
        }
    }
    if(colTime >= colLineTime) {
        if(*result == 4) {
            *result = 0;
        }
        ret = colTime;
    } else {
        if(*result == 4) {
            *result = 1;
        }
        ret = colLineTime;
    }
    if(ret < 0 || 1 < ret) {
        *result = 4;
        return -1;
    }
    return ret;
}

// Rejects points outside the triangle's centroid bound plus the supplied distance allowance.
// Used by both mesh collision passes before testing a triangle.
static inline BOOL CheckFacePoint(COLTRI *triP, HuVecF *p, float maxDist)
{
    HuVecF pCenter;
    VECSubtract(&triP->center, p, &pCenter);
    if(VECSquareMag(&pCenter) > triP->dist+maxDist) {
        return FALSE;
    } else {
        return TRUE;
    }
}

// Builds the cross-product normal from two triangle edges.
// Unused helper; no collision pass calls it.
static inline void CalcCross(HuVecF *a, HuVecF *b, HuVecF *c, HuVecF *out)
{
    HuVecF ba;
    HuVecF cb;
    VECSubtract(b, a, &ba);
    VECSubtract(c, b, &cb);
    VECCrossProduct(&ba, &cb, out);
    VECNormalize(out, out);
}

// Transforms a mesh triangle's local normal into world space.
// Used by mesh contact checks; transforms vertices before recomputing the normal.
static inline void CalcMeshNorm(COLMESH *meshP, HuVecF a, HuVecF b, HuVecF c, HuVecF *out)
{
    HuVecF ba;
    HuVecF cb;

    MTXMultVec(meshP->mtx, &a, &a);
    MTXMultVec(meshP->mtx, &b, &b);
    MTXMultVec(meshP->mtx, &c, &c);
    VECSubtract(&b, &a, &ba);
    VECSubtract(&c, &b, &cb);
    VECCrossProduct(&ba, &cb, out);
    VECNormalize(out, out);
}

// Corrects an initial overlap using the world face normal scaled by the mesh's basis-row lengths.
// Used by _BodyMeshCol only while the body's current contact time is zero.
static inline BOOL CheckPoint(COLBODY *bodyP, COLMESH *meshP, float planeOffset, int *index,
                              HuVecF *vtxBuf, HuVecF *out)
{
    int posNo;
    Mtx scaleMtx;
    HuVecF temp;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    if(bodyP->colT != 0) {
        return FALSE;
    }
    MakeScaleMtx(scaleMtx, meshP->mtx);
    CalcMeshNorm(meshP, vtxBuf[index[0]], vtxBuf[index[1]], vtxBuf[index[2]], out);
    VECScale(out, &temp, planeOffset-0.0001f);
    MTXMultVec(scaleMtx, &temp, &temp);
    VECSubtract(&bodyP->oldPos, &temp, &bodyP->pos[posNo]);
    bodyP->oldPos = bodyP->pos[posNo];
    bodyP->oldColT = 1;
    bodyP->param.attr &= ~COLBODY_ATTR_SWEEP_CONTACT_FLAG;
    return TRUE;
}

// Solves the smaller line/sphere root for mesh bounds rejection; the caller checks its range.
// The quadratic subtracts radius directly, not radius squared. Zero-length movement still divides
// by zero and returns TRUE.
static inline BOOL ColSphereLineCheck(HuVecF *start, HuVecF *end, HuVecF *center, float radius,
                                      float *t)
{
    HuVecF startDelta;
    HuVecF lineDelta;
    float lineMag;
    float lineDot;
    float startMag;
    float det;

    VECSubtract(start, center, &startDelta);
    VECSubtract(end, start, &lineDelta);
    lineMag = VECSquareMag(&lineDelta);
    lineDot = 2.0f * VECDotProduct(&startDelta, &lineDelta);
    startMag = VECSquareMag(&startDelta) - radius;
    det = (lineDot * lineDot) - (4.0f * (lineMag * startMag));
    if(det < 0.0f) {
        return FALSE;
    }
    *t = (-lineDot - sqrtf(det)) / (2.0f * lineMag);
    return TRUE;
}

#define MESHCOL_ATTR (COLBODY_ATTR_NO_CONTACT_FLAG|COLBODY_ATTR_MESHCOL_OFF|COLBODY_ATTR_COL_OFF)

// Resolves active bodies against triangle meshes using their configured shapes.
// Called by ColBodyExec when cylinder mode is clear.
static void _BodyMeshCol(void)
{
    COLBODY *bodyP;
    COLMESH *meshP;
    HSF_FACE *hsfFaceP;
    HuVecF *vtxBuf;
    COLTRI *triP;
    COLBODY *body2;
    int triNo;
    BOOL doneF;
    int colResult;
    int j;
    int posNo;

    float radius;
    float maxDist;
    float movementMagSquared;

    HuVecF pos;
    HuVecF posNew;
    HuVecF posDelta;
    HuVecF contactNormal;
    HuVecF boundsDelta;
    int index[3];
    HuVecF radiusVec;
    HuVecF projectedPoint;
    HuVecF planeCornerOffset;
    HuVecF bodySupportOffset;
    HuVecF worldContactPosition;
    HuVecF posAdjust;
    HuVecF norm;
    Mtx invNoTrans;

    int no;
    float boundsT[1];
    int i;
    float colT;
    int triNum;
    float capsuleColT;

    meshP = colMesh;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    for(no=colMeshCount; no--; meshP++) {
        RemoveMtxTrans(invNoTrans, meshP->mtxInv);
        for(bodyP=colWork.body, i=0; i<colWork.bodyNum; i++, bodyP++) {
            body2 = bodyP;
            if ((bodyP->param.attr & COLBODY_ATTR_ACTIVE) &&
                (bodyP->param.attr & MESHCOL_ATTR) == 0) {
                if(!(body2->param.mask & meshP->mask)) {
                    continue;
                } else {
                    doneF = TRUE;
                    while(doneF) {
                        doneF = FALSE;
                        pos = bodyP->oldPos;
                        VECAdd(&pos, &bodyP->moveDir, &posNew);
                        MTXMultVec(meshP->mtxInvOld, &pos, &pos);
                        MTXMultVec(meshP->mtxInv, &posNew, &posNew);
                        VECSubtract(&posNew, &pos, &posDelta);
                        movementMagSquared = VECSquareMag(&posDelta);
                        VECSubtract(&pos, &meshP->boundsCenter, &boundsDelta);
                        if(VECSquareMag(&boundsDelta) > meshP->boundsRadius * meshP->boundsRadius) {
                            if (!ColSphereLineCheck(&pos, &posNew, &meshP->boundsCenter,
                                                    meshP->boundsRadius, boundsT) ||
                                boundsT[0] < 0.0f || 1.0f < boundsT[0]) {
                                break;
                            }
                        }
                        vtxBuf = meshP->obj->mesh.vertex->data;
                        hsfFaceP = meshP->obj->mesh.face->data;
                        triP = meshP->tri;
                        for(j=0; j<meshP->obj->mesh.face->count; j++, hsfFaceP++) {
                            if(doneF) {
                                break;
                            }
                            switch(hsfFaceP->type) {
                                case HSF_FACE_QUAD:
                                    triNum = 2;
                                    break;

                                case HSF_FACE_TRI:
                                    triNum = 1;
                                    break;

                                case HSF_FACE_TRISTRIP:
                                    triNum = hsfFaceP->strip.count+1;
                                    break;
                            }
                            for(triNo=0; triNo<triNum; triNo++, triP++) {
                                if(triP->norm.x || triP->norm.y || triP->norm.z) {
                                    MakeIndexBuf(index, hsfFaceP, triNo);
                                    colT = bodyP->oldColT;
                                    VECScale(&triP->norm, &radiusVec, body2->param.radius);
                                    MTXMultVec(invNoTrans, &radiusVec, &radiusVec);
                                    radius = VECSquareMag(&radiusVec);
                                    // Add half-height directly to the squared-radius and
                                    // squared-movement allowance.
                                    maxDist = radius+(body2->param.height/2);
                                    maxDist = maxDist+movementMagSquared;
                                    if(CheckFacePoint(triP, &pos, maxDist)) {
                                        radius = sqrtf(radius);
                                        colResult = ColPlaneDistGet(&pos, &posNew, &posDelta,
                                                                    radius, colT, bodyP->colT,
                                                                    vtxBuf, index, triP, &colT);
                                        if(colResult == -6) {
                                            MakeMeshEject(body2, triP, &bodySupportOffset, &norm);
                                            if (!ColPlaneCheck(&pos, &bodySupportOffset, triP,
                                                               &planeCornerOffset, FALSE)) {
                                                colResult = -1;
                                                if (ColPlaneCheck(&pos, &bodySupportOffset, triP,
                                                                  &planeCornerOffset, TRUE)) {
                                                    VECAdd(&pos, &planeCornerOffset,
                                                           &projectedPoint);
                                                    if (ColPlaneEdgeCheck(
                                                            projectedPoint, vtxBuf[index[0]],
                                                            vtxBuf[index[1]], triP, FALSE)) {
                                                        colResult = -6;
                                                    }
                                                }
                                            } else {
                                                colResult = -4;
                                            }
                                        }
                                        switch(colResult) {
                                            case -4:
                                            case -6:
                                                if(VECSquareMag(&posDelta) < 0.0001f) {
                                                    break;
                                                } else {
                                                    COLBODY_POINT *colPointP;
                                                    int no;
                                                    BOOL ret;

                                                    ret = FALSE;
                                                    for(no=bodyP->colPointNum; no--;) {
                                                        colPointP = &bodyP->colPoint[no];
                                                        if (colPointP->obj == meshP->obj &&
                                                            colPointP->faceNo == j) {
                                                            ret = TRUE;
                                                            break;
                                                        }
                                                    }
                                                    if(!ret) {
                                                        posAdjust.x = 0.0f;
                                                        posAdjust.y = 0.0f;
                                                        posAdjust.z = 0.0f;
                                                        if (!_ColCapsuleEdgeCalc(
                                                                &pos, &posNew, &posDelta, radius,
                                                                bodyP->oldColT, bodyP->colT, vtxBuf,
                                                                index, &capsuleColT, &posAdjust) &&
                                                            !_ColCapsuleVtxCalc(
                                                                &pos, &posDelta, radius,
                                                                bodyP->oldColT, bodyP->colT, vtxBuf,
                                                                index, &capsuleColT, &posAdjust)) {
                                                            if(colResult == -6) {
                                                                if (CheckPoint(bodyP, meshP, colT,
                                                                               index, vtxBuf,
                                                                               &contactNormal)) {
                                                                    VECSubtract(&bodyP->oldPos,
                                                                                &meshP->bodyMove[i],
                                                                                &bodyP->oldPos);
                                                                    VECSubtract(&bodyP->pos[posNo],
                                                                                &meshP->bodyMove[i],
                                                                                &bodyP->pos[posNo]);
                                                                    doneF = TRUE;
                                                                    goto colPoint;
                                                                }
                                                            }
                                                        } else {
                                                            VECScale(&bodyP->moveDir,
                                                                     &worldContactPosition,
                                                                     capsuleColT);
                                                            VECAdd(&worldContactPosition,
                                                                   &bodyP->oldPos,
                                                                   &worldContactPosition);
                                                            bodyP->pos[posNo] =
                                                                worldContactPosition;
                                                            // The normalized edge/vertex
                                                            // separation remains mesh-local;
                                                            // only the fallback below recomputes
                                                            // a world-space normal.
                                                            if(VECSquareMag(&posAdjust) > 0.001f) {
                                                                VECNormalize(&posAdjust,
                                                                             &contactNormal);
                                                            } else {
                                                                CalcMeshNorm(meshP,
                                                                             vtxBuf[index[0]],
                                                                             vtxBuf[index[1]],
                                                                             vtxBuf[index[2]],
                                                                             &contactNormal);
                                                            }
                                                            bodyP->oldColT = capsuleColT;
                                                            if(bodyP->oldColT < 0) {
                                                                bodyP->oldColT = bodyP->colT;
                                                            }
                                                            bodyP->param.attr |=
                                                                COLBODY_ATTR_SWEEP_CONTACT_FLAG;
                                                            goto colPoint;
                                                        }
                                                    }
                                                }
                                                break;

                                            case 0:
                                                CalcMeshNorm(meshP, vtxBuf[index[0]],
                                                             vtxBuf[index[1]], vtxBuf[index[2]],
                                                             &contactNormal);
                                                VECScale(&bodyP->moveDir, &bodyP->pos[posNo], colT);
                                                VECAdd(&bodyP->pos[posNo], &bodyP->oldPos,
                                                       &bodyP->pos[posNo]);
                                                bodyP->oldColT = colT;
                                                bodyP->param.attr &=
                                                    ~COLBODY_ATTR_SWEEP_CONTACT_FLAG;
                                                goto colPoint;

                                            case -5:
                                                if (CheckPoint(bodyP, meshP, colT, index, vtxBuf,
                                                               &contactNormal)) {
                                                    VECSubtract(&bodyP->oldPos, &meshP->bodyMove[i],
                                                                &bodyP->oldPos);
                                                    VECSubtract(&bodyP->pos[posNo],
                                                                &meshP->bodyMove[i],
                                                                &bodyP->pos[posNo]);
                                                    doneF = TRUE;
                                                    default:
                                                    colPoint:
                                                        InitColPoint(bodyP, i, contactNormal,
                                                                     hsfFaceP, meshP, j);
                                                        bodyP->param.attr |=
                                                            COLBODY_ATTR_MESH_CONTACT_FLAG;
                                                        if (doneF) {
                                                            bodyP->param.attr |=
                                                                COLBODY_ATTR_CONTACT_CORRECTED_FLAG;
                                                            bodyP->param.attr &=
                                                                ~COLBODY_ATTR_MESH_CONTACT_MASK;
                                                            // Ignore the correction result; the
                                                            // overlap adjustment still triggers
                                                            // another mesh pass.
                                                            _ColCorrection(bodyP);
                                                            goto skipTri;
                                                    }

                                                    bodyP->param.attr &=
                                                        ~COLBODY_ATTR_CONTACT_CORRECTED_FLAG;
                                                    }
                                                break;

                                            case -1:
                                            case -2:
                                            case -3:
                                                break;

                                        }
                                    }
                                }
                            }
                            skipTri:
                            (void)bodyP;
                        }
                    }
                }
            }
        }
    }
}

// Builds a cylinder support offset from a triangle normal, radius, and height for _BodyMeshCylCol.
// Y is zero when abs(normal.y) < 0.001, otherwise the signed half-height.
static inline void MakeCylEject(COLBODY *bodyP, COLTRI *triP, HuVecF *out, HuVecF *norm)
{
    float halfHeight;
    float radius;

    halfHeight = bodyP->param.height;
    radius = bodyP->param.radius;
    *norm = triP->norm;
    out->x = norm->x;
    out->y = 0;
    out->z = norm->z;
    if(fabsf(out->x) > 0.001f || fabsf(out->z) > 0.001f) {
        VECNormalize(out, out);
    }
    halfHeight /= 2;
    out->x *= radius;
    if(fabsf(norm->y) < 0.001f) {
        out->y = 0;
    } else {
        out->y = norm->y > 0 ? halfHeight : -halfHeight;
    }
    out->z *= radius;
}

// Resolves active bodies against triangle meshes using vertical cylinder tests.
// Called by ColBodyExec when cylinder mode is set.
static void _BodyMeshCylCol(void)
{
    COLBODY *bodyP;
    HSF_FACE *hsfFaceP;
    COLMESH *meshP;
    COLTRI *triP;
    HuVecF *vtxBuf;
    COLBODY *body2;
    int i;
    int posNo;
    BOOL doneF;
    int j;

    float maxDist;
    float ejectDot;
    float deltaMag2;

    HuVecF pos;
    HuVecF posNew;
    HuVecF posDelta;
    HuVecF meshNorm;
    HuVecF boundsDelta;
    int index[3];
    HuVecF ejectDir;
    HuVecF planeA;
    HuVecF axisVec;
    HuVecF finalPos;
    HuVecF posAdjust;
    HuVecF norm;

    int no;
    float boundsT[1];
    int colResult;
    float colT;
    int triNo;
    int triNum;
    int capsuleColResult;
    float axisLen;

    meshP = colMesh;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    for(no=colMeshCount; no--; meshP++) {
        for(bodyP=colWork.body, i=0; i<colWork.bodyNum; i++, bodyP++) {
            body2 = bodyP;
            if ((bodyP->param.attr & COLBODY_ATTR_ACTIVE) &&
                (bodyP->param.attr & MESHCOL_ATTR) == 0) {
                if(!(body2->param.mask & meshP->mask)) {
                    continue;
                } else {
                    doneF = TRUE;
                    while(doneF) {
                        doneF = FALSE;
                        pos = bodyP->oldPos;
                        VECAdd(&pos, &bodyP->moveDir, &posNew);
                        MTXMultVec(meshP->mtxInvOld, &pos, &pos);
                        MTXMultVec(meshP->mtxInv, &posNew, &posNew);
                        VECSubtract(&posNew, &pos, &posDelta);
                        deltaMag2 = VECSquareMag(&posDelta);
                        VECSubtract(&pos, &meshP->boundsCenter, &boundsDelta);
                        if(VECSquareMag(&boundsDelta) > meshP->boundsRadius * meshP->boundsRadius) {
                            if (!ColSphereLineCheck(&pos, &posNew, &meshP->boundsCenter,
                                                    meshP->boundsRadius, boundsT) ||
                                boundsT[0] < 0.0f || 1.0f < boundsT[0]) {
                                break;
                            }
                        }
                        vtxBuf = meshP->obj->mesh.vertex->data;
                        hsfFaceP = meshP->obj->mesh.face->data;
                        triP = meshP->tri;
                        for(j=0; j<meshP->obj->mesh.face->count; j++, hsfFaceP++) {
                            if(doneF) {
                                break;
                            }
                            switch(hsfFaceP->type) {
                                case HSF_FACE_QUAD:
                                    triNum = 2;
                                    break;

                                case HSF_FACE_TRI:
                                    triNum = 1;
                                    break;

                                case HSF_FACE_TRISTRIP:
                                    triNum = hsfFaceP->strip.count+1;
                                    break;
                            }
                            for(triNo=0; triNo<triNum; triNo++, triP++) {
                                if(triP->norm.x || triP->norm.y || triP->norm.z) {
                                    MakeIndexBuf(index, hsfFaceP, triNo);
                                    colT = bodyP->oldColT;
                                    MakeCylEject(body2, triP, &ejectDir, &norm);
                                    ejectDot = VECDotProduct(&triP->norm, &ejectDir);
                                    maxDist = ejectDot+(body2->param.height/2);
                                    maxDist = deltaMag2+(maxDist*maxDist);
                                    if(CheckFacePoint(triP, &pos, maxDist)) {
                                        colResult = ColEjectDistGet(
                                            &pos, &posNew, &posDelta, &ejectDir, ejectDot, colT,
                                            bodyP->colT, vtxBuf, index, triP, &colT);
                                        switch(colResult) {
                                            case -6:
                                                if (ColPlaneYCheck(&pos, &ejectDir, triP, &axisVec,
                                                                   FALSE)) {
                                                    // Ignore the corner-test result; on failure,
                                                    // keep the offset selected by ColPlaneYCheck.
                                                    ColPlaneCheck(&pos, &ejectDir, triP, &axisVec,
                                                                  FALSE);
                                                    VECAdd(&pos, &axisVec, &planeA);
                                                    axisLen = triP->d +
                                                              VECDotProduct(&triP->norm, &planeA);
                                                    VECScale(&triP->norm, &axisVec, axisLen);
                                                    VECSubtract(&planeA, &axisVec, &planeA);
                                                    if (ColPlaneEdgeCheck(planeA, vtxBuf[index[0]],
                                                                          vtxBuf[index[1]], triP,
                                                                          FALSE)) {
                                                    case -4: {
                                                        COLBODY_POINT *colPointP;
                                                        int no;
                                                        BOOL ret;

                                                        ret = FALSE;
                                                        for (no = bodyP->colPointNum; no--;) {
                                                            colPointP = &bodyP->colPoint[no];
                                                            if (colPointP->obj == meshP->obj &&
                                                                colPointP->faceNo == j) {
                                                                ret = TRUE;
                                                                break;
                                                            }
                                                        }
                                                        if (ret ||
                                                            VECSquareMag(&posDelta) < 0.0001f) {
                                                            break;
                                                        }
                                                        capsuleColResult = _BodyTriCylCol(
                                                            &pos, &posDelta, body2, vtxBuf, index,
                                                            &colT, NULL);
                                                        if (!(capsuleColResult < 0 ||
                                                              colT < bodyP->colT ||
                                                              bodyP->oldColT < colT)) {
                                                            VECScale(&bodyP->moveDir, &finalPos,
                                                                     colT);
                                                            VECAdd(&finalPos, &bodyP->oldPos,
                                                                   &finalPos);
                                                            bodyP->pos[posNo] = finalPos;
                                                            CalcMeshNorm(meshP, vtxBuf[index[0]],
                                                                         vtxBuf[index[1]],
                                                                         vtxBuf[index[2]],
                                                                         &meshNorm);
                                                            bodyP->oldColT = colT;
                                                            bodyP->param.attr |=
                                                                COLBODY_ATTR_SWEEP_CONTACT_FLAG;
                                                            goto colPoint;
                                                        }
                                                        }
                                                        }
                                                }
                                                break;

                                            case 0:
                                                CalcMeshNorm(meshP, vtxBuf[index[0]],
                                                             vtxBuf[index[1]], vtxBuf[index[2]],
                                                             &meshNorm);
                                                VECScale(&bodyP->moveDir, &bodyP->pos[posNo], colT);
                                                VECAdd(&bodyP->pos[posNo], &bodyP->oldPos,
                                                       &bodyP->pos[posNo]);
                                                bodyP->oldColT = colT;
                                                bodyP->param.attr &=
                                                    ~COLBODY_ATTR_SWEEP_CONTACT_FLAG;
                                                goto colPoint;

                                            case -5:
                                                if(bodyP->colT == 0.0f) {
                                                    CalcMeshNorm(meshP, vtxBuf[index[0]],
                                                                 vtxBuf[index[1]], vtxBuf[index[2]],
                                                                 &meshNorm);
                                                    VECScale(&meshNorm, &posAdjust, colT-0.0001f);
                                                    VECSubtract(&bodyP->oldPos, &posAdjust,
                                                                &bodyP->pos[posNo]);
                                                    bodyP->oldPos = bodyP->pos[posNo];
                                                    bodyP->oldColT = 1;
                                                    doneF = TRUE;
                                                    bodyP->param.attr &=
                                                        ~COLBODY_ATTR_SWEEP_CONTACT_FLAG;
                                                    default:
                                                    colPoint:
                                                        InitColPoint(bodyP, i, meshNorm, hsfFaceP,
                                                                     meshP, j);
                                                        bodyP->param.attr |=
                                                            COLBODY_ATTR_MESH_CONTACT_FLAG;
                                                        if (doneF) {
                                                            bodyP->param.attr |=
                                                                COLBODY_ATTR_CONTACT_CORRECTED_FLAG;
                                                            bodyP->param.attr &=
                                                                ~COLBODY_ATTR_MESH_CONTACT_MASK;
                                                            // Ignore the correction result; the
                                                            // overlap adjustment still triggers
                                                            // another mesh pass.
                                                            _ColCorrection(bodyP);
                                                            goto skipTri;
                                                    }
                                                    bodyP->param.attr &=
                                                        ~COLBODY_ATTR_CONTACT_CORRECTED_FLAG;
                                                }
                                                break;

                                            case -1:
                                            case -2:
                                            case -3:
                                                break;
                                        }
                                    }
                                }
                            }
                            skipTri:
                            (void)bodyP;
                        }
                    }
                }
            }
        }
    }
}

#undef MESHCOL_ATTR

#define BODYCOL_ATTR (COLBODY_ATTR_BODYCOL_OFF|COLBODY_ATTR_COL_OFF)

// Builds broad-phase candidate pairs for active vertical cylinder bodies.
// Called by ColBodyExec in cylinder mode; may discard a later mesh contact for another pass.
static int _BodyColCylBroad(void)
{
    COLBODY *bodyP;
    COLBODY *body2;
    COLBROAD *broadP;
    int i;
    int j;
    BOOL resetPoint;
    BOOL ret;
    u8 clearPoint1;
    u8 clearPoint2;

    float t;

    COLCYLINDER cylA;
    COLCYLINDER cylB;
    int colResult;

    int posIdx;

    ret = FALSE;
    colWork.broadColNum = 0;
    posIdx = (colWork.attr & 0x4) ? 1 : 0;

    for(bodyP=colWork.body, i=0; i<colWork.bodyNum; i++, bodyP++) {
        if((bodyP->param.attr & COLBODY_ATTR_ACTIVE) && (bodyP->param.attr & BODYCOL_ATTR) == 0) {
            cylA.startPos = bodyP->oldPos;
            cylA.vel = bodyP->moveDir;
            cylA.t = bodyP->colT;
            cylA.radius = bodyP->param.radius;
            cylA.height = bodyP->param.height;
            for(body2=bodyP+1, j=i+1; j<colWork.bodyNum; j++, body2++) {
                if ((body2->param.attr & COLBODY_ATTR_ACTIVE) &&
                    (body2->param.attr & BODYCOL_ATTR) == 0 &&
                    (bodyP->param.mask & body2->param.mask)) {
                    if (!(bodyP->colBit[j >> 5] & (1 << (j & 0x1F))) ||
                        !(body2->colBit[i >> 5] & (1 << (i & 0x1F)))) {
                        cylB.startPos = body2->oldPos;
                        cylB.vel = body2->moveDir;
                        cylB.t = body2->colT;
                        cylB.radius = body2->param.radius;
                        cylB.height = body2->param.height;
                        t = _ColCylTest(&cylA, &cylB, &colResult);
                        if(colResult != 4) {
                            if(colResult == 2 || colResult == 3) {
                                if(bodyP->colT > body2->colT) {
                                    t = bodyP->colT;
                                } else {
                                    t = body2->colT;
                                }
                            } else {
                                if(t < bodyP->colT && (bodyP->colT-t) >0.001f) {
                                    continue;
                                }
                                if(t < body2->colT && (body2->colT-t) >0.001f) {
                                    continue;
                                }
                                resetPoint = FALSE;
                                clearPoint1 = FALSE;
                                clearPoint2 = FALSE;
                                if(t > bodyP->oldColT && (t-bodyP->oldColT) > 0.001f) {
                                    resetPoint = TRUE;
                                } else {
                                    clearPoint1 = TRUE;
                                }
                                if(t > body2->oldColT && (t-body2->oldColT) > 0.001f) {
                                    resetPoint = TRUE;
                                } else {
                                    clearPoint2 = TRUE;
                                }
                                if(resetPoint) {
                                    if(clearPoint1) {
                                        ClearColPoint(bodyP);
                                    }
                                    if(clearPoint2) {
                                        ClearColPoint(body2);
                                    }
                                    ret = TRUE;
                                    continue;
                                }
                            }
                            broadP = &colWork.broadCol[colWork.broadColNum];
                            broadP->t = t;
                            broadP->body1 = bodyP;
                            broadP->body2 = body2;
                            broadP->colResult = colResult;
                            broadP->body1Idx = i;
                            broadP->body2Idx = j;

                            colWork.broadColNum++;
                    bodyP->param.attr |= COLBODY_ATTR_BODY_CONTACT_FLAG;
                    body2->param.attr |= COLBODY_ATTR_BODY_CONTACT_FLAG;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

// Builds broad-phase candidate pairs for active collision bodies.
// Called by ColBodyExec outside cylinder mode; may request another collision pass.
static int _BodyColBroad(void)
{
    COLBODY *bodyP;
    COLBODY *body2;
    COLBROAD *broadP;
    int i;
    int j;
    BOOL resetPoint;
    BOOL ret;
    u8 clearPoint1;
    u8 clearPoint2;

    float t;

    COLCYLINDER cylA;
    COLCYLINDER cylB;
    int colResult;

    int posIdx;

    ret = FALSE;
    colWork.broadColNum = 0;
    posIdx = (colWork.attr & 0x4) ? 1 : 0;

    for(bodyP=colWork.body, i=0; i<colWork.bodyNum; i++, bodyP++) {
        if((bodyP->param.attr & COLBODY_ATTR_ACTIVE) && (bodyP->param.attr & BODYCOL_ATTR) == 0) {
            cylA.startPos = bodyP->oldPos;
            cylA.vel = bodyP->moveDir;
            cylA.t = bodyP->colT;
            cylA.radius = bodyP->param.radius;
            cylA.height = bodyP->param.height;
            for(body2=bodyP+1, j=i+1; j<colWork.bodyNum; j++, body2++) {
                if ((body2->param.attr & COLBODY_ATTR_ACTIVE) &&
                    (body2->param.attr & BODYCOL_ATTR) == 0 &&
                    (bodyP->param.mask & body2->param.mask)) {
                    if (!(bodyP->colBit[j >> 5] & (1 << (j & 0x1F))) ||
                        !(body2->colBit[i >> 5] & (1 << (i & 0x1F)))) {
                        cylB.startPos = body2->oldPos;
                        cylB.vel = body2->moveDir;
                        cylB.t = body2->colT;
                        cylB.radius = body2->param.radius;
                        cylB.height = body2->param.height;
                        t = _ColCapsuleTest(&cylA, &cylB, &colResult);
                        if(colResult != 4) {
                            if(colResult == 2 || colResult == 3) {
                                if(bodyP->colT > body2->colT) {
                                    t = bodyP->colT;
                                } else {
                                    t = body2->colT;
                                }
                            } else {
                                if(t < bodyP->colT && (bodyP->colT-t) >0.001f) {
                                    continue;
                                }
                                if(t < body2->colT && (body2->colT-t) >0.001f) {
                                    continue;
                                }
                                resetPoint = FALSE;
                                clearPoint1 = FALSE;
                                clearPoint2 = FALSE;
                                if(t > bodyP->oldColT && (t-bodyP->oldColT) >0.001f) {
                                    resetPoint = TRUE;
                                } else {
                                    clearPoint1 = TRUE;
                                }
                                if(t > body2->oldColT && (t-body2->oldColT) >0.001f) {
                                    resetPoint = TRUE;
                                } else {
                                    clearPoint2 = TRUE;
                                }
                                if(resetPoint) {
                                    if(clearPoint1) {
                                        ClearColPoint(bodyP);
                                    }
                                    if(clearPoint2) {
                                        ClearColPoint(body2);
                                    }
                                    ret = TRUE;
                                    continue;
                                }
                            }
                            broadP = &colWork.broadCol[colWork.broadColNum];
                            broadP->t = t;
                            broadP->body1 = bodyP;
                            broadP->body2 = body2;
                            broadP->colResult = colResult;
                            broadP->body1Idx = i;
                            broadP->body2Idx = j;

                            colWork.broadColNum++;
                    bodyP->param.attr |= COLBODY_ATTR_BODY_CONTACT_FLAG;
                    body2->param.attr |= COLBODY_ATTR_BODY_CONTACT_FLAG;
                        }
                    }
                }
            }
        }
    }
    return ret;
}

#undef BODYCOL_ATTR

// Resolves candidate body pairs in order of their chosen movement fraction.
// Called by ColBodyExec after collection; each body's response requires a nonzero combined hook
// result.
static int _BodyColNarrow(void)
{
    COLBODY *body1;
    COLBODY *body2;
    COLBROAD *broadP;
    int colResult;
    int i;
    int cbResult1;
    int cbResult2;
    COLNARROW *narrowP;
    COLNARROW *narrow;
    int ret;
    int posNo;

    float h1;
    float h2;
    float r1;
    float r2;
    float dot;

    COL_NARROW_PARAM col1;
    COL_NARROW_PARAM col2;
    HuVecF fixDir;
    HuVecF pointOfs;
    HuVecF colDeltaNorm;
    HuVecF colDelta;
    HuVecF colXZDelta;
    HuVecF originalMove1;
    HuVecF originalMove2;
    HuVecF newMove1;
    HuVecF newMove2;

    narrowP = colWork.narrowCol;
    if(colWork.broadColNum == 0) {
        return FALSE;
    }
    ret = FALSE;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    for(broadP=colWork.broadCol, i=0; i<colWork.broadColNum; i++, broadP++) {
        narrowP[i].t = broadP->t;
        narrowP[i].broadP = broadP;
    }
    SortCollisions(narrowP, colWork.broadColNum);
    for(narrow=narrowP, i=0; i<colWork.broadColNum; i++, narrow++) {
        broadP = narrow->broadP;
        body1 = broadP->body1;
        body2 = broadP->body2;
                if(!(body1->param.attr & COLBODY_ATTR_BODY_PAIR_CONTACT_FLAG) &&
                   !(body2->param.attr & COLBODY_ATTR_BODY_PAIR_CONTACT_FLAG)) {
            body1->colBit[broadP->body2Idx >> 5] |= (1 << (broadP->body2Idx & 0x1F));
            body2->colBit[broadP->body1Idx >> 5] |= (1 << (broadP->body1Idx & 0x1F));
            r1 = broadP->body1->param.radius;
            r2 = broadP->body2->param.radius;
            h1 = broadP->body1->param.height/2;
            h2 = broadP->body2->param.height/2;
            originalMove1 = body1->moveDir;
            col1.paramA = body1->param.paramA;
            col1.paramB = body1->param.paramB;
            col1.type = body1->param.type;
            col1.normPos = originalMove1;
            col1.colResult = broadP->colResult;
            VECScale(&body1->moveDir, &col1.point, broadP->t);
            VECAdd(&col1.point, &body1->oldPos, &col1.point);
            originalMove2 = body2->moveDir;
            col2.paramA = body2->param.paramA;
            col2.paramB = body2->param.paramB;
            col2.type = body2->param.type;
            col2.normPos = originalMove2;
            col2.colResult = broadP->colResult;

            VECScale(&body2->moveDir, &col2.point, broadP->t);
            VECAdd(&col2.point, &body2->oldPos, &col2.point);
            if(!(colWork.attr & 0x20)) {
                col1.point.y = (body1->param.height*0.5f)+(col1.point.y-body1->param.radius);
                col2.point.y = (body2->param.height*0.5f)+(col2.point.y-body2->param.radius);
            }
            cbResult1 = 0;
            if(body1->param.narrowHook) {
                cbResult1 = body1->param.narrowHook(&col1, &col2);
            }
            if(body1->param.narrowHook2) {
                cbResult1 |= body1->param.narrowHook2(&col1, &col2);
            }
            newMove1 = col1.normPos;
            // Keep body1's revised movement separately and restore both movement inputs before
            // body2's hooks.
            col1.normPos = originalMove1;
            col2.normPos = originalMove2;
            cbResult2 = 0;
            if(body2->param.narrowHook) {
                cbResult2 = body2->param.narrowHook(&col2, &col1);
            }
            if(body2->param.narrowHook2) {
                cbResult2 |= body2->param.narrowHook2(&col2, &col1);
            }
            newMove2 = col2.normPos;
            col1.normPos = originalMove1;
            col2.normPos = originalMove2;
            // At the point limit, discard each body's hook result and stop its movement at the
            // contact.
            // Only body2 sets the contact-point-limit flag here.
            if(body1->colPointNum >= 8) {
                body1->moveDir.x = 0;
                body1->moveDir.y = 0;
                body1->moveDir.z = 0;
                body1->oldPos = col1.point;
                cbResult1 = 0;
            }
            if(body2->colPointNum >= 8) {
                    body2->param.attr |= COLBODY_ATTR_CONTACT_POINT_LIMIT_FLAG;
                body2->moveDir.x = 0;
                body2->moveDir.y = 0;
                body2->moveDir.z = 0;
                body2->oldPos = col2.point;
                cbResult2 = 0;
            }
            VECSubtract(&col2.point, &col1.point, &colDelta);
            // For nearly coincident contact points, choose random X/Z separation and force
            // Y to 0.01.
            if(VECSquareMag(&colDelta) < 0.0001f) {
                colDelta.x = ((u32)frandmod(20)-10.0f)*0.01f;
                colDelta.z = ((u32)frandmod(20)-10.0f)*0.01f;
                colDelta.y = 0.01f;
            }
            VECNormalize(&colDelta, &colDeltaNorm);
            colXZDelta = colDeltaNorm;
            colXZDelta.y = 0;
            if(VECSquareMag(&colXZDelta) > 0.0001f) {
                VECNormalize(&colXZDelta, &colXZDelta);
            }
            if(!(colWork.attr & 0x20)) {
                col1.point.y = body1->param.radius+(col1.point.y-(0.5f*body1->param.height));
                col2.point.y = body2->param.radius+(col2.point.y-(0.5f*body2->param.height));
            }
            if(cbResult1) {
                ret = TRUE;
                colResult = broadP->colResult;
                // For existing overlaps, flag 0x40 forces side resolution and 0x80 forces
                // vertical resolution.
                if((body1->param.attr & 0x40) && colResult == 3) {
                    colResult = 2;
                } else if((body1->param.attr & 0x80) && colResult == 2) {
                    colResult = 3;
                }
                if(!(body1->param.attr & 0x8)) {
                    if(colResult == 2 && r1+r2 > 0.0001f) {
                        if(cbResult2) {
                            VECScale(&colDelta, &fixDir, r1/(r1+r2));
                            VECScale(&colXZDelta, &pointOfs, 0.001f+r1);
                            VECSubtract(&pointOfs, &fixDir, &fixDir);
                        } else {
                            VECScale(&colXZDelta, &pointOfs, 0.002f+(r1+r2));
                            VECSubtract(&pointOfs, &colDelta, &fixDir);
                        }
                        fixDir.y = 0;
                        if(VECSquareMag(&fixDir) > 0.001f) {
                            VECScale(&fixDir, &fixDir, -1);
                            if(fixDir.x < 0) {
                                if(newMove1.x > fixDir.x) {
                                    newMove1.x = fixDir.x;
                                }

                            } else if(newMove1.x < fixDir.x) {
                                newMove1.x = fixDir.x;
                            }
                            if(fixDir.z < 0) {
                                if(newMove1.z > fixDir.z) {
                                    newMove1.z = fixDir.z;
                                }
                            } else {
                                if(newMove1.z < fixDir.z) {
                                    newMove1.z = fixDir.z;
                                }
                            }
                        }
                    } else if(colResult == 3 && h1+h2 > 0.0001f) {
                        if(cbResult2) {
                            VECScale(&colDelta, &fixDir, h1/(h1+h2));
                            pointOfs.x = pointOfs.z = 0;
                            if(colDeltaNorm.y >= 0) {
                                pointOfs.y = 0.001f+h1;
                            } else {
                                pointOfs.y = -(0.001f+h1);
                            }
                            VECSubtract(&pointOfs, &fixDir, &fixDir);
                        } else {
                            pointOfs.x = pointOfs.z = 0;
                            if(colDeltaNorm.y >= 0) {
                                pointOfs.y = 0.002f+(h1+h2);
                            } else {
                                pointOfs.y = -(0.002f+(h1+h2));
                            }
                            VECSubtract(&pointOfs, &colDelta, &fixDir);
                        }
                        fixDir.x = fixDir.z = 0;
                        if(VECSquareMag(&fixDir) > 0.001f) {
                            VECScale(&fixDir, &fixDir, -1);
                            if(fixDir.y < 0) {
                                if(newMove1.y > fixDir.y) {
                                    newMove1.y = fixDir.y;
                                }
                            } else if(newMove1.y < fixDir.y) {
                                newMove1.y = fixDir.y;
                            }
                        }
                    }
                }
                fixDir.x = fixDir.y = fixDir.z  = 0;
                switch(colResult) {
                    case 0:
                    case 2:
                        dot = VECDotProduct(&colXZDelta, &newMove1);
                        if(dot > 0) {
                            VECScale(&colXZDelta, &fixDir, dot);
                        }
                        if(body2->param.attr & 0x20) {
                            body1->groundAttr |= 0x8000;
                        }
                        break;

                    case 1:
                    case 3:
                        // Remove the saved pre-hook Y movement toward the other body, even if the
                        // hooks changed
                        // the revised movement's Y component.
                        if ((colDeltaNorm.y < 0 && col1.normPos.y < 0) ||
                            (colDeltaNorm.y > 0 && col1.normPos.y > 0)) {
                            fixDir.y = col1.normPos.y;
                        }
                        if(body2->param.attr & 0x20) {
                            body1->groundAttr |= 0x4000;
                        }
                        break;
                }
                VECSubtract(&newMove1, &fixDir, &newMove1);
                if(VECSquareMag(&newMove1) > 0.001f) {
                    VECNormalize(&newMove1, &fixDir);
                } else {
                    fixDir = newMove1;
                }
                VECScale(&fixDir, &fixDir, 0.002f);
                VECScale(&newMove1, &pointOfs, broadP->t);
                VECSubtract(&col1.point, &pointOfs, &body1->oldPos);
                VECSubtract(&body1->oldPos, &fixDir, &body1->oldPos);
                body1->moveDir = newMove1;
                body1->pos[posNo] = col1.point;
                body1->param.attr |= COLBODY_ATTR_BODY_PAIR_CONTACT_FLAG;
                body1->colT = broadP->t;
                body1->oldColT = 1;
                ClearColPoint(body1);
            }
            VECScale(&colDeltaNorm, &colDeltaNorm, -1);
            VECScale(&colXZDelta, &colXZDelta, -1);
            VECScale(&colDelta, &colDelta, -1);
            if(cbResult2) {
                ret = TRUE;
                colResult = broadP->colResult;
                // For existing overlaps, flag 0x40 forces side resolution and 0x80 forces
                // vertical resolution.
                if((body2->param.attr & 0x40) && colResult == 3) {
                    colResult = 2;
                } else if((body2->param.attr & 0x80) && colResult == 2) {
                    colResult = 3;
                }
                if(!(body2->param.attr & 0x8)) {
                    if(colResult == 2 && r1+r2 > 0.0001f) {
                        if(cbResult1) {
                            VECScale(&colDelta, &fixDir, r2/(r1+r2));
                            VECScale(&colXZDelta, &pointOfs, 0.001f+r2);
                            VECSubtract(&pointOfs, &fixDir, &fixDir);
                        } else {
                            VECScale(&colXZDelta, &pointOfs, 0.002f+(r1+r2));
                            VECSubtract(&pointOfs, &colDelta, &fixDir);
                        }
                        fixDir.y = 0;
                        if(VECSquareMag(&fixDir) > 0.001f) {
                            VECScale(&fixDir, &fixDir, -1);
                            if(fixDir.x < 0) {
                                if(newMove2.x > fixDir.x) {
                                    newMove2.x = fixDir.x;
                                }

                            } else if(newMove2.x < fixDir.x) {
                                newMove2.x = fixDir.x;
                            }
                            if(fixDir.z < 0) {
                                if(newMove2.z > fixDir.z) {
                                    newMove2.z = fixDir.z;
                                }
                            } else {
                                if(newMove2.z < fixDir.z) {
                                    newMove2.z = fixDir.z;
                                }
                            }
                        }
                    } else if(colResult == 3 && h1+h2 > 0.0001f) {
                        if(cbResult1) {
                            pointOfs.x = pointOfs.z = 0;
                            VECScale(&colDelta, &fixDir, h2/(h1+h2));

                            if(colDeltaNorm.y >= 0) {
                                pointOfs.y = 0.001f+h2;
                            } else {
                                pointOfs.y = -(0.001f+h2);
                            }
                            VECSubtract(&pointOfs, &fixDir, &fixDir);
                        } else {
                            pointOfs.x = pointOfs.z = 0;
                            if(colDeltaNorm.y >= 0) {
                                pointOfs.y = 0.002f+(h1+h2);
                            } else {
                                pointOfs.y = -(0.002f+(h1+h2));
                            }
                            VECSubtract(&pointOfs, &colDelta, &fixDir);
                        }
                        fixDir.x = fixDir.z = 0;
                        if(VECSquareMag(&fixDir) > 0.001f) {
                            VECScale(&fixDir, &fixDir, -1);
                            if(fixDir.y < 0) {
                                if(newMove2.y > fixDir.y) {
                                    newMove2.y = fixDir.y;
                                }
                            } else if(newMove2.y < fixDir.y) {
                                newMove2.y = fixDir.y;
                            }
                        }
                    }
                }
                fixDir.x = fixDir.y = fixDir.z  = 0;
                switch(colResult) {
                    case 0:
                    case 2:
                        dot = VECDotProduct(&colXZDelta, &newMove2);
                        if(dot > 0) {
                            VECScale(&colXZDelta, &fixDir, dot);
                        }
                        if(body1->param.attr & 0x20) {
                            body2->groundAttr |= 0x8000;
                        }
                        break;

                    case 1:
                    case 3:
                        // Remove the saved pre-hook Y movement toward the other body, even if the
                        // hooks changed
                        // the revised movement's Y component.
                        if ((colDeltaNorm.y < 0 && col2.normPos.y < 0) ||
                            (colDeltaNorm.y > 0 && col2.normPos.y > 0)) {
                            fixDir.y = col2.normPos.y;
                        }
                        if(body1->param.attr & 0x20) {
                            body2->groundAttr |= 0x4000;
                        }
                        break;
                }
                VECSubtract(&newMove2, &fixDir, &newMove2);
                if(VECSquareMag(&newMove2) > 0.001f) {
                    VECNormalize(&newMove2, &fixDir);
                } else {
                    fixDir = newMove2;
                }
                // Use a negative offset here, so subtracting it nudges body2's reconstructed start
                // forward.
                VECScale(&fixDir, &fixDir, -0.002f);
                VECScale(&newMove2, &pointOfs, broadP->t);
                VECSubtract(&col2.point, &pointOfs, &body2->oldPos);
                VECSubtract(&body2->oldPos, &fixDir, &body2->oldPos);
                body2->moveDir = newMove2;
                body2->pos[posNo] = col2.point;
                body2->param.attr |= COLBODY_ATTR_BODY_PAIR_CONTACT_FLAG;
                body2->colT = broadP->t;
                body2->oldColT = 1;
                ClearColPoint(body2);
            }
        }

    }
    return ret;
}

// Applies the selected surface response parameters to a body's current contact.
// Called by _ColCorrection; its code parameter is unused. The contact limit stops movement and adds
// a random nudge. Mesh displacement with squared magnitude above 0.0001 restarts movement and
// restores a successful result.
static BOOL _BodyApplyColAttr(COLBODY *bodyP, COL_ATTRPARAM *attrParam, int code)
{
    COLBODY_POINT *point = &bodyP->colPoint[bodyP->colPointNum];
    int posIdx = (colWork.attr & 0x4) ? 1 : 0;
    BOOL result = (bodyP->param.attr & COLBODY_ATTR_CONTACT_POINT_LIMIT_FLAG) ? FALSE : TRUE;
    Mtx normMtx;
    HuVecF posOfs;
    HuVecF bounce;
    HuVecF moveDir;
    HuVecF up;
    HuVecF ofs;
    HuVecF normal;
    float c;
    float s;
    float upDot;
    float dot;

    if(!result) {
        bodyP->moveDir.x = 0;
        bodyP->moveDir.y = 0;
        bodyP->moveDir.z = 0;
        VECScale(&point->normal, &bounce, bodyP->param.bounce*(0.3f+((u32)frandmod(70)/100.0f)));
        VECScale(&bounce, &bounce, bodyP->param.radius);
        VECAdd(&bodyP->pos[posIdx], &bounce, &bodyP->pos[posIdx]);
        bodyP->oldPos = bodyP->pos[posIdx];
        if(VECSquareMag(&point->colOfs) > 0.0001f) {
            bodyP->moveDir = point->colOfs;
            result = TRUE;
        }
    } else {
        moveDir = bodyP->moveDir;
        up.x = 0;
        up.y = 1;
        up.z = 0;
    if(!(bodyP->param.attr & COLBODY_ATTR_SWEEP_CONTACT_FLAG) || (bodyP->param.attr & 0x200)) {
            moveDir.y *= attrParam->yDeviate;
            upDot = VECDotProduct(&up, &point->normal);
            if(upDot > 0 && upDot > attrParam->maxDot) {
                moveDir.y = 0;
            }
        }
        switch(attrParam->type) {
            case 0:
                normal = point->normal;
                c = cos((float)M_PI);
                s = sin((float)M_PI);
                normMtx[0][0] = ((1-c)*(normal.x*normal.x))+c;
                normMtx[0][1] = ((1-c)*(normal.x*normal.y))-(normal.z*s);
                normMtx[0][2] = ((1-c)*(normal.x*normal.z))+(normal.y*s);
                normMtx[1][0] = ((1-c)*(normal.y*normal.x))+(normal.z*s);
                normMtx[1][1] = ((1-c)*(normal.y*normal.y))+c;
                normMtx[1][2] = ((1-c)*(normal.y*normal.z))-(normal.x*s);
                normMtx[2][0] = ((1-c)*(normal.z*normal.x))-(normal.y*s);
                normMtx[2][1] = ((1-c)*(normal.z*normal.y))+(normal.x*s);
                normMtx[2][2] = ((1-c)*(normal.z*normal.z))+c;
                normMtx[0][3] = normMtx[1][3] = normMtx[2][3] = 0;
                // Reflect the original movement, discarding the vertical adjustments made above.
                MTXMultVec(normMtx, &bodyP->moveDir, &moveDir);
                moveDir.x = -moveDir.x;
                moveDir.y = -moveDir.y;
                moveDir.z = -moveDir.z;
                break;

            case 1:
                dot = VECDotProduct(&point->normal, &moveDir);
                if(dot < 0) {
                    VECScale(&point->normal, &ofs, dot);
                    VECSubtract(&moveDir, &ofs, &moveDir);
                }
                break;
        }
        bodyP->paramAttr = attrParam->attr;
        VECScale(&moveDir, &bodyP->moveDir, attrParam->speed);
        if(!(bodyP->param.attr & 0x100)) {
            VECAdd(&bodyP->moveDir, &point->colOfs, &bodyP->moveDir);
        }
        VECScale(&bodyP->moveDir, &posOfs, bodyP->colT);
        VECSubtract(&bodyP->pos[posIdx], &posOfs, &bodyP->oldPos);
        VECScale(&point->normal, &posOfs, 0.002f);
        VECAdd(&bodyP->oldPos, &posOfs, &bodyP->oldPos);
    }
    return result;
}

// Applies pending mesh responses to active bodies after body-pair resolution in ColBodyExec.
// Skips bodies already resolved as pairs and does not repeat an already-applied correction.
static inline int _BodyColResponse(void)
{
    COLBODY *bodyP;
    int posNo;
    int result;
    int i;
    bodyP = colWork.body;
    posNo = (colWork.attr & 0x4) ? 1 : 0;
    result = 0;
    for(i=0; i<colWork.bodyNum; i++, bodyP++) {
        if(!(bodyP->param.attr & COLBODY_ATTR_ACTIVE)) {
            continue;
        }
                if(!(bodyP->param.attr & COLBODY_ATTR_BODY_PAIR_CONTACT_FLAG)) {
            if(bodyP->param.attr & COLBODY_ATTR_MESH_CONTACT_MASK) {
                bodyP->colT = bodyP->oldColT;
                if ((bodyP->param.attr & COLBODY_ATTR_CONTACT_CORRECTED_FLAG) ||
                    _ColCorrection(bodyP)) {
                    result = 1;
                }
                if(bodyP->colPointNum >= 8) {
                    bodyP->param.attr |= COLBODY_ATTR_CONTACT_POINT_LIMIT_FLAG;
                } else {
                    bodyP->colPointNum++;
                }
            }

            bodyP->oldColT = 1;
        }

    }
    return result;
}

// Finishes each collision pass by retaining contacts and clearing temporary state.
#define _ColPointUpdate() \
do { \
    COLBODY *bodyP; \
    int no; \
    bodyP = colWork.body; \
    for(no=colWork.bodyNum; no--; bodyP++) { \
        if(bodyP->param.attr & COLBODY_ATTR_ACTIVE) { \
            if((bodyP->param.attr & COLBODY_ATTR_COLLISION_MASK) == 0) { \
                bodyP->param.attr |= COLBODY_ATTR_NO_CONTACT_FLAG; \
            } else { \
                bodyP->param.attr &= ~COLBODY_ATTR_NO_CONTACT_FLAG; \
            } \
            if(bodyP->param.attr & COLBODY_ATTR_CONTACT_CORRECTED_FLAG) { \
                if(bodyP->colPointNum >= 8) { \
                    bodyP->param.attr |= COLBODY_ATTR_CONTACT_POINT_LIMIT_FLAG; \
                } else { \
                    bodyP->colPointNum++; \
                } \
            } \
            bodyP->hitAttr |= bodyP->param.attr & COLBODY_ATTR_PASS_CONTACT_MASK; \
            bodyP->param.attr &= COLBODY_ATTR_PASS_RETAIN_MASK; \
        } \
    } \
} while(0)

// Refreshes body positions relative to each mesh for the current frame.
// Unused helper that combines the body and mesh position refresh steps.
static inline void _BodyMeshUpdate(int posNo)
{
    COLBODY *bodyP;
    COLMESH *meshP;
    int i;
    int no;
    bodyP = colWork.body;
    for(no=colWork.bodyNum; no--; bodyP++) {
        VECAdd(&bodyP->oldPos, &bodyP->moveDir, &bodyP->pos[posNo]);
        bodyP->param.attr |= bodyP->hitAttr;
    }
    meshP = colMesh;
    for(no=colMeshCount; no--; meshP++) {
        for(bodyP=colWork.body, i=0; i<colWork.bodyNum; i++, bodyP++) {
            meshP->bodyPos[i] = bodyP->pos[posNo];
            MTXMultVec(meshP->mtxInv, &meshP->bodyPos[i], &meshP->bodyPos[i]);

        }
    }
    (void)bodyP;
}

// Calculates the unit inward-facing edge normals used by triangle collision checks.
// Called by ColMakeTri after the triangle's plane normal is calculated.
static inline void MakeNormal(HuVecF *a, HuVecF *b, HuVecF *c, COLTRI *out)
{
    HuVecF ba;
    HuVecF cb;
    HuVecF ac;

    VECSubtract(b, a, &ba);
    VECSubtract(c, b, &cb);
    VECSubtract(a, c, &ac);
    VECCrossProduct(&out->norm, &ba, &out->edgeNorm1);
    VECCrossProduct(&out->norm, &cb, &out->edgeNorm2);
    VECCrossProduct(&out->norm, &ac, &out->edgeNorm3);
    VECNormalize(&out->edgeNorm1, &out->edgeNorm1);
    VECNormalize(&out->edgeNorm2, &out->edgeNorm2);
    VECNormalize(&out->edgeNorm3, &out->edgeNorm3);
}

// Builds plane, edge, center, and bound data for one mesh triangle.
// Called by ColMapInit while expanding each face into collision triangles.
static inline void ColMakeTri(COLTRI *tri, Vec *vtxBuf, int *idx)
{
    HuVecF ba;
    HuVecF cb;
    HuVecF aCenter;
    float dist;
    VECSubtract(&vtxBuf[idx[1]], &vtxBuf[idx[0]], &ba);
    VECSubtract(&vtxBuf[idx[2]], &vtxBuf[idx[1]], &cb);
    VECCrossProduct(&ba, &cb, &tri->norm);
    VECNormalize(&tri->norm, &tri->norm);
    tri->d = -VECDotProduct(&tri->norm, &vtxBuf[idx[0]]);
    MakeNormal(&vtxBuf[idx[0]], &vtxBuf[idx[1]], &vtxBuf[idx[2]], tri);
    tri->center.x = (vtxBuf[idx[0]].x+vtxBuf[idx[1]].x+vtxBuf[idx[2]].x)/3;
    tri->center.y = (vtxBuf[idx[0]].y+vtxBuf[idx[1]].y+vtxBuf[idx[2]].y)/3;
    tri->center.z = (vtxBuf[idx[0]].z+vtxBuf[idx[1]].z+vtxBuf[idx[2]].z)/3;
    VECSubtract(&vtxBuf[idx[0]], &tri->center, &aCenter);
    tri->dist = VECMag(&aCenter);
    VECSubtract(&vtxBuf[idx[1]], &tri->center, &aCenter);
    dist = VECMag(&aCenter);
    if(tri->dist < dist) {
        tri->dist = dist;
    }
    VECSubtract(&vtxBuf[idx[2]], &tri->center, &aCenter);
    dist = VECMag(&aCenter);
    if(tri->dist < dist) {
        tri->dist = dist;
    }
    tri->dist = (tri->dist*tri->dist);

}

COLBODY *ColBodyGet(int no)
{
    return &colWork.body[no];
}

// Clears collision-map bookkeeping and drops its model, body, and mesh registrations.
// Does not free allocated storage; use ColMapKill for teardown that releases it.
void ColMapClear(void)
{
    CancelTRXF = FALSE;
    colMapInitF = FALSE;
    colWork.mdlId = NULL;
    colWork.mdlNum = 0;
    colWork.attr = 0;
    colWork.broadCol = NULL;
    colWork.broadColNum = 0;
    colWork.bodyNum = 0;
    colWork.body = NULL;
    colWork.narrowCol = NULL;
    colWork.colOrder1 = NULL;
    colWork.colOrder2 = NULL;
    memset(colWork.attrParam, 0, sizeof(colWork.attrParam));
    memset(colWork.attrParamHi, 0, sizeof(colWork.attrParamHi));
    colMesh = NULL;
    colMeshCount = 0;
}

static void UseFloat3(void)
{
    (void)3.0f;
}

// Registers collision models and builds triangle data before actors begin moving.
void ColMapInit(HU3D_MODELID *mdlId, s16 mdlNum, int bodyNum)
{
    COLTRI *triP;
    HuVecF *vtxBuf;
    HSF_FACE *hsfFaceP;
    int triNum;
    COLTRI *triOther;
    int no;
    COLBODY *bodyP;
    int meshNum;
    HSF_OBJECT *objP;
    int i;
    COLMESH *meshP;
    int objNum;
    int faceNo;

    int index[3];

    if(ColMapInitCheck()) {
        return;
    }
    colWork.mdlNum = mdlNum;
    colWork.mdlId = HuMemDirectMallocNum(HEAP_MODEL, mdlNum*sizeof(HU3D_MODELID), HU_MEMNUM_OVL);
    colWork.attr = 1;
    colWork.bodyNum = bodyNum;
    if(!colWork.body) {
        colWork.body = HuMemDirectMallocNum(HEAP_MODEL, COLBODY_MAX*sizeof(COLBODY), HU_MEMNUM_OVL);
        memset(colWork.body, 0, COLBODY_MAX*sizeof(COLBODY));
    }
    if(!colWork.narrowCol) {
        colWork.narrowCol =
            HuMemDirectMallocNum(HEAP_MODEL, COLBODY_MAX * 4 * sizeof(COLNARROW), HU_MEMNUM_OVL);
        memset(colWork.narrowCol, 0, COLBODY_MAX*4*sizeof(COLNARROW));
    }
    memcpy(colWork.mdlId, mdlId, mdlNum*sizeof(HU3D_MODELID));
    for(no=7; no--;) {
        colWork.attrParam[no].type = 1;
        colWork.attrParam[no].speed = 1;
        colWork.attrParam[no].yDeviate = 1;
        colWork.attrParam[no].maxDot = 1;
        colWork.attrParam[no].attr = 0;
        colWork.attrParamHi[no].type = 1;
        colWork.attrParamHi[no].speed = 1;
        colWork.attrParamHi[no].yDeviate = 1;
        colWork.attrParamHi[no].maxDot = 1;
        colWork.attrParamHi[no].attr = 0;
    }
    colWork.broadCol = HuMemDirectMallocNum(
        HEAP_MODEL, ((colWork.bodyNum * (colWork.bodyNum + 1)) / 2.0f) * sizeof(COLBROAD),
        HU_MEMNUM_OVL);
    colWork.colOrder1 = HuMemDirectMallocNum(HEAP_MODEL, COLBODY_MAX*4*sizeof(s16), HU_MEMNUM_OVL);
    colWork.colOrder2 = HuMemDirectMallocNum(HEAP_MODEL, COLBODY_MAX*4*sizeof(s16), HU_MEMNUM_OVL);
    for(no=bodyNum; no--;) {
        bodyP = &colWork.body[no];
        bodyP->param.paramB = 0;
        bodyP->param.paramA = 0;
        bodyP->param.type = 0;
        bodyP->param.narrowHook = NULL;
        bodyP->param.narrowHook2 = NULL;
        bodyP->param.mask = -1;
        bodyP->param.attr = 0;
        bodyP->param.bounce = 0.2f;
        bodyP->param.height = 0;
        bodyP->param.radius = 0;
        bodyP->colT = 0;
        bodyP->oldColT = 1;
        bodyP->colPointNum = 0;
    }
    for(meshNum=0, no=colWork.mdlNum; no--;) {
        HSF_DATA *hsfP = Hu3DData[colWork.mdlId[no]].hsf;
        objP = hsfP->object;
        for(objNum=hsfP->objectNum; objNum--; objP++) {
            if(objP->type == HSF_OBJ_MESH) {
                meshNum++;
            }
        }
    }
    colMesh = HuMemDirectMallocNum(HEAP_MODEL, sizeof(COLMESH)*meshNum, HU_MEMNUM_OVL);
    colMeshCount = meshNum;
    for(meshNum=0, no=colWork.mdlNum; no--;) {
        HSF_DATA *hsfP = Hu3DData[colWork.mdlId[no]].hsf;
        objP = hsfP->object;
        for(objNum=hsfP->objectNum; objNum--; objP++) {
            if(objP->type == HSF_OBJ_MESH) {
                for (triNum = 0, hsfFaceP = objP->mesh.face->data, faceNo = objP->mesh.face->count;
                     faceNo--; hsfFaceP++) {
                    switch(hsfFaceP->type) {
                        case HSF_FACE_QUAD:
                            triNum += 2;
                            break;

                        case HSF_FACE_TRI:
                            triNum += 1;
                            break;

                        case HSF_FACE_TRISTRIP:
                            triNum += hsfFaceP->strip.count+1;
                            break;
                    }
                }
                vtxBuf = objP->mesh.vertex->data;
                triP = HuMemDirectMallocNum(HEAP_MODEL, sizeof(COLTRI)*triNum, HU_MEMNUM_OVL);
                memset(triP, 0, sizeof(COLTRI)*triNum);
                for (triNum = 0, hsfFaceP = objP->mesh.face->data, faceNo = objP->mesh.face->count;
                     faceNo--; hsfFaceP++) {
                    switch(hsfFaceP->type) {
                        case HSF_FACE_QUAD:
                            MakeIndexBuf(index, hsfFaceP, 0);
                            ColMakeTri(&triP[triNum], vtxBuf, index);
                            triNum++;
                            MakeIndexBuf(index, hsfFaceP, 1);
                            ColMakeTri(&triP[triNum], vtxBuf, index);
                            triNum++;
                            break;

                        case HSF_FACE_TRI:
                            MakeIndexBuf(index, hsfFaceP, 0);
                            ColMakeTri(&triP[triNum], vtxBuf, index);
                            triNum++;
                            break;

                        case HSF_FACE_TRISTRIP:
                            for(i=0; i<hsfFaceP->strip.count+1; i++) {
                                MakeIndexBuf(index, hsfFaceP, i);
                                triOther = &triP[triNum];
                                ColMakeTri(triOther, vtxBuf, index);
                                triNum++;
                            }
                            break;
                    }
                }
                // Warn at 256 meshes, but continue building this and subsequent mesh records.
                if(meshNum == 256) {
        OSReport("( colman.c : ColMapInit ) | テンポラリのバッファをオーバーライトしています\n\0\0\0\0");
                }
                memset(&colMesh[meshNum], 0, sizeof(COLMESH));
                colMesh[meshNum].tri = triP;
                colMesh[meshNum].obj = objP;
                colMesh[meshNum].bodyPos = HuMemDirectMallocNum(
                    HEAP_MODEL, sizeof(HuVecF) * colWork.bodyNum * 2, HU_MEMNUM_OVL);
                colMesh[meshNum].bodyMove = colMesh[meshNum].bodyPos+colWork.bodyNum;
                colMesh[meshNum].mask = -1;
                colMesh[meshNum].mdlNo = no;
                memset(colMesh[meshNum].bodyPos, 0, sizeof(HuVecF)*colWork.bodyNum);
                memset(colMesh[meshNum].bodyMove, 0, sizeof(HuVecF)*colWork.bodyNum);
                ColBoundsGet(&colMesh[meshNum].boundsCenter, &colMesh[meshNum].boundsRadius,
                    vtxBuf, objP->mesh.vertex->count);
                colMesh[meshNum].boundsRadius *= 1.25f;
                memset(colMesh[meshNum].mtx, 0, sizeof(Mtx));
                memset(colMesh[meshNum].mtxOld, 0, sizeof(Mtx));
                memset(colMesh[meshNum].mtxInv, 0, sizeof(Mtx));
                memset(colMesh[meshNum].mtxInvOld, 0, sizeof(Mtx));
                meshNum++;
            }
        }
    }
    colWork.attr |= 0x10;
    ColMtxCalcModelAll();
    for(meshP=colMesh, no=colMeshCount; no--; meshP++) {
        // Ignore inversion failure; a singular transform leaves the zeroed inverse in place.
        MTXInverse(meshP->mtx, meshP->mtxInv);
        memcpy(meshP->mtxOld, meshP->mtx, sizeof(Mtx));
        memcpy(meshP->mtxInvOld, meshP->mtxInv, sizeof(Mtx));
    }
    colMapInitF = TRUE;
}

// Sets the mesh mask used to include a registered model in collision queries.
void ColMapMaskSet(int mdlNo, u32 mask)
{
    COLMESH *meshP;
    int no;
    if(!ColMapInitCheck()) {
        return;
    }
    for(meshP=colMesh, no=colMeshCount; no--; meshP++) {
        if(meshP->mdlNo == mdlNo) {
            meshP->mask = mask;
        }
    }
}

// Returns the collision mask assigned to a registered model.
// An uninitialized map returns without a value; initialized maps return zero for an absent model.
u32 ColMapMaskGet(int mdlNo)
{
    COLMESH *meshP;
    int no;
    if(!ColMapInitCheck()) {
        return;
    }
    for(meshP=colMesh, no=colMeshCount; no--; meshP++) {
        if(meshP->mdlNo == mdlNo) {
            return meshP->mask;
        }
    }
    return 0;
}

// Disables cylinder collision mode and returns whether that mode was previously set.
BOOL ColCylReset(void)
{
    BOOL ret = (colWork.attr & 0x20) ? TRUE : FALSE;
    colWork.attr &= ~0x20;
    return ret;
}

// Enables cylinder collision mode and returns whether that mode was previously clear.
BOOL ColCylSet(void)
{
    BOOL ret = (colWork.attr & 0x20) ? FALSE : TRUE;
    colWork.attr |= 0x20;
    return ret;
}

// Frees collision-map model, body, pair, and triangle storage during teardown.
void ColMapKill(void)
{
    int no;
    CancelTRXF = FALSE;
    colMapInitF = FALSE;
    if(colWork.broadCol) {
        HuMemDirectFree(colWork.broadCol);
    }
    if(colWork.mdlId) {
        HuMemDirectFree(colWork.mdlId);
    }
    if(colWork.body) {
        HuMemDirectFree(colWork.body);
    }
    if(colWork.narrowCol) {
        HuMemDirectFree(colWork.narrowCol);
    }
    if(colWork.colOrder1) {
        HuMemDirectFree(colWork.colOrder1);
    }
    if(colWork.colOrder2) {
        HuMemDirectFree(colWork.colOrder2);
    }
    colWork.mdlNum = 0;
    colWork.broadCol = NULL;
    colWork.mdlId = NULL;
    colWork.attr = 0;
    colWork.broadColNum = 0;
    colWork.bodyNum = 0;
    colWork.body = NULL;
    colWork.narrowCol = NULL;
    colWork.colOrder1 = NULL;
    colWork.colOrder2 = NULL;
    memset(colWork.attrParam, 0, sizeof(colWork.attrParam));
    memset(colWork.attrParamHi, 0, sizeof(colWork.attrParamHi));
    if(colMesh) {
        for(no=colMeshCount; no--;) {
            if(colMesh[no].tri) {
                HuMemDirectFree(colMesh[no].tri);
            }
            if(colMesh[no].bodyPos) {
                HuMemDirectFree(colMesh[no].bodyPos);
            }
        }
        HuMemDirectFree(colMesh);
    }
    colMesh = NULL;
    colMeshCount = 0;
}

BOOL ColMapInitCheck(void)
{
    return colMapInitF;
}

// Clears the dirty and per-frame collision state bits.
void ColDirtyClear(void)
{
    colWork.attr &= ~0x18;
}

// Stores response parameters for the material code selected by a polygon flag.
void ColAttrParamSet(COL_ATTRPARAM *param, u32 polyAttr)
{
    int code = ColCodeGet(polyAttr);
    COL_ATTRPARAM *dst;
    if(code < 0 || !param) {
        return;
    }
    dst = (code >= 7) ? (&colWork.attrParamHi[code-7]) : (&colWork.attrParam[code]);
    *dst = *param;
}

// Retrieves response parameters for the material code selected by a polygon flag.
void ColAttrParamGet(COL_ATTRPARAM *param, u32 polyAttr)
{
    int code = ColCodeGet(polyAttr);
    COL_ATTRPARAM *dst;
    if(code < 0 || !param) {
        return;
    }
    dst = (code >= 7) ? (&colWork.attrParamHi[code-7]) : (&colWork.attrParam[code]);
    *param = *dst;
}

// Sets an actor body's position in the current buffer; COLBODY_ATTR_RESET also copies it to the
// other slot.
void ColBodyPosSet(HuVecF *pos, int no)
{
    COLBODY *bodyP = &colWork.body[no];
    int idx = (colWork.attr & 0x4) ? 1 : 0;
    HuVecF temp = *pos;
    bodyP->pos[idx] = temp;
    if(bodyP->param.attr & COLBODY_ATTR_RESET) {
        bodyP->pos[idx^1] = temp;
    }
}

// Reads the submitted position buffer, or the completed buffer after ColBodyExec swaps slots.
void ColBodyPosGet(HuVecF *pos, int no)
{
    int idx = (colWork.attr & 0x4) ? 1 : 0;
    if(colWork.attr & 0x8) {
        idx ^= 1;
    }
    *pos = colWork.body[no].pos[idx];
}

// Replaces the body's entire parameter block, including shape, mask, hooks, response, user data,
// and flags.
void ColBodyParamSet(COLBODY_PARAM *param, int no)
{
    if(no >= 0 && no < colWork.bodyNum) {
        colWork.body[no].param = *param;
    }
}

// Returns a body only when its index is within this map's reserved body range.
COLBODY *ColBodyGetSafe(int no)
{
    if(no >= 0 && no < colWork.bodyNum) {
        return &colWork.body[no];
    } else {
        return NULL;
    }
}

// Casts a segment through masked collision meshes, selecting hits with the plane-test tolerance.
// A hit up to 0.001 later than the current best can replace it, including beyond the segment end.
BOOL ColMapPolyGet(HuVecF *pos1, HuVecF *pos2, u32 mask, HuVecF *outPos, u32 *outCode,
                   int *outMdlNo, HSF_OBJECT **outObj, int *outFaceNo)
{
    HSF_FACE *hsfFaceP;
    COLMESH *meshP;
    COLTRI *triP;
    int i;
    int j;
    int faceNo;
    int triNum;
    u32 polyAttr;
    int mdlNo;
    HSF_OBJECT *obj;
    int no;
    BOOL temp;

    float dist;
    float mag2;

    HuVecF start;
    HuVecF end;
    HuVecF delta;

    int vtxIdx[3];
    HuVecF outDelta;
    HuVecF centerDist;
    HuVecF *vtx;
    int colPlaneResult;
    float outDist;

    meshP = colMesh;
    dist = 1;
    polyAttr = 0;
    mdlNo = 0;
    obj = NULL;
    faceNo = -1;
    for(no=colMeshCount; no--;  meshP++) {
        if(mask & meshP->mask) {
            start = *pos1;
            end = *pos2;
            MTXMultVec(meshP->mtxInv, &start, &start);
            MTXMultVec(meshP->mtxInv, &end, &end);
            VECSubtract(&end, &start, &delta);
            mag2 = VECSquareMag(&delta);
            vtx = meshP->obj->mesh.vertex->data;
            hsfFaceP = meshP->obj->mesh.face->data;
            triP = meshP->tri;
            for(i=0; i<meshP->obj->mesh.face->count; i++, hsfFaceP++) {
                switch(hsfFaceP->type) {
                    case HSF_FACE_QUAD:
                        triNum = 2;
                        break;

                    case HSF_FACE_TRI:
                        triNum = 1;
                        break;

                    case HSF_FACE_TRISTRIP:
                        triNum = hsfFaceP->strip.count+1;
                        break;
                }
                for(j=0; j<triNum; j++, triP++) {
                    if(triP->norm.x || triP->norm.y || triP->norm.z) {
                        MakeIndexBuf(vtxIdx, hsfFaceP, j);
                        VECSubtract(&triP->center, &start, &centerDist);
                        if(VECSquareMag(&centerDist) > triP->dist+mag2) {
                            temp = FALSE;
                        } else {
                            temp = TRUE;
                        }
                        if(temp != FALSE) {
                            colPlaneResult = ColPlaneDistGet(&start, &end, &delta, 0, dist, 0, vtx,
                                                             vtxIdx, triP, &outDist);
                            if(colPlaneResult >= 0) {
                                dist = outDist;
                                polyAttr = ColMatCodeGet(
                                    meshP->obj->mesh.material[hsfFaceP->mat & 0xFFF].flags);
                                mdlNo = meshP->mdlNo;
                                obj = meshP->obj;
                                faceNo = i;
                            }
                        }
                    }
                }
            }
        }
    }
    if(faceNo < 0) {
        return FALSE;
    }
    if(outPos) {
        VECSubtract(pos2, pos1, &outDelta);
        VECScale(&outDelta, &outDelta, dist);
        VECAdd(pos1, &outDelta, outPos);
    }
    if(outCode) {
        *outCode = polyAttr;
    }
    if(outMdlNo) {
        *outMdlNo = mdlNo;
    }
    if(outObj) {
        *outObj = obj;
    }
    if(outFaceNo) {
        *outFaceNo = faceNo;
    }
    return TRUE;
}

// Runs mesh and body collision passes after actor positions are submitted each frame.
// Called by the actor update after position submission; another call is skipped until
// ColDirtyClear.
void ColBodyExec(void)
{
    COLBODY *bodyP;
    COLMESH *meshP;
    int i;
    int no;
    int colNum;
    int posNo;

    HuVecF temp;
    int otherPosNo;

    posNo = (colWork.attr & 0x4) ? 1 : 0;
    otherPosNo = posNo ^ 1;
    colNum = 1;
    if(colWork.attr & 0x8) {
        return;
    }
    UpdateMeshMtx();
    for(bodyP=colWork.body, no=colWork.bodyNum; no--; bodyP++) {
        if(bodyP->param.attr & COLBODY_ATTR_ACTIVE) {
            bodyP->groundAttr = 0;
            bodyP->hitAttr = 0;
            bodyP->oldPos = bodyP->pos[otherPosNo];
            VECSubtract(&bodyP->pos[posNo], &bodyP->oldPos, &bodyP->moveDir);
            bodyP->pos[posNo] = bodyP->oldPos;
            bodyP->colT = 0;
            bodyP->oldColT = 1;
            bodyP->colPointNum = 0;
            bodyP->paramAttr = 0;
            memset(&bodyP->colBit[0], 0, sizeof(bodyP->colBit));
            ClearColPoint(bodyP);
        }
    }
    for(meshP=colMesh, no=colMeshCount; no--; meshP++) {
        for(bodyP=colWork.body, i=0; i<colWork.bodyNum; i++, bodyP++) {
            if(bodyP->param.attr & COLBODY_ATTR_ACTIVE) {
                if(bodyP->param.attr & COLBODY_ATTR_RESET) {
                    if(bodyP->param.attr & 0x8000) {
                        MTXMultVec(meshP->mtxInvOld, &bodyP->pos[otherPosNo], &temp);
                        MTXMultVec(meshP->mtx, &temp, &temp);
                        VECSubtract(&temp, &bodyP->pos[otherPosNo], &meshP->bodyMove[i]);
                    } else {
                        meshP->bodyMove[i].x = 0;
                        meshP->bodyMove[i].y = 0;
                        meshP->bodyMove[i].z = 0;
                    }
                } else {
                    MTXMultVec(meshP->mtx, &meshP->bodyPos[i], &temp);
                    VECSubtract(&temp, &bodyP->pos[otherPosNo], &meshP->bodyMove[i]);
                }
                if(fabs(meshP->bodyMove[i].x) < 0.001f) {
                    meshP->bodyMove[i].x = 0;
                }
                if(fabs(meshP->bodyMove[i].y) < 0.001f) {
                    meshP->bodyMove[i].y = 0;
                }
                if(fabs(meshP->bodyMove[i].z) < 0.001f) {
                    meshP->bodyMove[i].z = 0;
                }
            }
        }
    }
    // After calculating mesh-induced movement, consume reset and prior contact flags, keeping the
    // low 16 attribute bits.
    for(bodyP=colWork.body, no=colWork.bodyNum; no--; bodyP++) {
        bodyP->param.attr &= 0xFFFF;
    }
    while(colNum) {
        if(colWork.attr & 0x20) {
            _BodyMeshCylCol();
            colNum = _BodyColCylBroad();
        } else {
            _BodyMeshCol();
            colNum = _BodyColBroad();
        }
        colNum += _BodyColNarrow();
        colNum += _BodyColResponse();
        bodyP = colWork.body;
        for(no=colWork.bodyNum; no--; bodyP++) {
            if(bodyP->param.attr & COLBODY_ATTR_ACTIVE) {
                if((bodyP->param.attr & COLBODY_ATTR_COLLISION_MASK) == 0) {
                    bodyP->param.attr |= COLBODY_ATTR_NO_CONTACT_FLAG;
                } else {
                    bodyP->param.attr &= ~COLBODY_ATTR_NO_CONTACT_FLAG;
                }
                if(bodyP->param.attr & COLBODY_ATTR_CONTACT_CORRECTED_FLAG) {
                    if(bodyP->colPointNum >= 8) {
                        bodyP->param.attr |= COLBODY_ATTR_CONTACT_POINT_LIMIT_FLAG;
                    } else {
                        bodyP->colPointNum++;
                    }
                }
                bodyP->hitAttr |= bodyP->param.attr & COLBODY_ATTR_PASS_CONTACT_MASK;
                bodyP->param.attr &= COLBODY_ATTR_PASS_RETAIN_MASK;
            }
        }
    }
    bodyP = colWork.body;
    // Finalize every reserved body slot, including inactive ones, and restore its saved contact
    // flags.
    for(no=colWork.bodyNum; no--; bodyP++) {
        VECAdd(&bodyP->oldPos, &bodyP->moveDir, &bodyP->pos[posNo]);
        bodyP->param.attr |= bodyP->hitAttr;
    }
    meshP = colMesh;
    for(no=colMeshCount; no--; meshP++) {
        for(bodyP=colWork.body, i=0; i<colWork.bodyNum; i++, bodyP++) {
            meshP->bodyPos[i] = bodyP->pos[posNo];
            MTXMultVec(meshP->mtxInv, &meshP->bodyPos[i], &meshP->bodyPos[i]);
        }
    }
    // Swap submission slots; completed positions stay in the opposite slot for ColBodyPosGet.
    colWork.attr ^= 0x4;
    colWork.attr |= 0x8;
}
