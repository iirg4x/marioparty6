/* Obstacle course records and rows for Asteroad Rage. */
#ifndef M659_OBJECTS_H
#define M659_OBJECTS_H
#include "REL/m659/collision.h"
typedef struct M659OMRecord {
    HU3D_MODELID model; /* Rendered obstacle model. */
    HuVecF position; /* Obstacle position in world units. */
    HuVecF velocity; /* Collision velocity in world units per frame. */
    HuVecF rotation; /* Obstacle rotation in degrees. */
    float rotationSpeedX, rotationSpeedY; /* Rotation added per frame on each axis. */
    unsigned char unreadObstacleBytes[4]; /* Bytes with no established obstacle behavior in this
                                           * module. */
    M659Collision collision; /* Collision shape and response data for this obstacle. */
    int active; /* Nonzero while activated during play; the result transition hides and disables
                 * it without clearing this flag. */
    float transparencyLevel; /* Model transparency level, from 0.0 to 1.0. */
} M659OMRecord;
typedef struct M659OMRow {
    M659OMRecord *slots[7]; /* Obstacle at each of the seven lanes, or NULL. */
    s16 count; /* Obstacles placed in this row. */
    u16 mask; /* Occupied lane bits, bit 0 through bit 6. */
} M659OMRow;
typedef struct M659OMData {
    M659OMRow rows[22]; /* Course obstacles from front to back. */
    M659OMRecord records[154]; /* Storage for up to 7 obstacles in each row. */
    int recordCount; /* Number of obstacle records allocated. */
    s16 mode; /* Obstacle object update state. */
    float phase; /* Accumulated course scroll phase. */
    s16 playerRow; /* Obstacle row currently tracked for player progress. */
    s16 spawnRow; /* Row index recorded when an obstacle row is activated; not read in this
                   * module. */
} M659OMData;
extern OMOBJ *lbl_1_bss_4C;
extern float lbl_1_data_70[8];
s16 fn_1_33B0(void);
void fn_1_3898(M659OMRecord *record);
void fn_1_38E8(M659OMRecord *record);
void fn_1_39A8(OMOBJ *obj);
void fn_1_4094(OMOBJ *obj);
int fn_1_1840(s16 player);
#endif
