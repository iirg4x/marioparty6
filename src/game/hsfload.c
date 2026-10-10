// Loads HSF model data, resolving file offsets into the model's section and object links.
#define _MATH_H
#include "game/hsfload.h"
#include "string.h"
#include "ctype.h"

GXColor rgba[100];
HSF_HEADER head;
HSF_DATA Model;

static BOOL MotionOnly;
static HSF_DATA *MotionModel;
static void *VertexDataTop;
static void *NormalDataTop;
void *fileptr;
char *StringTable;
char *DicStringTable;
void **NSymIndex;
HSF_OBJECT *objtop;
HSF_BUFFER *vtxtop;
HSF_CLUSTER *ClusterTop;
HSF_ATTRIBUTE *AttributeTop;
HSF_MATERIAL *MaterialTop;

static void FileLoad(void *data);
static HSF_DATA *SetHsfModel(void);
static void MaterialLoad(void);
static void AttributeLoad(void);
static void SceneLoad(void);
static void ColorLoad(void);
static void VertexLoad(void);
static void NormalLoad(void);
static void STLoad(void);
static void FaceLoad(void);
static void ObjectLoad(void);
static void CenvLoad(void);
static void SkeletonLoad(void);
static void PartLoad(void);
static void ClusterLoad(void);
static void ShapeLoad(void);
static void MapAttrLoad(void);
static void PaletteLoad(void);
static void BitmapLoad(void);
static void MotionLoad(void);
static void MatrixLoad(void);

static s32 SearchObjectSetName(HSF_DATA *model, char *name);
static HSF_BUFFER *SearchVertexPtr(s32 tableIndex);
static HSF_BUFFER *SearchNormalPtr(s32 tableIndex);
static HSF_BUFFER *SearchStPtr(s32 tableIndex);
static HSF_BUFFER *SearchColorPtr(s32 tableIndex);
static HSF_BUFFER *SearchFacePtr(s32 tableIndex);
static HSF_CENV *SearchCenvPtr(s32 tableIndex);
static HSF_PART *SearchPartPtr(s32 tableIndex);
static HSF_PALETTE *SearchPalettePtr(s32 tableIndex);

static HSF_BITMAP *SearchBitmapPtr(s32 tableIndex);
static char *GetString(u32 *stringOffset);
static char *GetMotionString(u16 *stringOffset);

// Called by model and motion setup to turn an HSF file image into a usable model.
// The section loaders resolve the file's tables before envelope data is initialized.
HSF_DATA *LoadHSF(void *hsfFile)
{
    HSF_DATA *model;
    Model.root = NULL;
    objtop = NULL;
    FileLoad(hsfFile);
    SceneLoad();
    ColorLoad();
    PaletteLoad();
    BitmapLoad();
    MaterialLoad();
    AttributeLoad();
    VertexLoad();
    NormalLoad();
    STLoad();
    FaceLoad();
    ObjectLoad();
    CenvLoad();
    SkeletonLoad();
    PartLoad();
    ClusterLoad();
    ShapeLoad();
    MapAttrLoad();
    MotionLoad();
    MatrixLoad();
    model = SetHsfModel();
    InitEnvelope(model);
    objtop = NULL;
    return model;

}

// Resolves a motion model's cluster target names against the displayed model.
// HSF manager setup calls this after loading a model and its cluster motion.
void ClusterAdjustObject(HSF_DATA *targetModel, HSF_DATA *sourceModel)
{
    HSF_CLUSTER *cluster;
    s32 i;
    if(!sourceModel) {
        return;
    }
    if(sourceModel->clusterNum == 0) {
        return;
    }
    cluster = sourceModel->cluster;
    if(cluster->adjusted) {
        return;
    }
    cluster->adjusted = 1;
    for(i=0; i<sourceModel->clusterNum; i++, cluster++) {
        char *targetName = cluster->targetName;
        cluster->target = SearchObjectSetName(targetModel, targetName);
    }
}

// Reads the HSF header and establishes pointers to its shared lookup tables.
// LoadHSF calls this before any section-specific loader runs.
static void FileLoad(void *hsfFile)
{
    fileptr = hsfFile;
    memcpy(&head, fileptr, sizeof(HSF_HEADER));
    memset(&Model, 0, sizeof(HSF_DATA));
    NSymIndex = (void **)((u32)fileptr+head.symbol.ofs);
    StringTable = (char *)((u32)fileptr+head.string.ofs);
    ClusterTop = (HSF_CLUSTER *)((u32)fileptr+head.cluster.ofs);
    AttributeTop = (HSF_ATTRIBUTE *)((u32)fileptr+head.attribute.ofs);
    MaterialTop = (HSF_MATERIAL *)((u32)fileptr+head.material.ofs);
}

// Copies the resolved section pointers into the file-backed model descriptor.
// LoadHSF calls this after every section loader has populated Model.
static HSF_DATA *SetHsfModel(void)
{
    HSF_DATA *model = fileptr;
    model->scene = Model.scene;
    model->sceneNum = Model.sceneNum;
    model->attribute = Model.attribute;
    model->attributeNum = Model.attributeNum;
    model->bitmap = Model.bitmap;
    model->bitmapNum = Model.bitmapNum;
    model->cenv = Model.cenv;
    model->cenvNum = Model.cenvNum;
    model->skeleton = Model.skeleton;
    model->skeletonNum = Model.skeletonNum;
    model->face = Model.face;
    model->faceNum = Model.faceNum;
    model->material = Model.material;
    model->materialNum = Model.materialNum;
    model->motion = Model.motion;
    model->motionNum = Model.motionNum;
    model->normal = Model.normal;
    model->normalNum = Model.normalNum;
    model->root = Model.root;
    model->objectNum = Model.objectNum;
    model->object = objtop;
    model->matrix = Model.matrix;
    model->matrixNum = Model.matrixNum;
    model->palette = Model.palette;
    model->paletteNum = Model.paletteNum;
    model->st = Model.st;
    model->stNum = Model.stNum;
    model->vertex = Model.vertex;
    model->vertexNum = Model.vertexNum;
    model->cenv = Model.cenv;
    model->cenvNum = Model.cenvNum;
    model->cluster = Model.cluster;
    model->clusterNum = Model.clusterNum;
    model->part = Model.part;
    model->partNum = Model.partNum;
    model->shape = Model.shape;
    model->shapeNum = Model.shapeNum;
    model->mapAttr = Model.mapAttr;
    model->mapAttrNum = Model.mapAttrNum;
    return model;
}

// Converts a file string-table offset to the corresponding model name.
char *SetName(u32 *stringOffset)
{
    char *name = GetString(stringOffset);
    return name;
}

// Resolves the compact string offset stored in a motion track.
static inline char *SetMotionName(u16 *stringOffset)
{
    char *name = GetMotionString(stringOffset);
    return name;
}

// Resolves material names and symbol-table attribute references in the file.
// LoadHSF calls this before object setup so meshes can use the relocated render materials.
static void MaterialLoad(void)
{
    s32 i;
    s32 j;
    if(head.material.num) {
        HSF_MATERIAL *fileMaterials = (HSF_MATERIAL *)((u32)fileptr+head.material.ofs);
        HSF_MATERIAL *sourceMaterial;
        HSF_MATERIAL *loadedMaterial;
        // The first pass visits each record; relocation happens in the pass below.
        for(i=0; i<head.material.num; i++) {
            sourceMaterial = &fileMaterials[i];
        }
        loadedMaterial = fileMaterials;
        Model.material = loadedMaterial;
        Model.materialNum = head.material.num;
        fileMaterials = (HSF_MATERIAL *)((u32)fileptr+head.material.ofs);
        for(i=0; i<head.material.num; i++, loadedMaterial++) {
            sourceMaterial = &fileMaterials[i];
            loadedMaterial->name = SetName((u32 *)&sourceMaterial->name);
            loadedMaterial->pass = sourceMaterial->pass;
            loadedMaterial->vtxMode = sourceMaterial->vtxMode;
            loadedMaterial->litColor[0] = sourceMaterial->litColor[0];
            loadedMaterial->litColor[1] = sourceMaterial->litColor[1];
            loadedMaterial->litColor[2] = sourceMaterial->litColor[2];
            loadedMaterial->color[0] = sourceMaterial->color[0];
            loadedMaterial->color[1] = sourceMaterial->color[1];
            loadedMaterial->color[2] = sourceMaterial->color[2];
            loadedMaterial->shadowColor[0] = sourceMaterial->shadowColor[0];
            loadedMaterial->shadowColor[1] = sourceMaterial->shadowColor[1];
            loadedMaterial->shadowColor[2] = sourceMaterial->shadowColor[2];
            loadedMaterial->hiliteScale = sourceMaterial->hiliteScale;
            /* HSF_MATERIAL.unk18, HSF_MATERIAL.unk20, HSF_MATERIAL.unk2C and HSF_CLUSTER.unk95 are
             * only copied by the loader and never read by game code, so they keep their offset
             * names. */
            loadedMaterial->unk18 = sourceMaterial->unk18;
            loadedMaterial->invAlpha = sourceMaterial->invAlpha;
            loadedMaterial->unk20[0] = sourceMaterial->unk20[0];
            loadedMaterial->unk20[1] = sourceMaterial->unk20[1];
            loadedMaterial->refAlpha = sourceMaterial->refAlpha;
            loadedMaterial->unk2C = sourceMaterial->unk2C;
            loadedMaterial->attrNum = sourceMaterial->attrNum;
            loadedMaterial->attr = (s32 *)(NSymIndex+((u32)sourceMaterial->attr));
            rgba[i].r = loadedMaterial->litColor[0];
            rgba[i].g = loadedMaterial->litColor[1];
            rgba[i].b = loadedMaterial->litColor[2];
            rgba[i].a = 255;
            for(j=0; j<loadedMaterial->attrNum; j++) {
                // Attribute symbol entries are written back unchanged here.
                loadedMaterial->attr[j] = loadedMaterial->attr[j];
            }
        }
    }
}

// Resolves attribute names and links each attribute to its bitmap record.
// LoadHSF calls this after the bitmap table has been loaded.
static void AttributeLoad(void)
{
    HSF_ATTRIBUTE *fileAttributes;
    HSF_ATTRIBUTE *loadedAttribute;
    HSF_ATTRIBUTE *attributeTable;
    s32 i;
    if(head.attribute.num) {
        attributeTable = fileAttributes = (HSF_ATTRIBUTE *)((u32)fileptr+head.attribute.ofs);
        loadedAttribute = attributeTable;
        Model.attribute = loadedAttribute;
        Model.attributeNum = head.attribute.num;
        for(i=0; i<head.attribute.num; i++, loadedAttribute++) {
            if((u32)fileAttributes[i].name != -1) {
                loadedAttribute->name = SetName((u32 *)&fileAttributes[i].name);
            } else {
                loadedAttribute->name = NULL;
            }
            loadedAttribute->bitmap = SearchBitmapPtr((s32)fileAttributes[i].bitmap);
        }
    }
}

// Registers the scene's fog settings when the HSF file contains a scene record.
// LoadHSF calls this before loading material and geometry tables.
static void SceneLoad(void)
{
    HSF_SCENE *fileScene;
    HSF_SCENE *scene;
    if(head.scene.num) {
        fileScene = (HSF_SCENE *)((u32)fileptr+head.scene.ofs);
        scene = fileScene;
        scene->fogEnd = fileScene->fogEnd;
        scene->fogStart = fileScene->fogStart;
        Model.scene = scene;
        Model.sceneNum = head.scene.num;
    }
}

// Relocates the color table's names and points entries at the packed color data.
// LoadHSF calls this before material loading.
static void ColorLoad(void)
{
    s32 i;
    HSF_BUFFER *fileColors;
    HSF_BUFFER *loadedColor;
    void *colorData;
    u32 colorDataOffset;
    HSF_BUFFER *colorTable;

    if(head.color.num) {
        colorTable = fileColors = (HSF_BUFFER *)((u32)fileptr+head.color.ofs);
        colorData = &fileColors[head.color.num];
        // The table is walked before its base is reset for the relocation pass.
        for(i=0; i<head.color.num; i++, fileColors++);
        loadedColor = colorTable;
        Model.color = loadedColor;
        Model.colorNum = head.color.num;
        fileColors = (HSF_BUFFER *)((u32)fileptr+head.color.ofs);
        colorData = &fileColors[head.color.num];
        for(i=0; i<head.color.num; i++, loadedColor++, fileColors++) {
            colorDataOffset = (u32)fileColors->data;
            loadedColor->name = SetName((u32 *)&fileColors->name);
            loadedColor->data = (void *)((u32)colorData+colorDataOffset);
        }
    }
}

// Resolves vertex-buffer names and points each buffer at its packed 3D positions in the HSF image.
// LoadHSF calls this after attributes and before normals and texture coordinates.
static void VertexLoad(void)
{
    s32 vertexIndex, positionIndex;
    HSF_BUFFER *fileVertices;
    HSF_BUFFER *loadedVertex;
    void *vertexData;
    HuVecF *vertexPosition;
    u32 vertexDataOffset;

    if(head.vertex.num) {
        vtxtop = fileVertices = (HSF_BUFFER *)((u32)fileptr+head.vertex.ofs);
        vertexData = (void *)&fileVertices[head.vertex.num];
        for(vertexIndex=0; vertexIndex<head.vertex.num; vertexIndex++, fileVertices++) {
            for(positionIndex=0; positionIndex<(u32)fileVertices->count; positionIndex++) {
                // This unused pass computes packed-position addresses; the later pass
                // recomputes them and performs a self-copy.
                vertexPosition = (HuVecF *) (((u32) vertexData) + ((u32) fileVertices->data) +
                                             (positionIndex * sizeof(HuVecF)));
            }
        }
        loadedVertex = vtxtop;
        Model.vertex = loadedVertex;
        Model.vertexNum = head.vertex.num;
        fileVertices = (HSF_BUFFER *)((u32)fileptr+head.vertex.ofs);
        VertexDataTop = vertexData = (void *)&fileVertices[head.vertex.num];
        for (vertexIndex = 0; vertexIndex < head.vertex.num;
             vertexIndex++, loadedVertex++, fileVertices++) {
            vertexDataOffset = (u32)fileVertices->data;
            loadedVertex->count = fileVertices->count;
            loadedVertex->name = SetName((u32 *)&fileVertices->name);
            loadedVertex->data = (void *)((u32)vertexData+vertexDataOffset);
            for(positionIndex=0; positionIndex<loadedVertex->count; positionIndex++) {
                vertexPosition = (HuVecF *) (((u32) vertexData) + vertexDataOffset +
                                             (positionIndex * sizeof(HuVecF)));
                HuCopyVecF(&((HuVecF *)loadedVertex->data)[positionIndex], vertexPosition);
            }
        }
    }
}

// Resolves normal-buffer names and points each buffer at its packed vector data.
// LoadHSF calls this immediately after VertexLoad.
static void NormalLoad(void)
{
    s32 normalIndex, positionIndex;
    u32 normalDataOffset;
    HSF_BUFFER *fileNormals;
    HSF_BUFFER *loadedNormal;
    HSF_BUFFER *normalTable;
    void *normalData;

    if(head.normal.num) {
        s32 envelopeCount = head.cenv.num;
        normalTable = fileNormals = (HSF_BUFFER *)((u32)fileptr+head.normal.ofs);
        normalData = (void *)&fileNormals[head.normal.num];
        loadedNormal = normalTable;
        Model.normal = loadedNormal;
        Model.normalNum = head.normal.num;
        fileNormals = (HSF_BUFFER *)((u32)fileptr+head.normal.ofs);
        NormalDataTop = normalData = (void *)&fileNormals[head.normal.num];
        for (normalIndex = 0; normalIndex < head.normal.num;
             normalIndex++, loadedNormal++, fileNormals++) {
            normalDataOffset = (u32)fileNormals->data;
            loadedNormal->count = fileNormals->count;
            loadedNormal->name = SetName((u32 *)&fileNormals->name);
            loadedNormal->data = (void *)((u32)normalData+normalDataOffset);
        }
        // This envelope count is read but does not affect normal-buffer relocation.
    }
}

// Resolves texture-coordinate buffer names and points each buffer at its packed UV pairs
// in the HSF image.
// LoadHSF calls this after normals and before face data.
static void STLoad(void)
{
    s32 bufferIndex, coordinateIndex;
    HSF_BUFFER *fileCoordinates;
    HSF_BUFFER *coordinateTable;
    HSF_BUFFER *loadedCoordinate;
    void *coordinateData;
    HuVec2f *coordinate;
    u32 coordinateDataOffset;

    if(head.st.num) {
        coordinateTable = fileCoordinates = (HSF_BUFFER *)((u32)fileptr+head.st.ofs);
        coordinateData = (void *)&fileCoordinates[head.st.num];
        for(bufferIndex=0; bufferIndex<head.st.num; bufferIndex++, fileCoordinates++) {
            for(coordinateIndex=0; coordinateIndex<(u32)fileCoordinates->count; coordinateIndex++) {
                // This unused pass computes packed-UV addresses; the later pass recomputes them
                // and performs a self-copy.
                coordinate = (HuVec2f *) (((u32) coordinateData) + ((u32) fileCoordinates->data) +
                                          (coordinateIndex * sizeof(HuVec2f)));
            }
        }
        loadedCoordinate = coordinateTable;
        Model.st = loadedCoordinate;
        Model.stNum = head.st.num;
        fileCoordinates = (HSF_BUFFER *)((u32)fileptr+head.st.ofs);
        coordinateData = (void *)&fileCoordinates[head.st.num];
        for (bufferIndex = 0; bufferIndex < head.st.num;
             bufferIndex++, loadedCoordinate++, fileCoordinates++) {
            coordinateDataOffset = (u32)fileCoordinates->data;
            loadedCoordinate->count = fileCoordinates->count;
            loadedCoordinate->name = SetName((u32 *)&fileCoordinates->name);
            loadedCoordinate->data = (void *)((u32)coordinateData+coordinateDataOffset);
            for(coordinateIndex=0; coordinateIndex<loadedCoordinate->count; coordinateIndex++) {
                coordinate = (HuVec2f *) (((u32) coordinateData) + coordinateDataOffset +
                                          (coordinateIndex * sizeof(HuVec2f)));
                HuCopyVec2F(&((HuVec2f *)loadedCoordinate->data)[coordinateIndex], coordinate);
            }
        }
    }
}

// Relocates face records and triangle-strip indices, using the final face record's
// strip-data base for all faces.
// LoadHSF calls this after the vertex, normal, color, and UV tables are ready.
static void FaceLoad(void)
{
    HSF_BUFFER *fileFaces;
    HSF_BUFFER *loadedFace;
    HSF_BUFFER *faceTable;
    u32 faceDataOffset;
    HSF_FACE *faceData;
    HSF_FACE *sourceFace;
    HSF_FACE *loadedFacePart;
    u8 *stripIndexData;
    s32 faceIndex;
    s32 partIndex;

    if(head.face.num) {
        faceTable = fileFaces = (HSF_BUFFER *)((u32)fileptr+head.face.ofs);
        faceData = (HSF_FACE *)&fileFaces[head.face.num];
        loadedFace = faceTable;
        Model.face = loadedFace;
        Model.faceNum = head.face.num;
        fileFaces = (HSF_BUFFER *)((u32)fileptr+head.face.ofs);
        faceData = (HSF_FACE *)&fileFaces[head.face.num];
        for(faceIndex=0; faceIndex<head.face.num; faceIndex++, loadedFace++, fileFaces++) {
            faceDataOffset = (u32)fileFaces->data;
            loadedFace->name = SetName((u32 *)&fileFaces->name);
            loadedFace->count = fileFaces->count;
            loadedFace->data = (void *)((u32)faceData+faceDataOffset);
            stripIndexData = (u8 *)(&((HSF_FACE *)loadedFace->data)[loadedFace->count]);
        }
        loadedFace = faceTable;
        for(faceIndex=0; faceIndex<head.face.num; faceIndex++, loadedFace++) {
            sourceFace = loadedFacePart = loadedFace->data;
            for (partIndex = 0; partIndex < loadedFace->count;
                 partIndex++, loadedFacePart++, sourceFace++) {
                if(sourceFace->typeSrc == HSF_FACE_TRISTRIP) {
                    loadedFacePart->strip.data =
                        (HSF_FACE_INDEX *) (stripIndexData + (u32) sourceFace->strip.data *
                                                                 (sizeof(HSF_FACE_INDEX)));
                }
            }
        }
    }
}

// ObjectLoad calls this recursively to resolve child, mesh, and attachment links in the model tree.
static void DispObject(HSF_OBJECT *parent, HSF_OBJECT *object)
{
    u32 i;
    HSF_OBJECT *childObject;
    HSF_OBJECT *rootCandidate;
    struct {
        HSF_OBJECT *parent;
        HSF_BUFFER *shape;
        HSF_CLUSTER *cluster;
    } resolvedReference;

    resolvedReference.parent = parent;
    object->type = object->type;
    switch(object->type) {
        case HSF_OBJ_MESH:
        {
            HSF_MESH *meshData;
            HSF_OBJECT *resolvedObject;

            meshData = &object->mesh;
            resolvedObject = rootCandidate = object;
            resolvedObject->mesh.childNum = meshData->childNum;
            resolvedObject->mesh.child = (HSF_OBJECT **)&NSymIndex[(u32)meshData->child];
            for(i=0; i<resolvedObject->mesh.childNum; i++) {
                childObject = &objtop[(u32)resolvedObject->mesh.child[i]];
                resolvedObject->mesh.child[i] = childObject;
            }
            resolvedObject->mesh.parent = parent;
            if(Model.root == NULL) {
                Model.root = rootCandidate;
            }
            resolvedObject->type = HSF_OBJ_MESH;
            resolvedObject->mesh.vertex = SearchVertexPtr((s32)meshData->vertex);
            resolvedObject->mesh.normal = SearchNormalPtr((s32)meshData->normal);
            resolvedObject->mesh.st = SearchStPtr((s32)meshData->st);
            resolvedObject->mesh.color = SearchColorPtr((s32)meshData->color);
            resolvedObject->mesh.face = SearchFacePtr((s32)meshData->face);
            resolvedObject->mesh.shape = (HSF_BUFFER **)&NSymIndex[(u32)meshData->shape];
            for(i=0; i<resolvedObject->mesh.shapeNum; i++) {
                resolvedReference.shape = &vtxtop[(u32)resolvedObject->mesh.shape[i]];
                resolvedObject->mesh.shape[i] = resolvedReference.shape;
            }
            resolvedObject->mesh.cluster = (HSF_CLUSTER **)&NSymIndex[(u32)meshData->cluster];
            for(i=0; i<resolvedObject->mesh.clusterNum; i++) {
                resolvedReference.cluster = &ClusterTop[(u32)resolvedObject->mesh.cluster[i]];
                resolvedObject->mesh.cluster[i] = resolvedReference.cluster;
            }
            resolvedObject->mesh.cenv = SearchCenvPtr((s32)meshData->cenv);
            resolvedObject->mesh.material = Model.material;
            if((s32)meshData->attribute >= 0) {
                resolvedObject->mesh.attribute = Model.attribute;
            } else {
                resolvedObject->mesh.attribute = NULL;
            }
            resolvedObject->mesh.vtxtop = (void *)((u32)fileptr+(u32)meshData->vtxtop);
            resolvedObject->mesh.normtop = (void *)((u32)fileptr+(u32)meshData->normtop);
            resolvedObject->mesh.base.pos.x = meshData->base.pos.x;
            resolvedObject->mesh.base.pos.y = meshData->base.pos.y;
            resolvedObject->mesh.base.pos.z = meshData->base.pos.z;
            resolvedObject->mesh.base.rot.x = meshData->base.rot.x;
            resolvedObject->mesh.base.rot.y = meshData->base.rot.y;
            resolvedObject->mesh.base.rot.z = meshData->base.rot.z;
            resolvedObject->mesh.base.scale.x = meshData->base.scale.x;
            resolvedObject->mesh.base.scale.y = meshData->base.scale.y;
            resolvedObject->mesh.base.scale.z = meshData->base.scale.z;
            resolvedObject->mesh.mesh.min.x = meshData->mesh.min.x;
            resolvedObject->mesh.mesh.min.y = meshData->mesh.min.y;
            resolvedObject->mesh.mesh.min.z = meshData->mesh.min.z;
            resolvedObject->mesh.mesh.max.x = meshData->mesh.max.x;
            resolvedObject->mesh.mesh.max.y = meshData->mesh.max.y;
            resolvedObject->mesh.mesh.max.z = meshData->mesh.max.z;
            for(i=0; i<meshData->childNum; i++) {
                DispObject(resolvedObject, resolvedObject->mesh.child[i]);
            }
        }
        break;

        case HSF_OBJ_NULL1:
        {
            HSF_MESH *meshData;
            HSF_OBJECT *resolvedObject;
            meshData = &object->mesh;
            resolvedObject = rootCandidate = object;
            resolvedObject->mesh.parent = parent;
            resolvedObject->mesh.childNum = meshData->childNum;
            resolvedObject->mesh.child = (HSF_OBJECT **)&NSymIndex[(u32)meshData->child];
            for(i=0; i<resolvedObject->mesh.childNum; i++) {
                childObject = &objtop[(u32)resolvedObject->mesh.child[i]];
                resolvedObject->mesh.child[i] = childObject;
            }
            if(Model.root == NULL) {
                Model.root = rootCandidate;
            }
            for(i=0; i<meshData->childNum; i++) {
                DispObject(resolvedObject, resolvedObject->mesh.child[i]);
            }
        }
        break;

        case HSF_OBJ_REPLICA:
        {
            HSF_MESH *meshData;
            HSF_OBJECT *resolvedObject;
            meshData = &object->mesh;
            resolvedObject = rootCandidate = object;
            resolvedObject->mesh.parent = parent;
            resolvedObject->mesh.childNum = meshData->childNum;
            resolvedObject->mesh.child = (HSF_OBJECT **)&NSymIndex[(u32)meshData->child];
            for(i=0; i<resolvedObject->mesh.childNum; i++) {
                childObject = &objtop[(u32)resolvedObject->mesh.child[i]];
                resolvedObject->mesh.child[i] = childObject;
            }
            if(Model.root == NULL) {
                Model.root = rootCandidate;
            }
            resolvedObject->mesh.replica = &objtop[(u32)resolvedObject->mesh.replica];
            for(i=0; i<meshData->childNum; i++) {
                DispObject(resolvedObject, resolvedObject->mesh.child[i]);
            }
        }
        break;

        case HSF_OBJ_ROOT:
        {
            HSF_MESH *meshData;
            HSF_OBJECT *resolvedObject;
            meshData = &object->mesh;
            resolvedObject = rootCandidate = object;
            resolvedObject->mesh.parent = parent;
            resolvedObject->mesh.childNum = meshData->childNum;
            resolvedObject->mesh.child = (HSF_OBJECT **)&NSymIndex[(u32)meshData->child];
            for(i=0; i<resolvedObject->mesh.childNum; i++) {
                childObject = &objtop[(u32)resolvedObject->mesh.child[i]];
                resolvedObject->mesh.child[i] = childObject;
            }
            if(Model.root == NULL) {
                Model.root = rootCandidate;
            }
            for(i=0; i<meshData->childNum; i++) {
                DispObject(resolvedObject, resolvedObject->mesh.child[i]);
            }
        }
        break;

        case HSF_OBJ_JOINT:
        {
            HSF_MESH *meshData;
            HSF_OBJECT *resolvedObject;
            meshData = &object->mesh;
            resolvedObject = rootCandidate = object;
            resolvedObject->mesh.parent = parent;
            resolvedObject->mesh.childNum = meshData->childNum;
            resolvedObject->mesh.child = (HSF_OBJECT **)&NSymIndex[(u32)meshData->child];
            for(i=0; i<resolvedObject->mesh.childNum; i++) {
                childObject = &objtop[(u32)resolvedObject->mesh.child[i]];
                resolvedObject->mesh.child[i] = childObject;
            }
            if(Model.root == NULL) {
                Model.root = rootCandidate;
            }
            for(i=0; i<meshData->childNum; i++) {
                DispObject(resolvedObject, resolvedObject->mesh.child[i]);
            }
        }
        break;

        case HSF_OBJ_NULL2:
        {
            HSF_MESH *meshData;
            HSF_OBJECT *resolvedObject;
            meshData = &object->mesh;
            resolvedObject = rootCandidate = object;
            resolvedObject->mesh.parent = parent;
            resolvedObject->mesh.childNum = meshData->childNum;
            resolvedObject->mesh.child = (HSF_OBJECT **)&NSymIndex[(u32)meshData->child];
            for(i=0; i<resolvedObject->mesh.childNum; i++) {
                childObject = &objtop[(u32)resolvedObject->mesh.child[i]];
                resolvedObject->mesh.child[i] = childObject;
            }
            if(Model.root == NULL) {
                Model.root = rootCandidate;
            }
            for(i=0; i<meshData->childNum; i++) {
                DispObject(resolvedObject, resolvedObject->mesh.child[i]);
            }
        }
        break;

        case HSF_OBJ_MAP:
        {
            HSF_MESH *meshData;
            HSF_OBJECT *resolvedObject;
            meshData = &object->mesh;
            resolvedObject = rootCandidate = object;
            resolvedObject->mesh.parent = parent;
            resolvedObject->mesh.childNum = meshData->childNum;
            resolvedObject->mesh.child = (HSF_OBJECT **)&NSymIndex[(u32)meshData->child];
            for(i=0; i<resolvedObject->mesh.childNum; i++) {
                childObject = &objtop[(u32)resolvedObject->mesh.child[i]];
                resolvedObject->mesh.child[i] = childObject;
            }
            if(Model.root == NULL) {
                Model.root = rootCandidate;
            }
            for(i=0; i<meshData->childNum; i++) {
                DispObject(resolvedObject, resolvedObject->mesh.child[i]);
            }
        }
        break;

        default:
            break;
    }
}

// ObjectLoad calls this to set the type tags for light and camera objects.
static inline void FixupObject(HSF_OBJECT *object)
{
    HSF_LIGHT *light;
    HSF_CAMERA *camera;

    s32 type = object->type;
    switch(type) {
        case HSF_OBJ_LIGHT:
        {
            light = &object->light;
            object->type = HSF_OBJ_LIGHT;
        }
        break;

        case HSF_OBJ_CAMERA:
        {
            camera = &object->camera;
            object->type = HSF_OBJ_CAMERA;
        }
        break;

        default:
            break;

    }
}

// LoadHSF calls this after geometry tables are ready to resolve object links and build the tree.
static void ObjectLoad(void)
{
    s32 i;
    HSF_OBJECT *object;
    HSF_OBJECT *namedObject;

    if(head.object.num) {
        objtop = object = (HSF_OBJECT *)((u32)fileptr+head.object.ofs);
        for(i=0; i<head.object.num; i++, object++) {
            namedObject = object;
            namedObject->name = SetName((u32 *)&object->name);
        }
        object = objtop;
        for(i=0; i<head.object.num; i++, object++) {
            if((s32)object->mesh.parent == -1) {
                break;
            }
        }
        DispObject(NULL, object);
        Model.objectNum = head.object.num;
        object = objtop;
        for(i=0; i<head.object.num; i++, object++) {
            FixupObject(object);
        }
    }
}

// LoadHSF calls this to relocate envelope records and the dual- and multi-influence
// weight arrays.
static void CenvLoad(void)
{
    HSF_CENV_MULTI *multiFile;
    HSF_CENV_MULTI *multiNew;
    HSF_CENV_SINGLE *singleNew;
    HSF_CENV_SINGLE *singleFile;
    HSF_CENV_DUAL *dualFile;
    HSF_CENV_DUAL *dualNew;

    HSF_CENV *cenvNew;
    HSF_CENV *cenvFile;
    void *envelopeDataBase;
    void *envelopeWeightBase;

    s32 j;
    s32 i;

    if(head.cenv.num) {
        cenvFile = (HSF_CENV *)((u32)fileptr+head.cenv.ofs);
        envelopeDataBase = &cenvFile[head.cenv.num];
        envelopeWeightBase = envelopeDataBase;
        cenvNew = cenvFile;
        Model.cenvNum = head.cenv.num;
        Model.cenv = cenvFile;
        for(i=0; i<head.cenv.num; i++) {
            cenvNew[i].singleData =
                (HSF_CENV_SINGLE *) ((u32) cenvFile[i].singleData + (u32) envelopeDataBase);
            cenvNew[i].dualData =
                (HSF_CENV_DUAL *) ((u32) cenvFile[i].dualData + (u32) envelopeDataBase);
            cenvNew[i].multiData =
                (HSF_CENV_MULTI *) ((u32) cenvFile[i].multiData + (u32) envelopeDataBase);
            cenvNew[i].singleCount = cenvFile[i].singleCount;
            cenvNew[i].dualCount = cenvFile[i].dualCount;
            cenvNew[i].multiCount = cenvFile[i].multiCount;
            cenvNew[i].copyCount = cenvFile[i].copyCount;
            cenvNew[i].vtxCount = cenvFile[i].vtxCount;
            envelopeWeightBase = (void *) ((u32) envelopeWeightBase +
                                           (cenvNew[i].singleCount * sizeof(HSF_CENV_SINGLE)));
            envelopeWeightBase = (void *) ((u32) envelopeWeightBase +
                                           (cenvNew[i].dualCount * sizeof(HSF_CENV_DUAL)));
            envelopeWeightBase = (void *) ((u32) envelopeWeightBase +
                                           (cenvNew[i].multiCount * sizeof(HSF_CENV_MULTI)));
        }
        for(i=0; i<head.cenv.num; i++) {
            singleNew = singleFile = cenvNew[i].singleData;
            for(j=0; j<cenvNew[i].singleCount; j++) {
                singleNew[j].target = singleFile[j].target;
                singleNew[j].posNum = singleFile[j].posNum;
                singleNew[j].pos = singleFile[j].pos;
                singleNew[j].normalNum = singleFile[j].normalNum;
                singleNew[j].normal = singleFile[j].normal;

            }
            dualNew = dualFile = cenvNew[i].dualData;
            for(j=0; j<cenvNew[i].dualCount; j++) {
                dualNew[j].target1 = dualFile[j].target1;
                dualNew[j].target2 = dualFile[j].target2;
                dualNew[j].weightNum = dualFile[j].weightNum;
                dualNew[j].weight =
                    (HSF_CENV_DUAL_WEIGHT *) ((u32) envelopeWeightBase + (u32) dualFile[j].weight);
            }
            multiNew = multiFile = cenvNew[i].multiData;
            for(j=0; j<cenvNew[i].multiCount; j++) {
                multiNew[j].weightNum = multiFile[j].weightNum;
                multiNew[j].pos = multiFile[j].pos;
                multiNew[j].posNum = multiFile[j].posNum;
                multiNew[j].normal = multiFile[j].normal;
                multiNew[j].normalNum = multiFile[j].normalNum;
                multiNew[j].weight = (HSF_CENV_MULTI_WEIGHT *) ((u32) envelopeWeightBase +
                                                                (u32) multiFile[j].weight);
            }
            dualNew = dualFile = cenvNew[i].dualData;
            // This pass reads each relocated dual-weight pointer without changing the weights.
            for(j=0; j<cenvNew[i].dualCount; j++) {
                HSF_CENV_DUAL_WEIGHT *dualWeight = dualNew[j].weight;
            }
            multiNew = multiFile = cenvNew[i].multiData;
            // This pass walks each multi-influence weight array without changing its entries.
            for(j=0; j<cenvNew[i].multiCount; j++) {
                HSF_CENV_MULTI_WEIGHT *weight = multiNew[j].weight;
                s32 k;
                for(k=0; k<multiNew[j].weightNum; k++, weight++);
            }
        }
    }
}

// LoadHSF calls this to expose the skeleton table and resolve each skeleton name.
static void SkeletonLoad(void)
{
    HSF_SKELETON *skeletonFile;
    HSF_SKELETON *skeletonNew;
    s32 i;

    if(head.skeleton.num) {
        skeletonNew = skeletonFile = (HSF_SKELETON *)((u32)fileptr+head.skeleton.ofs);
        Model.skeletonNum = head.skeleton.num;
        Model.skeleton = skeletonFile;
        for(i=0; i<head.skeleton.num; i++) {
            skeletonNew[i].name = SetName((u32 *)&skeletonFile[i].name);
            skeletonNew[i].transform.pos.x = skeletonFile[i].transform.pos.x;
            skeletonNew[i].transform.pos.y = skeletonFile[i].transform.pos.y;
            skeletonNew[i].transform.pos.z = skeletonFile[i].transform.pos.z;
            skeletonNew[i].transform.rot.x = skeletonFile[i].transform.rot.x;
            skeletonNew[i].transform.rot.y = skeletonFile[i].transform.rot.y;
            skeletonNew[i].transform.rot.z = skeletonFile[i].transform.rot.z;
            skeletonNew[i].transform.scale.x = skeletonFile[i].transform.scale.x;
            skeletonNew[i].transform.scale.y = skeletonFile[i].transform.scale.y;
            skeletonNew[i].transform.scale.z = skeletonFile[i].transform.scale.z;
        }
    }
}

// LoadHSF calls this to attach each part's vertex-index list to the HSF data block.
static void PartLoad(void)
{
    HSF_PART *partFile;
    HSF_PART *partNew;

    u16 *data;
    s32 i, j;

    if(head.part.num) {
        partNew = partFile = (HSF_PART *)((u32)fileptr+head.part.ofs);
        Model.partNum = head.part.num;
        Model.part = partFile;
        data = (u16 *)&partFile[head.part.num];
        for(i=0; i<head.part.num; i++, partNew++) {
            partNew->name = SetName((u32 *)&partFile[i].name);
            partNew->num = partFile[i].num;
            partNew->vertex = &data[(u32)partFile[i].vertex];
            for(j=0; j<partNew->num; j++) {
                // This pass leaves the packed vertex-index entries unchanged.
                partNew->vertex[j] = partNew->vertex[j];
            }
        }
    }
}

// LoadHSF calls this to resolve cluster names, part links, and vertex-buffer references.
static void ClusterLoad(void)
{
    HSF_CLUSTER *clusterFile;
    HSF_CLUSTER *clusterNew;

    s32 i, j;

    if(head.cluster.num) {
        clusterNew = clusterFile = (HSF_CLUSTER *)((u32)fileptr+head.cluster.ofs);
        Model.clusterNum = head.cluster.num;
        Model.cluster = clusterFile;
        for(i=0; i<head.cluster.num; i++) {
            HSF_BUFFER *vertex;
            u32 vertexSym;
            clusterNew[i].name[0] = SetName((u32 *)&clusterFile[i].name[0]);
            clusterNew[i].name[1] = SetName((u32 *)&clusterFile[i].name[1]);
            clusterNew[i].targetName = SetName((u32 *)&clusterFile[i].targetName);
            clusterNew[i].part = SearchPartPtr((s32)clusterFile[i].part);
            clusterNew[i].unk95 = clusterFile[i].unk95;
            clusterNew[i].type = clusterFile[i].type;
            clusterNew[i].vertexNum = clusterFile[i].vertexNum;
            vertexSym = (u32)clusterFile[i].vertex;
            clusterNew[i].vertex = (HSF_BUFFER **)&NSymIndex[vertexSym];
            for(j=0; j<clusterNew[i].vertexNum; j++) {
                vertex = SearchVertexPtr((s32)clusterNew[i].vertex[j]);
                clusterNew[i].vertex[j] = vertex;
            }
        }
    }
}

// LoadHSF calls this to resolve each shape's vertex-buffer references.
static void ShapeLoad(void)
{
    s32 i, j;
    HSF_SHAPE *shapeNew;
    HSF_SHAPE *shapeFile;

    if(head.shape.num) {
        shapeNew = shapeFile = (HSF_SHAPE *)((u32)fileptr+head.shape.ofs);
        Model.shapeNum = head.shape.num;
        Model.shape = shapeFile;
        for(i=0; i<Model.shapeNum; i++) {
            u32 vertexSym;
            HSF_BUFFER *vertex;

            shapeNew[i].name = SetName((u32 *)&shapeFile[i].name);
            shapeNew[i].num16[0] = shapeFile[i].num16[0];
            shapeNew[i].num16[1] = shapeFile[i].num16[1];
            vertexSym = (u32)shapeFile[i].vertex;
            shapeNew[i].vertex = (HSF_BUFFER **)&NSymIndex[vertexSym];
            for(j=0; j<shapeNew[i].num16[1]; j++) {
                vertex = &vtxtop[(u32)shapeNew[i].vertex[j]];
                shapeNew[i].vertex[j] = vertex;
            }
        }
    }
}

// LoadHSF calls this to point map attributes at their packed 16-bit data.
static void MapAttrLoad(void)
{
    s32 i;
    HSF_MAPATTR *mapAttrBase;
    HSF_MAPATTR *mapAttrFile;
    HSF_MAPATTR *mapAttrNew;
    u16 *data;

    if(head.mapAttr.num) {
        mapAttrFile = mapAttrBase = (HSF_MAPATTR *)((u32)fileptr+head.mapAttr.ofs);
        mapAttrNew = mapAttrBase;
        Model.mapAttrNum = head.mapAttr.num;
        Model.mapAttr = mapAttrBase;
        data = (u16 *)&mapAttrBase[head.mapAttr.num];
        for(i=0; i<head.mapAttr.num; i++, mapAttrFile++, mapAttrNew++) {
            mapAttrNew->data = &data[(u32)mapAttrFile->data];
        }
    }
}

// LoadHSF calls this to resolve bitmap names, palette data, and pixel-data addresses.
static void BitmapLoad(void)
{
    HSF_BITMAP *bitmapFile;
    HSF_BITMAP *bitmapTemp;
    HSF_BITMAP *bitmapNew;
    HSF_PALETTE *palette;
    void *bitmapPixelDataBase;
    s32 i;

    if(head.bitmap.num) {
        bitmapTemp = bitmapFile = (HSF_BITMAP *)((u32)fileptr+head.bitmap.ofs);
        bitmapPixelDataBase = &bitmapFile[head.bitmap.num];
        // Advance to the table end for Model.bitmap; the relocation pass below resets bitmapFile to
        // the table base.
        for(i=0; i<head.bitmap.num; i++, bitmapFile++);
        bitmapNew = bitmapTemp;
        Model.bitmap = bitmapFile;
        Model.bitmapNum = head.bitmap.num;
        bitmapFile = (HSF_BITMAP *)((u32)fileptr+head.bitmap.ofs);
        bitmapPixelDataBase = &bitmapFile[head.bitmap.num];
        for(i=0; i<head.bitmap.num; i++, bitmapFile++, bitmapNew++) {
            bitmapNew->name = SetName((u32 *)&bitmapFile->name);
            bitmapNew->dataFmt = bitmapFile->dataFmt;
            bitmapNew->pixSize = bitmapFile->pixSize;
            bitmapNew->sizeX = bitmapFile->sizeX;
            bitmapNew->sizeY = bitmapFile->sizeY;
            bitmapNew->palSize = bitmapFile->palSize;
            palette = SearchPalettePtr((u32)bitmapFile->palData);
            if(palette) {
                bitmapNew->palData = palette->data;
            }
            bitmapNew->data = (void *)((u32)bitmapPixelDataBase+(u32)bitmapFile->data);
        }
    }
}

// LoadHSF calls this to resolve palette names and point each palette at its packed colors.
static void PaletteLoad(void)
{
    s32 i;
    s32 j;
    HSF_PALETTE *paletteFile;
    HSF_PALETTE *paletteTemp;
    HSF_PALETTE *paletteNew;

    void *dataBase;
    u16 *dataTemp;
    u16 *data;

    if(head.palette.num) {
        paletteTemp = paletteFile = (HSF_PALETTE *)((u32)fileptr+head.palette.ofs);
        dataBase = (u16 *)&paletteFile[head.palette.num];
        // This pass computes packed-color pointers; the relocation pass below recomputes and stores
        // them.
        for(i=0; i<head.palette.num; i++, paletteFile++) {
            dataTemp = (u16 *)((u32)dataBase+(u32)paletteFile->data);
        }
        Model.palette = paletteTemp;
        Model.paletteNum = head.palette.num;
        paletteNew = paletteTemp;
        paletteFile = (HSF_PALETTE *)((u32)fileptr+head.palette.ofs);
        dataBase = (u16 *)&paletteFile[head.palette.num];
        for(i=0; i<head.palette.num; i++, paletteFile++, paletteNew++) {
            dataTemp = (u16 *)((u32)dataBase+(u32)paletteFile->data);
            data = dataTemp;
            paletteNew->name = SetName((u32 *)&paletteFile->name);
            paletteNew->data = data;
            paletteNew->palSize = paletteFile->palSize;
            for(j=0; j<paletteFile->palSize; j++) {
                // This pass leaves the packed palette entries unchanged.
                data[j] = data[j];
            }
        }
    }
}

// Motion name lookup calls this to strip the prefix and trailing object-name suffix.
char *MakeObjectName(s8 *name)
{
    static char buf[768];
    s32 index, numSeparate;
    char *nameP;
    numSeparate = 0;
    index = 0;
    nameP = (char *)name;
    while(*nameP) {
        if(*nameP == '-') {
            name = (s8 *)nameP+1;
            break;
        }
        nameP++;
    }
    while(*name) {
        if(numSeparate != 0) {
            break;
        }
        if(*name == '_' && !isalpha(name[1])) {
            numSeparate++;
            break;
        }
        buf[index] = *name;
        name++;
        index++;
    }
    buf[index] = '\0';
    return buf;
}

// FindObjectName uses this comparison for model names; it returns strcmp's result unchanged.
s32 CmpObjectName(char *name1, char *name2)
{
    s32 temp = 0;
    return strcmp(name1, name2);
}

// Motion loaders call this to read a track name from the optional dictionary or HSF string table.
static inline char *MotionGetName(HSF_TRACK *track)
{
    char *ret;
    if(DicStringTable) {
        ret = &DicStringTable[track->target];
    } else {
        ret = GetMotionString(&track->target);
    }
    return ret;
}

// MotionLoadTransform calls this to map a track target name to the model object-table index.
static inline s32 FindObjectName(char *name)
{
    s32 i;
    HSF_OBJECT *object;

    object = objtop;
    for(i=0; i<head.object.num; i++, object++) {
        if(!CmpObjectName(object->name, name)) {
            return i;
        }
    }
    return -1;
}

// MotionLoadCluster calls this when motion targets clusters in the displayed model.
static inline s32 FindClusterName(char *name)
{
    s32 i;
    HSF_CLUSTER *cluster;

    cluster = ClusterTop;
    for(i=0; i<head.cluster.num; i++, cluster++) {
        if(!strcmp(cluster->name[0], name)) {
            return i;
        }
    }
    return -1;
}

// MotionLoadCluster calls this when the loaded file is itself a motion-only model.
static inline s32 FindMotionClusterName(char *name)
{
    s32 i;
    HSF_CLUSTER *cluster;

    cluster = MotionModel->cluster;
    for(i=0; i<MotionModel->clusterNum; i++, cluster++) {
        if(!strcmp(cluster->name[0], name)) {
            return i;
        }
    }
    return -1;
}

// MotionLoadAttribute calls this to map an attribute track name in the displayed model.
static inline s32 FindAttributeName(char *name)
{
    s32 i;
    HSF_ATTRIBUTE *attribute;

    attribute = AttributeTop;
    for(i=0; i<head.attribute.num; i++, attribute++) {
        if(!attribute->name) {
            continue;
        }
        if(!strcmp(attribute->name, name)) {
            return i;
        }
    }
    return -1;
}

// MotionLoadAttribute calls this when attributes belong to the motion-only model.
static inline s32 FindMotionAttributeName(char *name)
{
    s32 i;
    HSF_ATTRIBUTE *attribute;

    attribute = MotionModel->attribute;
    for(i=0; i<MotionModel->attributeNum; i++, attribute++) {
        if(!attribute->name) {
            continue;
        }
        if(!strcmp(attribute->name, name)) {
            return i;
        }
    }
    return -1;
}

// MotionLoad calls this to resolve object targets and curve data for transform and morph tracks.
static inline void MotionLoadTransform(HSF_TRACK *track, void *data)
{
    float *stepData;
    float *linearData;
    float *bezierData;
    HSF_TRACK *outTrack;
    char *name;
    s32 dataNum;
    outTrack = track;
    name = MotionGetName(track);
    if(objtop) {
        outTrack->target = FindObjectName(name);
    }
    dataNum = track->dataNum;
    switch(track->curveType) {
        case HSF_CURVE_STEP:
        {
            stepData = (float *)((u32)data+(u32)track->data);
            outTrack->data = stepData;
        }
        break;

        case HSF_CURVE_LINEAR:
        {
            linearData = (float *)((u32)data+(u32)track->data);
            outTrack->data = linearData;
        }
        break;

        case HSF_CURVE_BEZIER:
        {
            bezierData = (float *)((u32)data+(u32)track->data);
            outTrack->data = bezierData;
        }
        break;

        case HSF_CURVE_CONST:
            break;
    }
}

// MotionLoad calls this to resolve a cluster target and its step, linear, or Bezier samples.
static inline void MotionLoadCluster(HSF_TRACK *track, void *data)
{
    s32 dataNum;
    float *stepData;
    float *linearData;
    float *bezierData;
    HSF_TRACK *outTrack;
    char *name;

    outTrack = track;
    name = SetMotionName(&track->target);
    if(!MotionOnly) {
        outTrack->cluster = FindClusterName(name);
    } else {
        outTrack->cluster = FindMotionClusterName(name);
    }
    dataNum = track->dataNum;
    (void)outTrack;
    switch(track->curveType) {
        case HSF_CURVE_STEP:
        {
            stepData = (float *)((u32)data+(u32)track->data);
            outTrack->data = stepData;
        }
        break;

        case HSF_CURVE_LINEAR:
        {
            linearData = (float *)((u32)data+(u32)track->data);
            outTrack->data = linearData;
        }
        break;

        case HSF_CURVE_BEZIER:
        {
            bezierData = (float *)((u32)data+(u32)track->data);
            outTrack->data = bezierData;
        }
        break;

        case HSF_CURVE_CONST:
            break;
    }
}

// MotionLoad calls this to resolve cluster-weight track targets and curve samples.
static inline void MotionLoadClusterWeight(HSF_TRACK *track, void *data)
{
    s32 dataNum;
    float *stepData;
    float *linearData;
    float *bezierData;
    HSF_TRACK *outTrack;
    char *name;

    outTrack = track;
    name = SetMotionName(&track->target);
    if(!MotionOnly) {
        outTrack->cluster = FindClusterName(name);
    } else {
        outTrack->cluster = FindMotionClusterName(name);
    }
    dataNum = track->dataNum;
    (void)outTrack;
    switch(track->curveType) {
        case HSF_CURVE_STEP:
        {
            stepData = (float *)((u32)data+(u32)track->data);
            outTrack->data = stepData;
        }
        break;

        case HSF_CURVE_LINEAR:
        {
            linearData = (float *)((u32)data+(u32)track->data);
            outTrack->data = linearData;
        }
        break;

        case HSF_CURVE_BEZIER:
        {
            bezierData = (float *)((u32)data+(u32)track->data);
            outTrack->data = bezierData;
        }
        break;

        case HSF_CURVE_CONST:
            break;
    }
}

// MotionLoad calls this to point material tracks at their step, linear, or Bezier samples.
static inline void MotionLoadMaterial(HSF_TRACK *track, void *data)
{
    float *stepData;
    float *linearData;
    float *bezierData;
    s32 dataNum;
    HSF_TRACK *outTrack;
    outTrack = track;
    dataNum = track->dataNum;
    switch(track->curveType) {
        case HSF_CURVE_STEP:
        {
            stepData = (float *)((u32)data+(u32)track->data);
            outTrack->data = stepData;
        }
        break;

        case HSF_CURVE_LINEAR:
        {
            linearData = (float *)((u32)data+(u32)track->data);
            outTrack->data = linearData;
        }
        break;

        case HSF_CURVE_BEZIER:
        {
            bezierData = (float *)((u32)data+(u32)track->data);
            outTrack->data = bezierData;
        }
        break;

        case HSF_CURVE_CONST:
            break;
    }
}

// MotionLoad calls this to resolve attribute targets when cluster is not -1 and point
// curve or bitmap-key data at its samples.
static inline void MotionLoadAttribute(HSF_TRACK *track, void *data)
{
    HSF_BITMAP_KEY *fileBitmap;
    HSF_BITMAP_KEY *newBitmap;
    s32 i;
    float *stepData;
    float *linearData;
    float *bezierData;
    HSF_TRACK *outTrack;
    char *name;
    outTrack = track;
    if(outTrack->cluster != -1) {
        name = SetMotionName(&track->target);
        if(!MotionOnly) {
            outTrack->attrIdx = FindAttributeName(name);
        } else {
            outTrack->attrIdx = FindMotionAttributeName(name);
        }
    }

    switch(track->curveType) {
        case HSF_CURVE_STEP:
        {
            stepData = (float *)((u32)data+(u32)track->data);
            outTrack->data = stepData;
        }
        break;

        case HSF_CURVE_LINEAR:
        {
            linearData = (float *)((u32)data+(u32)track->data);
            outTrack->data = linearData;
        }
        break;

        case HSF_CURVE_BEZIER:
        {
            bezierData = (float *)((u32)data+(u32)track->data);
            outTrack->data = bezierData;
        }
        break;

        case HSF_CURVE_BITMAP:
        {
            newBitmap = fileBitmap = (HSF_BITMAP_KEY *)((u32)data+(u32)track->data);
            outTrack->data = fileBitmap;
            for(i=0; i<outTrack->numKeyframes; i++, fileBitmap++, newBitmap++) {
                newBitmap->data = SearchBitmapPtr((s32)fileBitmap->data);
            }
        }
        break;
        case HSF_CURVE_CONST:
            break;
    }
}

// LoadHSF calls this after model attributes and materials are available to prepare motion tracks.
static void MotionLoad(void)
{
    HSF_MOTION *fileMotion;
    HSF_MOTION *tempMotion;
    HSF_MOTION *newMotion;
    HSF_TRACK *trackStart;
    void *trackData;
    s32 i;

    MotionOnly = FALSE;
    MotionModel = NULL;
    if(head.motion.num) {
        tempMotion = fileMotion = (HSF_MOTION *)((u32)fileptr+head.motion.ofs);
        newMotion = tempMotion;
        Model.motion = newMotion;
        Model.motionNum = fileMotion->numTracks;
        trackStart = (HSF_TRACK *)&fileMotion[head.motion.num];
        trackData = &trackStart[fileMotion->numTracks];
        newMotion->track = trackStart;
        for(i=0; i<(s32)fileMotion->numTracks; i++) {
            switch(trackStart[i].type) {
                case HSF_TRACK_TRANSFORM:
                case HSF_TRACK_MORPH:
                    MotionLoadTransform(&trackStart[i], trackData);
                    break;

                case HSF_TRACK_CLUSTER:
                    MotionLoadCluster(&trackStart[i], trackData);
                    break;

                case HSF_TRACK_CLUSTER_WEIGHT:
                    MotionLoadClusterWeight(&trackStart[i], trackData);
                    break;

                case HSF_TRACK_MATERIAL:
                    MotionLoadMaterial(&trackStart[i], trackData);
                    break;

                case HSF_TRACK_ATTRIBUTE:
                    MotionLoadAttribute(&trackStart[i], trackData);
                    break;

                default:
                    break;
            }
        }
    }
    // These repeated reads do not alter the loaded motion data or the returned model.
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
    (void)i;
}

// LoadHSF calls this to point the matrix table at the matrix records following its header.
static void MatrixLoad(void)
{
    HSF_MATRIX *matrixFile;

    if(head.matrix.num) {
        matrixFile = (HSF_MATRIX *)((u32)fileptr+head.matrix.ofs);
        matrixFile->data = (Mtx *)((u32)fileptr+head.matrix.ofs+sizeof(HSF_MATRIX));
        Model.matrix = matrixFile;
        Model.matrixNum = head.matrix.num;
    }
}

// ClusterAdjustObject calls this to find a target object by name in a model's object table.
static s32 SearchObjectSetName(HSF_DATA *model, char *name)
{
    HSF_OBJECT *object = model->object;
    s32 objectIndex;
    for(objectIndex=0; objectIndex<model->objectNum; objectIndex++, object++) {
        if(!CmpObjectName(object->name, name)) {
            return objectIndex;
        }
    }
    OSReport("Search Object Error %s\n", name);
    return -1;
}

// DispObject and ClusterLoad call this to resolve a vertex-table index; -1 means no buffer.
static HSF_BUFFER *SearchVertexPtr(s32 tableIndex)
{
    HSF_BUFFER *vertex;
    if(tableIndex == -1) {
        return NULL;
    }
    vertex = (HSF_BUFFER *)((u32)fileptr+head.vertex.ofs);
    vertex += tableIndex;
    return vertex;
}

// DispObject calls this to resolve a normal-table index; -1 means the mesh has no normal buffer.
static HSF_BUFFER *SearchNormalPtr(s32 tableIndex)
{
    HSF_BUFFER *normal;
    if(tableIndex == -1) {
        return NULL;
    }
    normal = (HSF_BUFFER *)((u32)fileptr+head.normal.ofs);
    normal += tableIndex;
    return normal;
}

// DispObject calls this to resolve a texture-coordinate-table index; -1 means no buffer.
static HSF_BUFFER *SearchStPtr(s32 tableIndex)
{
    HSF_BUFFER *st;
    if(tableIndex == -1) {
        return NULL;
    }
    st = (HSF_BUFFER *)((u32)fileptr+head.st.ofs);
    st += tableIndex;
    return st;
}

// DispObject calls this to resolve a color-table index; -1 means the mesh has no color buffer.
static HSF_BUFFER *SearchColorPtr(s32 tableIndex)
{
    HSF_BUFFER *color;
    if(tableIndex == -1) {
        return NULL;
    }
    color = (HSF_BUFFER *)((u32)fileptr+head.color.ofs);
    color += tableIndex;
    return color;
}

// DispObject calls this to resolve a face-table index; -1 means the mesh has no face buffer.
static HSF_BUFFER *SearchFacePtr(s32 tableIndex)
{
    HSF_BUFFER *face;
    if(tableIndex == -1) {
        return NULL;
    }
    face = (HSF_BUFFER *)((u32)fileptr+head.face.ofs);
    face += tableIndex;
    return face;
}

// DispObject calls this to resolve an envelope-table index; -1 means the mesh has no envelope.
static HSF_CENV *SearchCenvPtr(s32 tableIndex)
{
    HSF_CENV *cenv;
    if(tableIndex == -1) {
        return NULL;
    }
    cenv = (HSF_CENV *)((u32)fileptr+head.cenv.ofs);
    cenv += tableIndex;
    return cenv;
}

// ClusterLoad calls this to resolve a cluster's part-table index; -1 means no part is linked.
static HSF_PART *SearchPartPtr(s32 tableIndex)
{
    HSF_PART *part;
    if(tableIndex == -1) {
        return NULL;
    }
    part = (HSF_PART *)((u32)fileptr+head.part.ofs);
    part += tableIndex;
    return part;
}

// BitmapLoad calls this to resolve a bitmap's palette-table index; -1 means no palette is linked.
static HSF_PALETTE *SearchPalettePtr(s32 tableIndex)
{
    HSF_PALETTE *palette;
    if(tableIndex == -1) {
        return NULL;
    }
    palette = Model.palette;
    palette += tableIndex;
    return palette;
}

// MotionLoadAttribute calls this to resolve a bitmap-key index; -1 means no bitmap is linked.
static HSF_BITMAP *SearchBitmapPtr(s32 tableIndex)
{
    HSF_BITMAP *bitmap;
    if(tableIndex == -1) {
        return NULL;
    }
    bitmap = (HSF_BITMAP *)((u32)fileptr+head.bitmap.ofs);
    bitmap += tableIndex;
    return bitmap;
}

// SetName calls this to return a model string at its byte offset in the HSF string table.
static char *GetString(u32 *stringOffset)
{
    char *text = &StringTable[*stringOffset];
    return text;
}

// SetMotionName calls this to return a motion string at its 16-bit offset in the same table.
static char *GetMotionString(u16 *stringOffset)
{
    char *text = &StringTable[*stringOffset];
    return text;
}
