// Queries map-model surfaces and accumulates horizontal wall collision corrections.
#include "game/hsfformat.h"
#include "game/object.h"
#include "game/hu3d.h"

#include "math.h"

#undef HuSetVecF

typedef struct MapSphere {
    HuVecF center;
    float radius;
} MAP_SPHERE;

void MapWallCheck(MAP_SPHERE *worldSphere, MAP_SPHERE *localSphere, HSF_MAPATTR *mapAttribute);
BOOL Hitcheck_Triangle_with_Sphere(HuVecF *triangle, HuVecF *sphereCenter, float sphereRadius,
                                   HuVecF *closestPoint);
BOOL Hitcheck_Quadrangle_with_Sphere(HuVecF *quadrangle, HuVecF *sphereCenter, float sphereRadius,
                                     HuVecF *closestPoint);
void AppendAddXZ(float directionX, float directionZ, float distance);
void CharRotInv(Mtx matrix, Mtx inverseMatrix, HuVecF *point, OMOBJ *object);

static BOOL PolygonRangeCheck(HSF_MAPATTR *mapAttribute, float x, float z, float *heightOut,
                              float worldHeightLimit);
static s32 DefIfnnerMapCircle(HuVecF *point, s16 *polygonData, HuVecF *vertices, HuVecF *direction);
static s32 CalcPPLength(MAP_SPHERE *sphere, s16 *polygonData, HuVecF *vertices);
static float MapIflnnerCalc(float x, float y, float z, HuVecF *edgeStart, HuVecF *edgeEnd,
                            HuVecF *direction);
static float MapCalcPoint(float x, float y, float z, HuVecF *vertices, u16 *polygonData);
static BOOL AreaCheck(float x, float z, u16 *polygonData, HuVecF *vertices);
static s32 MapIflnnerTriangle(float x, float z, u16 *polygonData, HuVecF *vertices);
static s32 MapIflnnerQuadrangle(float x, float z, u16 *polygonData, HuVecF *vertices);
BOOL GetPolygonCircleMtx(s16 *polygonData, HuVecF *vertices, MAP_SPHERE *worldSphere,
                                MAP_SPHERE *localSphere);
s32 PrecalcPntToTriangle(HuVecF *triangleOrigin, HuVecF *firstEdge, HuVecF *secondEdge,
                                HuVecF *faceNormal, HuVecF *relativePoint, HuVecF *closestOffset);
static void DefSetHitFace(float x, float y, float z);

// Map objects consulted by each query; only entries below nMap are visited.
OMOBJ *MapObject[16];
// Local-to-world and world-to-local transforms for the map object being queried.
Mtx MapMT;
Mtx MapMTR;
// Accumulated horizontal correction after the current object's matrix conversion.
static HuVecF MTRAdd;
// Local normal of the last containing triangle examined by a height query.
static HuVecF FieldVec;
// Vertex indices of triangles accepted by XZ containment tests.
s32 ColisionIdx[10][3];
// Normals recorded alongside wall contact points.
HuVecF HitFaceVec[32];
// Zero-initialized point used by the extra wall-plane side test; never updated here.
static HuVecF OldXYZ;
// Wall contact points in world coordinates; HitFaceCount is the next slot.
HuVecF HitFace[32];
// Storage reserved for character-query entries; this file does not access it.
u8 CharObject[40];

// Horizontal world-space correction accumulated by wall collision queries.
float AddX;
float AddZ;
// Number of registered map objects and character-query entries, respectively.
s32 nMap;
s32 nChar;
// Number of contacts for the current map object, reset by MapWall.
s32 HitFaceCount;
// Model data and vertex array used by the current polygon traversal.
static HSF_DATA *AttrHsf;
static HuVecF *topvtx;
// Number of triangles recorded by containment tests during the current query.
s32 ColisionCount;

// Handles a caller's sphere-versus-map wall query, accumulating XZ correction.
// Contacts and containment indices are reset separately for each registered map object.
void MapWall(float radius, float worldX, float worldY, float worldZ)
{
    MAP_SPHERE worldSphere; // World X, Y, Z followed by the query radius.
    MAP_SPHERE localSphere; // Object-space center followed by the unchanged radius.
    float queryRadius;
    float localX;
    float localZ;
    OMOBJ *mapObject;
    HU3D_MODEL *mapModel;
    HSF_DATA *mapData;
    HSF_MAPATTR *firstMapAttribute;
    HSF_MAPATTR *mapAttribute;
    s32 modelId;
    s32 objectIndex;
    s32 attributeIndex;

    for (objectIndex = 0; objectIndex < nMap; objectIndex++) {
        mapObject = MapObject[objectIndex];
        modelId = MapObject[objectIndex]->mdlId[0];
        localSphere.center.x = worldSphere.center.x = worldX;
        localSphere.center.y = worldSphere.center.y = worldY;
        localSphere.center.z = worldSphere.center.z = worldZ;
        localSphere.radius = worldSphere.radius = radius;
        queryRadius = worldSphere.radius;
        CharRotInv(MapMT, MapMTR, &localSphere.center, mapObject);
        ColisionCount = 0;
        HitFaceCount = 0;
        mapModel = &lbl_802C01C8[modelId];
        mapData = mapModel->hsf;
        AttrHsf = mapData;
        firstMapAttribute = AttrHsf->mapAttr;
        mapAttribute = mapData->mapAttr;
        for (attributeIndex = 0; attributeIndex < mapData->mapAttrNum;
             attributeIndex++, mapAttribute++) {
            localX = localSphere.center.x;
            localZ = localSphere.center.z;
            localSphere.radius = radius;
            if (mapAttribute->minX <= localX + queryRadius &&
                mapAttribute->maxX > localX - queryRadius &&
                mapAttribute->minZ <= localZ + queryRadius &&
                mapAttribute->maxZ > localZ - queryRadius) {
                MapWallCheck(&worldSphere, &localSphere, mapAttribute);
            }
        }
    }
}

// MapWall calls this for each nearby map-attribute block to test its flagged wall polygons.
// The world- and object-space sphere records hold a center and radius.
// Polygon tests accumulate a shared correction.
void MapWallCheck(MAP_SPHERE *worldSphere, MAP_SPHERE *localSphere, HSF_MAPATTR *mapAttribute) {
    u32 dataOffset;
    u16 polygonHeader;
    u16 *polygonData;
    s32 wallCount;
    Mtx normalMatrix;

    // The wall counter is initialized twice and is not consumed after traversal.
    wallCount = 0;
    wallCount = 0;
    topvtx = AttrHsf->vertex->data;
    polygonData = mapAttribute->data;
    MTRAdd.x = AddX;
    MTRAdd.z = AddZ;
    MTRAdd.y = 0.0f;
    PSMTXInvXpose(MapMT, normalMatrix);
    PSMTXMultVec(normalMatrix, &MTRAdd, &MTRAdd);
    for (dataOffset = 0; dataOffset < mapAttribute->dataLen;) {
        polygonHeader = *polygonData;
        if (polygonHeader & 0x8000) {
            GetPolygonCircleMtx((s16*) polygonData, topvtx, worldSphere, localSphere);
            wallCount++;
        }
        dataOffset += (polygonHeader & 0xFF) + 1;
        polygonData += (polygonHeader & 0xFF) + 1;
    }
}

// Handles a height query: each block supplies its highest surface strictly below
// worldY + heightAllowance, then the candidate closest to worldY is returned.
// The reported normal comes from the last containing triangle examined in the chosen block.
float MapPos(float worldX, float worldY, float worldZ, float heightAllowance, HuVecF *normalOut) {
    HuVecF queryPoint;
    float bestHeight;
    float localX;
    float candidateHeight;
    float localZ;
    HSF_MAPATTR *mapAttribute;
    HU3D_MODEL *mapModel;
    OMOBJ *mapObject;
    s32 objectIndex;
    s32 attributeIndex;
    HSF_DATA *mapData;
    Mtx normalMatrix;

    bestHeight = -100000.0f;
    ColisionCount = 0;
    for (objectIndex = 0; objectIndex < nMap; objectIndex++) {
        mapObject = MapObject[objectIndex];
        mapModel = &lbl_802C01C8[mapObject->mdlId[0]];
        mapData = mapModel->hsf;
        queryPoint.x = worldX;
        queryPoint.y = worldY;
        queryPoint.z = worldZ;
        CharRotInv(MapMT, MapMTR, &queryPoint, mapObject);
        localX = queryPoint.x;
        localZ = queryPoint.z;
        AttrHsf = mapData;
        mapAttribute = AttrHsf->mapAttr;
        for (attributeIndex = 0; attributeIndex < mapData->mapAttrNum;
             attributeIndex++, mapAttribute++) {
            if (mapAttribute->minX <= localX && mapAttribute->maxX >= localX &&
                mapAttribute->minZ <= localZ && mapAttribute->maxZ >= localZ &&
                PolygonRangeCheck(mapAttribute, localX, localZ, &candidateHeight,
                                  worldY + heightAllowance) == TRUE) {
                queryPoint.x = localX;
                queryPoint.y = candidateHeight;
                queryPoint.z = localZ;
                PSMTXMultVec(MapMT, &queryPoint, &queryPoint);
                candidateHeight = queryPoint.y;
                if (candidateHeight > worldY + heightAllowance ||
                    fabs(worldY - candidateHeight) > fabs(worldY - bestHeight)) {
                    continue;
                }
                bestHeight = candidateHeight;
                normalOut->x = FieldVec.x;
                normalOut->y = FieldVec.y;
                normalOut->z = FieldVec.z;
                PSMTXInvXpose(MapMT, normalMatrix);
                PSMTXMultVec(normalMatrix, normalOut, normalOut);
                bestHeight = queryPoint.y;
            }
        }
    }
    if (bestHeight == -100000.0f) {
        // No surface: retain the input height, write X twice, and leave normal Z untouched.
        normalOut->x = 0.0f;
        normalOut->y = 1.0f;
        normalOut->x = 0.0f;
        return worldY;
    } else {
        return bestHeight;
    }
}

// Called by MapPos for each candidate attribute block; finds the nearest height strictly below
// worldHeightLimit. Wall polygons are skipped, and heightOut remains in object coordinates.
static BOOL PolygonRangeCheck(HSF_MAPATTR *mapAttribute, float x, float z, float *heightOut,
                       float worldHeightLimit) {
    HuVecF worldPoint;
    float localHeight;
    float closestHeightGap;
    u16 *polygonData;
    u16 polygonHeader;
    s32 candidateCount; // Bounding-box candidates counted but not used to choose the height.
    s32 foundHeight;
    s32 dataOffset;

    candidateCount = 0;
    foundHeight = 0;
    closestHeightGap = 100000.0f;
    topvtx = AttrHsf->vertex->data;
    polygonData = mapAttribute->data;
    for (dataOffset = 0; dataOffset < mapAttribute->dataLen;) {
        polygonHeader = *polygonData;
        if (polygonHeader & 0x8000) {
            dataOffset += (polygonHeader & 0xFF) + 1;
            polygonData += (polygonHeader & 0xFF) + 1;
        } else {
            switch (polygonHeader & 0xFF) {
                case 1:
                    dataOffset += 2;
                    polygonData += 2;
                    break;
                case 2:
                    dataOffset += 3;
                    polygonData += 3;
                    break;
                case 3:
                    if (AreaCheck(x, z, polygonData, topvtx) == TRUE) {
                        candidateCount++;
                        if (MapIflnnerTriangle(x, z, polygonData, topvtx) == 1) {
                            localHeight = MapCalcPoint(x, 0.0f, z, topvtx, polygonData);
                            worldPoint.x = x;
                            worldPoint.y = localHeight;
                            worldPoint.z = z;
                            PSMTXMultVec(MapMT, &worldPoint, &worldPoint);
                            if (worldHeightLimit > worldPoint.y &&
                                closestHeightGap > fabs(worldHeightLimit - worldPoint.y)) {
                                closestHeightGap = fabs(worldHeightLimit - worldPoint.y);
                                *heightOut = localHeight;
                                foundHeight = 1;
                            }
                        }
                    }
                    dataOffset += 4;
                    polygonData += 4;
                    break;
                case 4:
                    if (AreaCheck(x, z, polygonData, topvtx) == TRUE) {
                        candidateCount++;
                        if (MapIflnnerQuadrangle(x, z, polygonData, topvtx) == 1) {
                            localHeight = MapCalcPoint(x, 0.0f, z, topvtx, polygonData);
                            worldPoint.x = x;
                            worldPoint.y = localHeight;
                            worldPoint.z = z;
                            PSMTXMultVec(MapMT, &worldPoint, &worldPoint);
                            if (worldHeightLimit > worldPoint.y) {
                                if (closestHeightGap > fabs(worldHeightLimit - worldPoint.y)) {
                                    closestHeightGap = fabs(worldHeightLimit - worldPoint.y);
                                    *heightOut = localHeight;
                                    foundHeight = 1;
                                }
                            }
                        }
                    }
                    dataOffset += 5;
                    polygonData += 5;
                    break;
                default:
                    dataOffset++;
                    polygonData++;
                    break;
            }
        }
    }
    if (foundHeight != 0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

// Called by GetPolygonCircleMtx to test polygon edge sides along the supplied vector.
// Triangles can return inside; the quadrangle path performs edge tests but always returns zero.
static s32 DefIfnnerMapCircle(HuVecF *point, s16 *polygonData, HuVecF *vertices,
                              HuVecF *direction) {
    float pointX;
    float pointY;
    float pointZ;
    float edgeSide;
    s32 edgeIndex;
    s32 polygonState; // Vertex count initially; quadrangle edge tests can replace it with one.
    s32 nextEdgeIndex;

    pointX = point->x;
    pointY = point->y;
    pointZ = point->z;
    polygonState = *polygonData & 0xFF;
    polygonData++;
    if (polygonState == 3) {
        edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[0]],
                                  &vertices[polygonData[1]], direction);
        if (edgeSide > 0.0f) {
            for (edgeIndex = 1; edgeIndex < polygonState; edgeIndex++) {
                nextEdgeIndex = (edgeIndex + 1) % polygonState;
                edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[edgeIndex]],
                                         &vertices[polygonData[nextEdgeIndex]], direction);
                if (edgeSide < 0.0f) {
                    return 0;
                }
            }
            return 1;
        } else {
            for (edgeIndex = 1; edgeIndex < polygonState; edgeIndex++) {
                nextEdgeIndex = (edgeIndex + 1) % polygonState;
                edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[edgeIndex]],
                                         &vertices[polygonData[nextEdgeIndex]], direction);
                if (edgeSide > 0.0f) {
                    return 0;
                }
            }
            return 1;
        }
    } else if (polygonState == 4) {
        edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[0]],
                                  &vertices[polygonData[2]], direction);
        if (edgeSide > 0.0f) {
            edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[2]],
                                      &vertices[polygonData[3]], direction);
            if (edgeSide < 0.0f) {
                polygonState = 1;
            } else {
                edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[3]],
                                         &vertices[polygonData[0]], direction);
                if (edgeSide < 0.0f) {
                    polygonState = 1;
                }
            }
        } else {
            edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[2]],
                                      &vertices[polygonData[3]], direction);
            if (edgeSide > 0.0f) {
                polygonState = 1;
            } else {
                edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[3]],
                                         &vertices[polygonData[0]], direction);
                if (edgeSide > 0.0f) {
                    polygonState = 1;
                }
            }
        }
        if (polygonState != 0) {
            edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[0]],
                                      &vertices[polygonData[3]], direction);
            if (edgeSide > 0.0f) {
                edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[3]],
                                         &vertices[polygonData[1]], direction);
                if (edgeSide < 0.0f) {
                    return 0;
                }
                edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[1]],
                                         &vertices[polygonData[0]], direction);
                if (edgeSide < 0.0f) {
                    return 0;
                }
            } else {
                edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[3]],
                                         &vertices[polygonData[1]], direction);
                if (edgeSide > 0.0f) {
                    return 0;
                }
                edgeSide = MapIflnnerCalc(pointX, pointY, pointZ, &vertices[polygonData[1]],
                                         &vertices[polygonData[0]], direction);
                if (edgeSide > 0.0f) {
                    return 0;
                }
            }
        }
    }
    return 0;
}

// Used by face-normal and wall-correction helpers to normalize nonzero vectors in place.
// A zero vector is left unchanged.
static inline void MapspaceInlineFunc00(HuVecF *vector) {
    float x;
    float y;
    float z;
    float length;

    x = vector->x;
    y = vector->y;
    z = vector->z;
    length = sqrtf(x * x + y * y + z * z);
    if (length != 0.0f) {
        vector->x /= length;
        vector->y /= length;
        vector->z /= length;
    }
}

// Used by height, wall-plane and containment tests to build a normalized face normal.
// Collinear points produce a zero normal.
static inline void MapspaceInlineFunc01(HuVecF *normalOut, HuVecF *origin, HuVecF *firstPoint,
                                        HuVecF *secondPoint) {
    float firstEdgeX;
    float firstEdgeY;
    float firstEdgeZ;
    float secondEdgeX;
    float secondEdgeY;
    float secondEdgeZ;

    firstEdgeX = firstPoint->x - origin->x;
    firstEdgeY = firstPoint->y - origin->y;
    firstEdgeZ = firstPoint->z - origin->z;
    secondEdgeX = secondPoint->x - origin->x;
    secondEdgeY = secondPoint->y - origin->y;
    secondEdgeZ = secondPoint->z - origin->z;
    normalOut->x = firstEdgeY * secondEdgeZ - firstEdgeZ * secondEdgeY;
    normalOut->y = firstEdgeZ * secondEdgeX - firstEdgeX * secondEdgeZ;
    normalOut->z = firstEdgeX * secondEdgeY - firstEdgeY * secondEdgeX;
    MapspaceInlineFunc00(normalOut);
}

// Called by GetPolygonCircleMtx to reject unflagged or distant wall planes and project the sphere
// center onto the plane. Returns zero for rejection, otherwise minus one or one for its side.
static s32 CalcPPLength(MAP_SPHERE *sphere, s16 *polygonData, HuVecF *vertices) {
    HuVecF *planePoint;
    HuVecF faceNormal;
    float projectionDistance; // Signed displacement from the sphere center to the wall plane.
    float pointOffsetX;
    float pointOffsetY;
    float pointOffsetZ;
    float signedDistance;
    float planeX;
    float planeY;
    float planeZ;
    s16 polygonHeader;
    s16 vertexIndex;
    s32 planeSide;

    planeSide = -1;
    polygonHeader = polygonData[0];
    if (!(polygonHeader & 0x8000)) {
        return 0;
    }
    if ((polygonHeader & 0xFF) == 4) {
        MapspaceInlineFunc01(&faceNormal, &vertices[polygonData[1]], &vertices[polygonData[4]],
                             &vertices[polygonData[3]]);
    } else {
        MapspaceInlineFunc01(&faceNormal, &vertices[polygonData[1]], &vertices[polygonData[2]],
                             &vertices[polygonData[3]]);
    }
    vertexIndex = polygonData[1];
    planePoint = &vertices[vertexIndex];
    planeX = planePoint->x;
    planeY = planePoint->y;
    planeZ = planePoint->z;
    pointOffsetX = planeX - sphere->center.x;
    pointOffsetY = planeY - sphere->center.y;
    pointOffsetZ = planeZ - sphere->center.z;
    signedDistance =
        faceNormal.x * pointOffsetX + faceNormal.y * pointOffsetY + faceNormal.z * pointOffsetZ;
    if (signedDistance >= 0.0f) {
        planeSide = 1;
    }
    if (fabs(signedDistance) > sphere->radius) {
        return 0;
    }
    projectionDistance =
        faceNormal.x * pointOffsetX + faceNormal.y * pointOffsetY + faceNormal.z * pointOffsetZ;
    sphere->center.x += faceNormal.x * projectionDistance;
    sphere->center.y += faceNormal.y * projectionDistance;
    sphere->center.z += faceNormal.z * projectionDistance;
    return planeSide;
}

// Called by DefIfnnerMapCircle to compare a point with an edge along the supplied direction.
// The sign of the scalar triple product is used for containment tests.
static float MapIflnnerCalc(float x, float y, float z, HuVecF *edgeStart, HuVecF *edgeEnd,
                            HuVecF *direction) {
    float startOffsetX;
    float startOffsetY;
    float startOffsetZ;
    float endOffsetX;
    float endOffsetY;
    float endOffsetZ;
    float edgeSide;

    startOffsetX = edgeStart->x - x;
    startOffsetY = edgeStart->y - y;
    startOffsetZ = edgeStart->z - z;
    endOffsetX = edgeEnd->x - x;
    endOffsetY = edgeEnd->y - y;
    endOffsetZ = edgeEnd->z - z;
    edgeSide = direction->x * (startOffsetY * endOffsetZ - startOffsetZ * endOffsetY)
        + direction->y * (startOffsetZ * endOffsetX - startOffsetX * endOffsetZ)
        + direction->z * (startOffsetX * endOffsetY - startOffsetY * endOffsetX);
    return edgeSide;
}

// Called by PolygonRangeCheck after containment accepts a triangle; finds the vertical plane
// intersection and records its normal. The polygonData argument is unused.
static float MapCalcPoint(float x, float y, float z, HuVecF *vertices, u16 *polygonData) {
    HuVecF faceNormal;
    float planeZ;
    float planeY;
    float planeX;
    float pointX;
    float pointZ;
    float verticalStep;
    float heightOffset;
    float pointY;
    float normalX;
    float normalY;
    float normalZ;
    s32 collisionIndex;
    HuVecF *planePoint;

    collisionIndex = ColisionCount - 1;
    planePoint = &vertices[ColisionIdx[collisionIndex][0]];
    planeX = planePoint->x;
    planeY = planePoint->y;
    planeZ = planePoint->z;
    pointX = x;
    pointY = y;
    pointZ = z;
    verticalStep = 1.0f;
    MapspaceInlineFunc01(&faceNormal, &vertices[ColisionIdx[collisionIndex][0]],
                         &vertices[ColisionIdx[collisionIndex][1]],
                         &vertices[ColisionIdx[collisionIndex][2]]);
    normalX = faceNormal.x;
    normalY = faceNormal.y;
    normalZ = faceNormal.z;
    FieldVec.x = normalX;
    FieldVec.y = normalY;
    FieldVec.z = normalZ;
    heightOffset =
        normalX * (planeX - pointX) + normalY * (planeY - pointY) + normalZ * (planeZ - pointZ);
    heightOffset /= normalY;
    return pointY + verticalStep * heightOffset;
}

// Called by PolygonRangeCheck before containment tests; checks the polygon's inclusive XZ bounds.
static BOOL AreaCheck(float x, float z, u16 *polygonData, HuVecF *vertices) {
    float maxX;
    float maxZ;
    float minX;
    float minZ;
    s32 vertexCount;
    s32 vertexIndex;
    s32 cornerIndex;

    maxX = maxZ = -100000.0f;
    minX = minZ = 100000.0f;
    vertexCount = *polygonData & 0xFF;
    polygonData++;
    for (cornerIndex = 0; cornerIndex < vertexCount; cornerIndex++, polygonData++) {
        vertexIndex = *polygonData;
        if (minX > vertices[vertexIndex].x) {
            minX = vertices[vertexIndex].x;
        }
        if (maxX < vertices[vertexIndex].x) {
            maxX = vertices[vertexIndex].x;
        }
        if (minZ > vertices[vertexIndex].z) {
            minZ = vertices[vertexIndex].z;
        }
        if (maxZ < vertices[vertexIndex].z) {
            maxZ = vertices[vertexIndex].z;
        }
    }
    if (minX <= x && maxX >= x
        && minZ <= z && maxZ >= z) {
        return TRUE;
    } else {
        return FALSE;
    }
}

// Used by triangle and quadrangle height tests to compare an XZ point with an edge.
// The sign of this 2D cross product determines which side contains the point.
static inline float MapspaceInlineFunc02(float x, float z, HuVecF *edgeStart, HuVecF *edgeEnd) {
    float startOffsetX;
    float startOffsetZ;
    float endOffsetX;
    float endOffsetZ;
    float edgeSide;

    startOffsetX = edgeStart->x - x;
    startOffsetZ = edgeStart->z - z;
    endOffsetX = edgeEnd->x - x;
    endOffsetZ = edgeEnd->z - z;
    edgeSide = -(startOffsetZ * endOffsetX - startOffsetX * endOffsetZ);
    return edgeSide;
}

// Called by PolygonRangeCheck to test XZ containment; rejects vertical faces and records the
// triangle's indices on success.
static s32 MapIflnnerTriangle(float x, float z, u16 *polygonData, HuVecF *vertices) {
    HuVecF faceNormal;
    float edgeSide;
    s32 nextEdgeIndex;
    s32 edgeIndex;

    MapspaceInlineFunc01(&faceNormal, &vertices[polygonData[1]], &vertices[polygonData[2]],
                         &vertices[polygonData[3]]);
    if (faceNormal.y == 0.0f) {
        return 0;
    }
    polygonData++;
    edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[0]], &vertices[polygonData[1]]);
    if (edgeSide > 0.0f) {
        for (edgeIndex = 1; edgeIndex < 3; edgeIndex++) {
            nextEdgeIndex = (edgeIndex + 1) % 3;
            edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[edgeIndex]],
                                            &vertices[polygonData[nextEdgeIndex]]);
            if (edgeSide < 0.0f) {
                return 0;
            }
        }
    } else {
        for (edgeIndex = 1; edgeIndex < 3; edgeIndex++) {
            nextEdgeIndex = (edgeIndex + 1) % 3;
            edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[edgeIndex]],
                                            &vertices[polygonData[nextEdgeIndex]]);
            if (edgeSide > 0.0f) {
                return 0;
            }
        }
    }
    ColisionIdx[ColisionCount][0] = polygonData[0];
    ColisionIdx[ColisionCount][1] = polygonData[1];
    ColisionIdx[ColisionCount][2] = polygonData[2];
    ColisionCount++;
    return 1;
}

// Called by PolygonRangeCheck to test XZ containment against two triangles of a quadrangle.
// Records the accepted split (0, 3, 2) or (0, 1, 3); vertical faces are rejected.
static s32 MapIflnnerQuadrangle(float x, float z, u16 *polygonData, HuVecF *vertices) {
    HuVecF faceNormal;
    float edgeSide;
    s32 tryOtherTriangle;

    MapspaceInlineFunc01(&faceNormal, &vertices[polygonData[1]], &vertices[polygonData[2]],
                         &vertices[polygonData[3]]);
    if (faceNormal.y == 0.0f) {
        return 0;
    }
    tryOtherTriangle = 0;
    polygonData++;
    edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[0]], &vertices[polygonData[3]]);
    if (edgeSide > 0.0f) {
        edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[3]], &vertices[polygonData[2]]);
        if (edgeSide < 0.0f) {
            tryOtherTriangle = 1;
        } else {
            edgeSide =
                MapspaceInlineFunc02(x, z, &vertices[polygonData[2]], &vertices[polygonData[0]]);
            if (edgeSide < 0.0f) {
                tryOtherTriangle = 1;
            }
        }
    } else {
        edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[3]], &vertices[polygonData[2]]);
        if (edgeSide > 0.0f) {
            tryOtherTriangle = 1;
        } else {
            edgeSide =
                MapspaceInlineFunc02(x, z, &vertices[polygonData[2]], &vertices[polygonData[0]]);
            if (edgeSide > 0.0f) {
                tryOtherTriangle = 1;
            }
        }
    }
    if (tryOtherTriangle == 0) {
        ColisionIdx[ColisionCount][0] = polygonData[0];
        ColisionIdx[ColisionCount][1] = polygonData[3];
        ColisionIdx[ColisionCount][2] = polygonData[2];
        ColisionCount++;
        return 1;
    }
    edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[0]], &vertices[polygonData[1]]);
    if (edgeSide > 0.0f) {
        edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[1]], &vertices[polygonData[3]]);
        if (edgeSide < 0.0f) {
            return 0;
        }
        edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[3]], &vertices[polygonData[0]]);
        if (edgeSide < 0.0f) {
            return 0;
        }
    } else {
        edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[1]], &vertices[polygonData[3]]);
        if (edgeSide > 0.0f) {
            return 0;
        }
        edgeSide = MapspaceInlineFunc02(x, z, &vertices[polygonData[3]], &vertices[polygonData[0]]);
        if (edgeSide > 0.0f) {
            return 0;
        }
    }
    ColisionIdx[ColisionCount][0] = polygonData[0];
    ColisionIdx[ColisionCount][1] = polygonData[1];
    ColisionIdx[ColisionCount][2] = polygonData[3];
    ColisionCount++;
    return 1;
}

// Called by GetPolygonCircleMtx after transforming OldXYZ to object coordinates; returns the
// saved point's side of the plane through the first three polygon vertices.
static inline s32 MapspaceInlineFunc03(MAP_SPHERE *sphere, s16 *vertexIndices, HuVecF *vertices) {
    HuVecF faceNormal;
    HuVecF *planePoint;
    float pointOffsetX;
    float pointOffsetY;
    float pointOffsetZ;
    float signedDistance;
    s16 vertexIndex;

    MapspaceInlineFunc01(&faceNormal, &vertices[vertexIndices[0]], &vertices[vertexIndices[1]],
                         &vertices[vertexIndices[2]]);
    vertexIndex = vertexIndices[1];
    planePoint = &vertices[vertexIndex];
    pointOffsetX = planePoint->x;
    pointOffsetY = planePoint->y;
    pointOffsetZ = planePoint->z;
    pointOffsetX -= sphere->center.x;
    pointOffsetY -= sphere->center.y;
    pointOffsetZ -= sphere->center.z;
    signedDistance =
        faceNormal.x * pointOffsetX + faceNormal.y * pointOffsetY + faceNormal.z * pointOffsetZ;
    return (signedDistance < 0.0f) ? -1 : 1;
}

// Called by MapWallCheck for one wall polygon; tests sphere overlap, records the contact and
// accumulates horizontal separation. A positive plane-side result also checks OldXYZ.
BOOL GetPolygonCircleMtx(s16 *polygonData, HuVecF *vertices, MAP_SPHERE *worldSphere,
                                MAP_SPHERE *localSphere) {
    HuVecF quadrangle[4];
    HuVecF triangle[3];
    MAP_SPHERE projectedSphere; // Object-space projected center and radius; later holds OldXYZ.
    MAP_SPHERE querySphere; // Corrected local sphere, later replaced with its world-space center.
    float contactOffsetX;
    float contactOffsetZ;
    float pushDistance;
    HuVecF closestPoint;
    HuVecF movementDirection;
    s32 unusedCounter; // Initialized but not used by the collision calculation.
    float contactOffsetY;
    s32 planeSide;
    BOOL overlaps;
    s16 *vertexIndices;
    HuVecF *contactNormal;
    Mtx normalMatrix;

    unusedCounter = 0;
    querySphere.center.x = projectedSphere.center.x = localSphere->center.x + MTRAdd.x;
    querySphere.center.y = projectedSphere.center.y = localSphere->center.y;
    querySphere.center.z = projectedSphere.center.z = localSphere->center.z + MTRAdd.z;
    querySphere.radius = projectedSphere.radius = localSphere->radius;
    vertexIndices = polygonData + 1;
    if ((planeSide = CalcPPLength(&projectedSphere, polygonData, vertices)) == 0) {
        return 0;
    }
    closestPoint.x = closestPoint.y = closestPoint.z = 0.0f;
    if ((polygonData[0] & 0xFF) == 4) {
        quadrangle[0].x = vertices[vertexIndices[0]].x;
        quadrangle[0].y = vertices[vertexIndices[0]].y;
        quadrangle[0].z = vertices[vertexIndices[0]].z;
        quadrangle[1].x = vertices[vertexIndices[1]].x;
        quadrangle[1].y = vertices[vertexIndices[1]].y;
        quadrangle[1].z = vertices[vertexIndices[1]].z;
        quadrangle[2].x = vertices[vertexIndices[2]].x;
        quadrangle[2].y = vertices[vertexIndices[2]].y;
        quadrangle[2].z = vertices[vertexIndices[2]].z;
        quadrangle[3].x = vertices[vertexIndices[3]].x;
        quadrangle[3].y = vertices[vertexIndices[3]].y;
        quadrangle[3].z = vertices[vertexIndices[3]].z;
        overlaps = Hitcheck_Quadrangle_with_Sphere(quadrangle, &querySphere.center,
                                                   projectedSphere.radius, &closestPoint);
    } else {
        triangle[0].x = vertices[vertexIndices[0]].x;
        triangle[0].y = vertices[vertexIndices[0]].y;
        triangle[0].z = vertices[vertexIndices[0]].z;
        triangle[1].x = vertices[vertexIndices[1]].x;
        triangle[1].y = vertices[vertexIndices[1]].y;
        triangle[1].z = vertices[vertexIndices[1]].z;
        triangle[2].x = vertices[vertexIndices[2]].x;
        triangle[2].y = vertices[vertexIndices[2]].y;
        triangle[2].z = vertices[vertexIndices[2]].z;
        overlaps = Hitcheck_Triangle_with_Sphere(triangle, &querySphere.center,
                                                 projectedSphere.radius, &closestPoint);
    }
    if (overlaps == TRUE) {
        querySphere.center.x = worldSphere->center.x + AddX;
        querySphere.center.y = worldSphere->center.y;
        querySphere.center.z = worldSphere->center.z + AddZ;
        PSMTXMultVec(MapMT, &closestPoint, &closestPoint);
        DefSetHitFace(closestPoint.x, closestPoint.y, closestPoint.z);
        contactNormal = &HitFaceVec[HitFaceCount];
        // This normal indexes the header and the first two indices as vertex numbers.
        MapspaceInlineFunc01(contactNormal, &vertices[polygonData[0]], &vertices[polygonData[1]],
                             &vertices[polygonData[2]]);
        contactOffsetX = closestPoint.x - querySphere.center.x;
        contactOffsetY = closestPoint.y - querySphere.center.y;
        contactOffsetZ = closestPoint.z - querySphere.center.z;
        pushDistance = projectedSphere.radius -
                       sqrtf(contactOffsetX * contactOffsetX + contactOffsetZ * contactOffsetZ);
        HitFaceCount++;
        if (planeSide > 0) {
            projectedSphere.center.x = OldXYZ.x;
            projectedSphere.center.y = OldXYZ.y;
            projectedSphere.center.z = OldXYZ.z;
            PSMTXMultVec(MapMTR, &projectedSphere.center, &projectedSphere.center);
            if (MapspaceInlineFunc03(&projectedSphere, vertexIndices, vertices) < 0) {
                // This direction subtracts the world-space query center from the object-space
                // OldXYZ point.
                movementDirection.x = projectedSphere.center.x - querySphere.center.x;
                movementDirection.y = projectedSphere.center.y - querySphere.center.y;
                movementDirection.z = projectedSphere.center.z - querySphere.center.z;
                MapspaceInlineFunc00(&movementDirection);
                // The edge test starts one word before this polygon's header.
                if (DefIfnnerMapCircle(&querySphere.center, polygonData - 1, vertices,
                                       &movementDirection) == 1) {
                    pushDistance = projectedSphere.radius + sqrtf(contactOffsetX * contactOffsetX +
                                                              contactOffsetZ * contactOffsetZ);
                }
            } else {
                pushDistance = 0.0f;
            }
        }
        if (pushDistance > 0.0f) {
            AppendAddXZ(-contactOffsetX, -contactOffsetZ, pushDistance);
            MTRAdd.x = AddX;
            MTRAdd.z = AddZ;
            MTRAdd.y = 0.0f;
            PSMTXInvXpose(MapMT, normalMatrix);
            PSMTXMultVec(normalMatrix, &MTRAdd, &MTRAdd);
        }
    }
    return overlaps;
}

// Called by both sphere hit tests to compute a closest-point offset from two triangle edges.
// triangleOrigin is unused. Non-origin endpoint cases only redirect the local output
// pointer, leaving the caller's vector unchanged; origin endpoint cases write zero.
s32 PrecalcPntToTriangle(HuVecF *triangleOrigin, HuVecF *firstEdge, HuVecF *secondEdge,
                                HuVecF *faceNormal, HuVecF *relativePoint, HuVecF *closestOffset) {
    HuVecF projectionVector;
    HuVecF edgeOffset;
    float reciprocalDeterminant;
    float firstWeight;
    float secondWeight;
    float edgeFraction;

    HuSetVecF(&projectionVector, -relativePoint->x, -relativePoint->y, -relativePoint->z);
    reciprocalDeterminant = 1.0f / (-(firstEdge->z * secondEdge->y * faceNormal->x) +
                                    firstEdge->y * secondEdge->z * faceNormal->x +
                                    firstEdge->z * secondEdge->x * faceNormal->y -
                                    firstEdge->x * secondEdge->z * faceNormal->y -
                                    firstEdge->y * secondEdge->x * faceNormal->z +
                                    firstEdge->x * secondEdge->y * faceNormal->z);
    firstWeight =
        reciprocalDeterminant *
        (secondEdge->z * (faceNormal->y * projectionVector.x - faceNormal->x * projectionVector.y) +
         secondEdge->y * (faceNormal->x * projectionVector.z - faceNormal->z * projectionVector.x) +
         secondEdge->x * (faceNormal->z * projectionVector.y - faceNormal->y * projectionVector.z));
    secondWeight =
        reciprocalDeterminant *
        (firstEdge->z * (faceNormal->x * projectionVector.y - faceNormal->y * projectionVector.x) +
         firstEdge->y * (faceNormal->z * projectionVector.x - faceNormal->x * projectionVector.z) +
         firstEdge->x * (faceNormal->y * projectionVector.z - faceNormal->z * projectionVector.y));
    if (firstWeight > 0.0f && secondWeight > 0.0f && firstWeight + secondWeight > 1.0f) {
        VECSubtract(secondEdge, firstEdge, &projectionVector);
        VECSubtract(relativePoint, firstEdge, &edgeOffset);
        edgeFraction = VECDotProduct(&projectionVector, &edgeOffset) /
                       VECDotProduct(&projectionVector, &projectionVector);
        // These endpoint cases leave the caller's output vector unchanged.
        if (edgeFraction <= 0.0f) {
            closestOffset = firstEdge;
        } else {
            if (edgeFraction >= 1.0f) {
                closestOffset = secondEdge;
            } else {
                VECScale(&projectionVector, &edgeOffset, edgeFraction);
                VECAdd(firstEdge, &edgeOffset, closestOffset);
            }
        }
    } else if (secondWeight < 0.0f) {
        edgeFraction =
            VECDotProduct(firstEdge, relativePoint) / VECDotProduct(firstEdge, firstEdge);
        if (edgeFraction <= 0.0f) {
            HuSetVecF(closestOffset, 0.0, 0.0, 0.0);
        } else {
            if (edgeFraction >= 1.0f) {
                closestOffset = firstEdge;
            } else {
                VECScale(firstEdge, closestOffset, edgeFraction);
            }
        }
    } else if (firstWeight < 0.0f) {
        edgeFraction =
            VECDotProduct(secondEdge, relativePoint) / VECDotProduct(secondEdge, secondEdge);
        if (edgeFraction <= 0.0f) {
            HuSetVecF(closestOffset, 0.0, 0.0, 0.0);
        } else {
            if (edgeFraction >= 1.0f) {
                closestOffset = secondEdge;
            } else {
                VECScale(secondEdge, closestOffset, edgeFraction);
            }
        }
    } else {
        HuSetVecF(closestOffset, firstWeight * firstEdge->x + secondWeight * secondEdge->x,
                  firstWeight * firstEdge->y + secondWeight * secondEdge->y,
                  firstWeight * firstEdge->z + secondWeight * secondEdge->z);
    }
    return 1;
}

// Called by GetPolygonCircleMtx for triangular walls; writes its closest-point candidate even
// without overlap, then returns whether that candidate is within the sphere radius.
BOOL Hitcheck_Triangle_with_Sphere(HuVecF *triangle, HuVecF *sphereCenter, float sphereRadius,
                                   HuVecF *closestPoint) {
    HuVecF triangleOrigin;
    HuVecF firstEdge;
    HuVecF secondEdge;
    HuVecF faceNormal;
    HuVecF relativeCenter;
    HuVecF closestOffset;
    float distance;

    triangleOrigin.x = triangle[0].x;
    triangleOrigin.y = triangle[0].y;
    triangleOrigin.z = triangle[0].z;
    VECSubtract(&triangle[1], &triangle[0], &firstEdge);
    VECSubtract(&triangle[2], &triangle[0], &secondEdge);
    VECCrossProduct(&firstEdge, &secondEdge, &faceNormal);
    VECSubtract(sphereCenter, &triangle[0], &relativeCenter);
    // Some endpoint paths leave closestOffset unwritten; it is not initialized before this call.
    PrecalcPntToTriangle(&triangleOrigin, &firstEdge, &secondEdge, &faceNormal, &relativeCenter,
                         &closestOffset);
    VECAdd(&closestOffset, &triangleOrigin, closestPoint);
    distance = VECDistance(closestPoint, sphereCenter);
    if (distance > sphereRadius) {
        return FALSE;
    } else {
        return TRUE;
    }
}

// Called by GetPolygonCircleMtx for quadrangular walls; evaluates splits (0, 2, 3) and (0, 3, 1),
// writes the nearer candidate even without overlap, and tests that distance against the radius.
BOOL Hitcheck_Quadrangle_with_Sphere(HuVecF *quadrangle, HuVecF *sphereCenter, float sphereRadius,
                                     HuVecF *closestPoint) {
    HuVecF triangleOrigin;
    HuVecF diagonalEdge;
    HuVecF lastEdge;
    HuVecF firstEdge;
    HuVecF faceNormal;
    HuVecF relativeCenter;
    HuVecF closestOffset;
    HuVecF firstClosestPoint;
    HuVecF secondClosestPoint;
    float secondDistance;
    float closestDistance;

    triangleOrigin.x = quadrangle->x;
    triangleOrigin.y = quadrangle->y;
    triangleOrigin.z = quadrangle->z;
    VECSubtract(&quadrangle[2], &quadrangle[0], &diagonalEdge);
    VECSubtract(&quadrangle[3], &quadrangle[0], &lastEdge);
    VECSubtract(&quadrangle[1], &quadrangle[0], &firstEdge);
    VECCrossProduct(&diagonalEdge, &lastEdge, &faceNormal);
    VECSubtract(sphereCenter, &quadrangle[0], &relativeCenter);
    PrecalcPntToTriangle(&triangleOrigin, &diagonalEdge, &lastEdge, &faceNormal, &relativeCenter,
                         &closestOffset);
    VECAdd(&closestOffset, &triangleOrigin, &firstClosestPoint);
    // The first call can leave closestOffset unwritten. This split reuses that offset
    // and the first split's normal; endpoint paths can keep the prior offset.
    PrecalcPntToTriangle(&triangleOrigin, &lastEdge, &firstEdge, &faceNormal, &relativeCenter,
                         &closestOffset);
    VECAdd(&closestOffset, &triangleOrigin, &secondClosestPoint);
    closestDistance = VECDistance(&firstClosestPoint, sphereCenter);
    secondDistance = VECDistance(&secondClosestPoint, sphereCenter);
    if (secondDistance > closestDistance) {
        closestPoint->x = firstClosestPoint.x;
        closestPoint->y = firstClosestPoint.y;
        closestPoint->z = firstClosestPoint.z;
    } else {
        closestDistance = secondDistance;
        closestPoint->x = secondClosestPoint.x;
        closestPoint->y = secondClosestPoint.y;
        closestPoint->z = secondClosestPoint.z;
    }
    if (closestDistance > sphereRadius) {
        return FALSE;
    } else {
        return TRUE;
    }
}

// Called by GetPolygonCircleMtx to store a world contact point before advancing HitFaceCount.
static void DefSetHitFace(float x, float y, float z) {
    HitFace[HitFaceCount].x = x;
    HitFace[HitFaceCount].y = y;
    HitFace[HitFaceCount].z = z;
}

// Called by GetPolygonCircleMtx after overlap; adds a distance-scaled correction along the XZ
// direction.
void AppendAddXZ(float directionX, float directionZ, float distance) {
    HuVecF direction;

    direction.x = directionX;
    direction.y = 0.0f;
    direction.z = directionZ;
    MapspaceInlineFunc00(&direction);
    AddX += direction.x * distance;
    AddZ += direction.z * distance;
}

// Called by MapWall and MapPos to convert a query point into map-object coordinates.
// Builds translation and Z, Y, X rotations in degrees; object scale is not applied.
void CharRotInv(Mtx matrix, Mtx inverseMatrix, HuVecF *point, OMOBJ *object) {
    Mtx rotationMatrix;

    PSMTXTrans(matrix, object->trans.x, object->trans.y, object->trans.z);
    if (object->rot.z) {
        PSMTXRotRad(rotationMatrix, 'z', MTXDegToRad(object->rot.z));
        PSMTXConcat(matrix, rotationMatrix, matrix);
    }
    if (object->rot.y) {
        PSMTXRotRad(rotationMatrix, 'y', MTXDegToRad(object->rot.y));
        PSMTXConcat(matrix, rotationMatrix, matrix);
    }
    if (object->rot.x) {
        PSMTXRotRad(rotationMatrix, 'x', MTXDegToRad(object->rot.x));
        PSMTXConcat(matrix, rotationMatrix, matrix);
    }
    PSMTXInverse(matrix, inverseMatrix);
    PSMTXMultVec(inverseMatrix, point, point);
}
