#ifndef _GAME_HSFFORMAT_H
#define _GAME_HSFFORMAT_H

#include "dolphin.h"
#include "humath.h"
#include "game/animdata.h"

//HSF Face Types

//HSF Object types

//HSF Object Flags

//HSF Track Types

//HSF Channel Types

//HSF Curve Types

//HSF Attribute Flags

//HSF Bitmap Formats

typedef float HSF_CONSTANT_KEY[2];
typedef float HSF_LINEAR_KEY[2];
typedef float HSF_BEZIER_KEY[4];

typedef struct HsfS8Vec_s {
    s8 x;
    s8 y;
    s8 z;
} HSF_S8VEC;

typedef struct HsfObject_s HSF_OBJECT;

typedef struct HsfSection_s {
    s32 ofs;
    s32 num;
} HSF_SECTION;

typedef struct hsf_header {
    char magic[8];
    HSF_SECTION scene;
    HSF_SECTION color;
    HSF_SECTION material;
    HSF_SECTION attribute;
    HSF_SECTION vertex;
    HSF_SECTION normal;
    HSF_SECTION st;
    HSF_SECTION face;
    HSF_SECTION object;
    HSF_SECTION bitmap;
    HSF_SECTION palette;
    HSF_SECTION motion;
    HSF_SECTION cenv;
    HSF_SECTION skeleton;
    HSF_SECTION part;
    HSF_SECTION cluster;
    HSF_SECTION shape;
    HSF_SECTION mapAttr;
    HSF_SECTION matrix;
    HSF_SECTION symbol;
    HSF_SECTION string;
} HSF_HEADER;

typedef struct HsfScene_s {
    GXFogType fogType;
    float fogStart;
    float fogEnd;
    GXColor fogColor;
} HSF_SCENE;

typedef struct HsfBitmap_s {
    char *name;
    u32 maxLod;
    u8 dataFmt;
    u8 pixSize;
    s16 sizeX;
    s16 sizeY;
    s16 palSize;
    GXColor tint;
    u16 *palData;
    u32 unk;
    void *data;
} HSF_BITMAP;

typedef struct HsfPalette_s {
    char *name;
    s32 unk;
    u32 palSize;
    u16 *data;
} HSF_PALETTE;

typedef struct HsfAttribute_s {
    char *name;
    void *animWorkP;
    u8 unk8[2];
    u8 colorCombineMode;
    u8 unkB;
    float kColor;
    u8 unk10[4];
    float nbtTpLvl;
    u8 unk18[8];
    float textureEnable;
    u8 unk24[4];
    HuVec2f scale;
    HuVec2f trans;
    u8 unk38[44];
    u32 wrapS;
    u32 wrapT;
    u8 unk6C[12];
    u32 maxLod;
    u32 flag;
    HSF_BITMAP *bitmap;
} HSF_ATTRIBUTE;

typedef struct HsfMaterial_s {
    char *name;
    u8 unk4[4];
    u16 pass;
    u8 vtxMode;
    u8 litColor[3];
    u8 color[3];
    u8 shadowColor[3];
    float hiliteScale;
    float unk18;
    float invAlpha;
    float unk20[2];
    float refAlpha;
    float unk2C;
    u32 flags;
    u32 attrNum;
    s32 *attr;
} HSF_MATERIAL;

typedef struct HsfBuffer_s {
    char *name;
    s32 count;
    union {
        void *data;
        HuVecF *positions;
        HuVec2f *coordinates;
    };
} HSF_BUFFER;

typedef struct HsfFaceIndex_s {
    s16 vertex;
    s16 normal;
    s16 color;
    s16 st;
} HSF_FACE_INDEX;

typedef struct HsfFace_s {
    union {
        s16 type;
        u16 typeSrc;
    };
    s16 mat;
    union {
        struct {
            HSF_FACE_INDEX index[3];
            u32 count;
            HSF_FACE_INDEX *data;
        } strip;
        HSF_FACE_INDEX index[4];
    };
    float nbt[3];
} HSF_FACE;

typedef struct HsfTransform_s {
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
} HSF_TRANSFORM;

typedef struct HsfCenvSingle_s {
    u32 target;
    u16 pos;
    u16 posNum;
    u16 normal;
    u16 normalNum;
} HSF_CENV_SINGLE;

typedef struct HsfCenvDualWeight_s {
    float weight;
    u16 pos;
    u16 posNum;
    u16 normal;
    u16 normalNum;
} HSF_CENV_DUAL_WEIGHT;

typedef struct HsfCenvDual_s {
    u32 target1;
    u32 target2;
    u32 weightNum;
    HSF_CENV_DUAL_WEIGHT *weight;
} HSF_CENV_DUAL;

typedef struct HsfCenvMultiWeight_s {
    u32 target;
    float value;
} HSF_CENV_MULTI_WEIGHT;

typedef struct HsfCenvMulti_s {
    u32 weightNum;
    u16 pos;
    u16 posNum;
    u16 normal;
    u16 normalNum;
    HSF_CENV_MULTI_WEIGHT *weight;
} HSF_CENV_MULTI;

typedef struct HsfCenv_s {
    char *name;
    HSF_CENV_SINGLE *singleData;
    HSF_CENV_DUAL *dualData;
    HSF_CENV_MULTI *multiData;
    u32 singleCount;
    u32 dualCount;
    u32 multiCount;
    u32 vtxCount;
    u32 copyCount;
} HSF_CENV;

typedef struct HsfPart_s {
    char *name;
    u32 num;
    u16 *vertex;
} HSF_PART;

typedef struct HsfCluster_s {
    char *name[2];
    union {
        char *targetName;
        s32 target;
    };
    HSF_PART *part;
    float index;
    float weight[32];
    u8 adjusted;
    u8 unk95;
    u16 type;
    u32 vertexNum;
    HSF_BUFFER **vertex;
} HSF_CLUSTER;

typedef struct HsfShape_s {
    char *name;
    union {
        u16 num16[2];
        u32 vertexNum;
    };
    HSF_BUFFER **vertex;
} HSF_SHAPE;

typedef struct HsfMesh_s {
    HSF_OBJECT *parent;
    u32 childNum;
    HSF_OBJECT **child;
    HSF_TRANSFORM base;
    HSF_TRANSFORM curr;
    union {
        struct {
            HuVecF min;
            HuVecF max;
            float baseMorph;
            float morphWeight[33];
        } mesh;
        HSF_OBJECT *replica;
    };
    
    HSF_BUFFER *face;
    HSF_BUFFER *vertex;
    HSF_BUFFER *normal;
    HSF_BUFFER *color;
    HSF_BUFFER *st;
    HSF_MATERIAL *material;
    HSF_ATTRIBUTE *attribute;
    u8 writeNum;
    u8 unk121;
    u8 shapeType;
    u8 matPass;
    u32 shapeNum;
    HSF_BUFFER **shape;
    u32 clusterNum;
    HSF_CLUSTER **cluster;
    u32 cenvNum;
    HSF_CENV *cenv;
    HuVecF *vtxtop;
    HuVecF *normtop;
} HSF_MESH;

typedef struct hsf_camera {
    HuVecF pos;
    HuVecF target;
    float upRot;
    float fov;
    float near;
    float far;
} HSF_CAMERA;

typedef struct hsf_light {
    HuVecF pos;
    HuVecF target;
    u8 type;
    u8 r;
    u8 g;
    u8 b;
    float unk2C;
    float refDistance;
    float refBrightness;
    float cutoff;
} HSF_LIGHT;

typedef struct HsfObject_s {
    char *name;
    u32 type;
    void *constData;
    u32 flags;
    union {
        HSF_MESH mesh;
        HSF_CAMERA camera;
        HSF_LIGHT light;
    };
} HSF_OBJECT;

typedef struct HsfSkeleton_s {
    char *name;
    HSF_TRANSFORM transform;
} HSF_SKELETON;

typedef struct HsfBitmapKey_s {
    float time;
    HSF_BITMAP *data;
} HSF_BITMAP_KEY;

typedef struct HsfTrack_s {
    u8 type;
    u8 start;
    union {
        u16 target;
        s16 cluster;
    };
    union {
        s32 clusterWeight;
        struct {
            union {
                s16 attrIdx;
                u16 index;
            };
            union {
                u16 channel;
                s16 morphWeight;
            };
        };
    };
    u16 curveType;
    union {
        u16 numKeyframes;
        s16 dataNum;
    };
    
    union {
        float value;
        void *data;
    };
} HSF_TRACK;

typedef struct HsfMotion_s {
    char *name;
    s32 numTracks;
    HSF_TRACK *track;
    float maxTime;
} HSF_MOTION;

typedef struct HsfMapAttr_s {
    float minX;
    float minZ;
    float maxX;
    float maxZ;
    u16 *data;
    u32 dataLen;
} HSF_MAPATTR;

typedef struct HsfMatrix_s {
    u32 base_idx;
    u32 count;
    Mtx *data;
} HSF_MATRIX;

typedef struct HsfData_s {
    u8 magic[8];
    HSF_SCENE *scene;
    HSF_ATTRIBUTE *attribute;
    HSF_MATERIAL *material;
    HSF_BUFFER *vertex;
    HSF_BUFFER *normal;
    HSF_BUFFER *st;
    HSF_BUFFER *color;
    HSF_BUFFER *face;
    HSF_BITMAP *bitmap;
    HSF_PALETTE *palette;
    HSF_OBJECT *root;
    HSF_CENV *cenv;
    HSF_SKELETON *skeleton;
    HSF_CLUSTER *cluster;
    HSF_PART *part;
    HSF_SHAPE *shape;
    HSF_MOTION *motion;
    HSF_OBJECT *object;
    HSF_MAPATTR *mapAttr;
    HSF_MATRIX *matrix;
    s16 sceneNum;
    s16 attributeNum;
    s16 materialNum;
    s16 vertexNum;
    s16 normalNum;
    s16 stNum;
    s16 colorNum;
    s16 faceNum;
    s16 bitmapNum;
    s16 paletteNum;
    s16 objectNum;
    s16 cenvNum;
    s16 skeletonNum;
    s16 clusterNum;
    s16 partNum;
    s16 shapeNum;
    s16 mapAttrNum;
    s16 motionNum;
    s16 matrixNum;
} HSF_DATA;

#endif
