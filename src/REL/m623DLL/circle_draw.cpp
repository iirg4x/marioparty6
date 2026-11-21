// Draws the soft circles registered for the two views.

extern "C" {
#include "dolphin/mtx.h"
#include "dolphin/gx.h"
#include "game/hu3d.h"
#include "dolphin/types.h"
#include "humath.h"
#include <math.h>
}

#include "REL/framework/singleton.h"
#include "REL/framework/model.h"
#include "REL/m623DLL/circle_entry.h"
#include <list>

// Draws the active circle records in an iterator range.
template <class Iterator> void drawRecords(Mtx *matrix, Iterator first, Iterator last);

#include "REL/m623DLL/circle_renderer.h"

// Stores one X and Z offset on the edge of a circle.
struct CircleOffset {
    f32 x;
    f32 z;
};

// The unit-circle perimeter samples for the fans.
CircleOffset lbl_1_bss_688[33];

// Draws the active records as transformed triangle fans.
template <class Iterator> void drawRecords(Mtx *matrix, Iterator first, Iterator last)
{
    s32 index;
    s32 alpha;
    f32 centerX;
    f32 centerY;
    f32 centerZ;
    f32 vertexX;
    f32 vertexY;
    f32 vertexZ;
    f32 scale;

    if (first != last) {
        fn_8002472C();
        GXLoadPosMtxImm(*matrix, GX_PNMTX0);
        GXSetNumTevStages(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                      GX_AF_NONE);
        GXSetNumChans(1);
        GXSetNumTexGens(0);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
        GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
        GXSetCullMode(GX_CULL_NONE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
        GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);

        while (first != last) {
            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 34);
            CircleEntry *record = *first;
            centerZ = record->position.z;
            centerY = record->position.y;
            centerX = record->position.x;
            GXPosition3f32(centerX, centerY, centerZ);
            // The center reaches half opacity and the perimeter stays transparent.
            alpha = (s32) (128.0f * record->alpha);
            GXColor4u8(0x20, 0x20, 0x40, (u8) alpha);
            scale = record->scale;
            for (index = 0; index < 33; ++index) {
                vertexZ = record->position.z + scale * lbl_1_bss_688[index].z;
                vertexY = record->position.y;
                vertexX = record->position.x + scale * lbl_1_bss_688[index].x;
                GXPosition3f32(vertexX, vertexY, vertexZ);
                GXColor4u8(0x20, 0x20, 0x40, 0);
            }
            ++first;
        }
    }
}

// Copies the circle's position.
void CircleEntry::fn_1_D534(const Vec *source)
{
    position.x = source->x;
    position.y = source->y;
    position.z = source->z;
}

// The origin of both circle renderer models.
const Vec lbl_1_rodata_5A0 = { 0.0f, 0.0f, 0.0f };

void fn_1_DC6C(HU3D_MODEL *model, Mtx *matrix);

void fn_1_DD48(HU3D_MODEL *model, Mtx *matrix);

// Stores the record's alpha.
void CircleEntry::setAlpha(f32 opacity)
{
    alpha = opacity;
}

// Stores the record's display scale.
void CircleEntry::setScale(f32 radius)
{
    scale = radius;
}

// Creates the two model hooks and initializes their render resources.
CircleRenderer::CircleRenderer() : cameraOneModel(fn_1_DC6C), cameraTwoModel(fn_1_DD48)
{
    int index;
    f64 angle;

    for (index = 0; index < 33; ++index) {
        angle = (f32) ((f64) index * (M_PI * 360.0 / 180.0) / 32.0 - M_PI);
        lbl_1_bss_688[index].x = (f32) cos(angle);
        lbl_1_bss_688[index].z = (f32) sin(angle);
    }
    cameraOneModel.setHookData(this);
    cameraTwoModel.setHookData(this);
    cameraOneModel.setLayer(4);
    cameraTwoModel.setLayer(4);
    cameraOneModel.setCamera(1);
    cameraTwoModel.setCamera(2);
    cameraOneModel.setPosition(const_cast<Vec *>(&lbl_1_rodata_5A0));
    cameraTwoModel.setPosition(const_cast<Vec *>(&lbl_1_rodata_5A0));
}

// Allocates a record in the list selected by its view.
CircleEntry *fn_1_D940(int viewIndex)
{
    CircleRenderer *self = &CircleRenderer::getChecked();
    std::list<CircleEntry *> &records =
        viewIndex == 0 ? self->cameraOneRecords : self->cameraTwoRecords;
    CircleEntry *record = new CircleEntry;
    // Gives new records their default radius.
    record->setScale(120.0f);
    record->setAlpha(1.0f);
    records.push_back(record);
    return record;
}

// Removes the supplied record from its owning list.
void fn_1_DA2C(CircleEntry *record)
{
    if (record != 0) {
        CircleRenderer *self = &CircleRenderer::getChecked();
        self->cameraOneRecords.remove(record);
        self->cameraTwoRecords.remove(record);
    }
}

// Destroys the renderer and its owned models and lists.
CircleRenderer::~CircleRenderer()
{
}

// Draws the first record list through its model hook.
void fn_1_DC6C(HU3D_MODEL *model, Mtx *matrix)
{
    CircleRenderer *owner = static_cast<CircleRenderer *>(model->hookData);
    drawRecords<std::list<CircleEntry *>::const_iterator>(matrix, owner->cameraOneRecords.begin(),
                                                          owner->cameraOneRecords.end());
}

// Draws the second record list through its model hook.
void fn_1_DD48(HU3D_MODEL *model, Mtx *matrix)
{
    CircleRenderer *owner = static_cast<CircleRenderer *>(model->hookData);
    drawRecords<std::list<CircleEntry *>::const_iterator>(matrix, owner->cameraTwoRecords.begin(),
                                                          owner->cameraTwoRecords.end());
}
