/* Manages HSF texture animation, texture scrolling, particles, water rendering, and model
 * lifetimes. */
#define _MATH_H
#define M_PI 3.141592653589793
#define PARTICLE_DISPLAY_LIST_MAX_SIZE 131072
double sin(double x);
double cos(double x);
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/init.h"
#include "game/main.h"

#include "string.h"

HU3D_TEXANIM Hu3DTexAnimData[HU3D_TEXANIM_MAX];
HU3D_TEXSCROLL Hu3DTexScrData[HU3D_TEXSCROLL_MAX];

/* Called by Hu3DInit to mark every texture-animation and texture-scroll slot free. */
void Hu3DAnimInit(void)
{
    s16 slotIndex;
    HU3D_TEXANIM *texAnimP;
    HU3D_TEXSCROLL *texScrP;
    for (texAnimP = &Hu3DTexAnimData[0], slotIndex = 0; slotIndex < HU3D_TEXANIM_MAX;
         slotIndex++, texAnimP++) {
        texAnimP->modelId = HU3D_MODELID_NONE;
    }
    for (texScrP = &Hu3DTexScrData[0], slotIndex = 0; slotIndex < HU3D_TEXSCROLL_MAX;
         slotIndex++, texScrP++) {
        texScrP->modelId = HU3D_MODELID_NONE;
    }
}

/* Attaches a sprite animation to every same-named HSF bitmap on the model. */
HU3D_ANIMID Hu3DAnimCreate(void *animData, HU3D_MODELID modelId, char *bmpName)
{
    HU3D_TEXANIM *texAnimEntry;
    HU3D_ANIMID animId;

    HSF_DATA *modelHsf;
    HSF_ATTRIBUTE *attribute;
    s16 attributeIndex;
    s16 matchingBitmapCount;

    for (texAnimEntry = &Hu3DTexAnimData[0], animId = 0; animId < HU3D_TEXANIM_MAX;
         animId++, texAnimEntry++) {
        if(texAnimEntry->modelId == HU3D_MODELID_NONE) {
            break;
        }
    }
    if(animId == HU3D_TEXANIM_MAX) {
        OSReport("Error: TexAnim Over\n");
        return HU3D_ANIMID_NONE;
    }
    modelHsf = Hu3DData[modelId].hsf;
    for (attribute = modelHsf->attribute, attributeIndex = matchingBitmapCount = 0;
         attributeIndex < modelHsf->attributeNum; attributeIndex++, attribute++) {
        if(strcmp(bmpName, attribute->bitmap->name) == 0) {
            HU3D_ATTR_ANIM *attributeAnim;
            if(!attribute->animWorkP) {
                attributeAnim = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3D_ATTR_ANIM),
                                                     Hu3DData[modelId].mallocNo);
                attribute->animWorkP = attributeAnim;
                attributeAnim->attr = HU3D_ATTRANIM_ATTR_NONE;
            } else {
                attributeAnim = attribute->animWorkP;
                if ((attributeAnim->attr & HU3D_ATTRANIM_ATTR_ANIM2D) &&
                    Hu3DTexAnimData[attributeAnim->animId].modelId != HU3D_MODELID_NONE) {
                    /* HU3D_MODEL_MAX makes Hu3DAnimKill skip this slot's model-attribute scan. */
                    Hu3DTexAnimData[attributeAnim->animId].modelId = HU3D_MODEL_MAX;
                }
            }
            attributeAnim->attr |= HU3D_ATTRANIM_ATTR_ANIM2D;
            attributeAnim->animId = animId;
            attributeAnim->scale.x = attributeAnim->scale.y = 1;
            attributeAnim->trans.x = attributeAnim->trans.y = 0;
            matchingBitmapCount++;
        }
    }
    if(matchingBitmapCount == 0) {
        OSReport("Error: Not Found TexAnim Name\n");
        return HU3D_ANIMID_NONE;
    }
    if(!animData) {
        texAnimEntry->anim = NULL;
    } else {
        /* Already relocated input gains one reference in HuSprAnimRead and another below. */
        texAnimEntry->anim = HuSprAnimRead(animData);
        texAnimEntry->anim->useNum++;
    }
    texAnimEntry->modelId = modelId;
    texAnimEntry->time = 0;
    texAnimEntry->bank = 0;
    texAnimEntry->anmNo = 0;
    texAnimEntry->attr = HU3D_ANIM_ATTR_NONE;
    texAnimEntry->speed = 1;
    return animId;
}

/* Returns NULL for unused slots or slots without animation data. Otherwise replaces the
* animation, returns its previous data, and resets bank, frame, time, flags, and speed. */
ANIMDATA *Hu3DAnimAnimSet(HU3D_ANIMID animId, ANIMDATA *newAnim)
{
    HU3D_TEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    if(texAnimP->modelId == HU3D_MODELID_NONE || !texAnimP->anim) {
        return NULL;
    } else {
        ANIMDATA *previousAnim = texAnimP->anim;
        previousAnim->useNum--;
        texAnimP->anim = newAnim;
        texAnimP->anim->useNum++;
        texAnimP->time = 0;
        texAnimP->bank = 0;
        texAnimP->anmNo = 0;
        texAnimP->attr = 0;
        texAnimP->speed = 1;
        return previousAnim;
    }
}

/* Shares a sprite animation with every same-named HSF bitmap on another model. */
HU3D_ANIMID Hu3DAnimLink(HU3D_ANIMID linkAnimId, HU3D_MODELID modelId, char *bmpName)
{
    HU3D_TEXANIM *texAnimEntry;
    HU3D_ANIMID animId;

    HSF_DATA *modelHsf;
    HSF_ATTRIBUTE *attribute;
    s16 attributeIndex;
    s16 matchingBitmapCount;

    HU3D_TEXANIM *sourceAnimEntry = &Hu3DTexAnimData[linkAnimId];

    for (texAnimEntry = &Hu3DTexAnimData[0], animId = 0; animId < HU3D_TEXANIM_MAX;
         animId++, texAnimEntry++) {
        if(texAnimEntry->modelId == HU3D_MODELID_NONE) {
            break;
        }
    }
    if(animId == HU3D_TEXANIM_MAX) {
        OSReport("Error: TexAnim Over\n");
        return HU3D_ANIMID_NONE;
    }
    modelHsf = Hu3DData[modelId].hsf;
    for (attribute = modelHsf->attribute, attributeIndex = matchingBitmapCount = 0;
         attributeIndex < modelHsf->attributeNum; attributeIndex++, attribute++) {
        if(strcmp(bmpName, attribute->bitmap->name) == 0) {
            HU3D_ATTR_ANIM *attributeAnim;
            if(!attribute->animWorkP) {
                attributeAnim = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3D_ATTR_ANIM),
                                                     Hu3DData[modelId].mallocNo);
                attribute->animWorkP = attributeAnim;
                attributeAnim->attr = HU3D_ATTRANIM_ATTR_NONE;
            } else {
                attributeAnim = attribute->animWorkP;
                if ((attributeAnim->attr & HU3D_ATTRANIM_ATTR_ANIM2D) &&
                    Hu3DTexAnimData[attributeAnim->animId].modelId != HU3D_MODELID_NONE) {
                    /* HU3D_MODEL_MAX makes Hu3DAnimKill skip this slot's model-attribute scan. */
                    Hu3DTexAnimData[attributeAnim->animId].modelId = HU3D_MODEL_MAX;
                }
            }
            attributeAnim->attr |= HU3D_ATTRANIM_ATTR_ANIM2D;
            attributeAnim->animId = animId;
            attributeAnim->scale.x = attributeAnim->scale.y = 1;
            attributeAnim->trans.x = attributeAnim->trans.y = 0;
            matchingBitmapCount++;
        }
    }
    if(matchingBitmapCount == 0) {
        OSReport("Error: Not Found TexAnim Name\n");
        return HU3D_ANIMID_NONE;
    }
    texAnimEntry->anim = sourceAnimEntry->anim;
    texAnimEntry->anim->useNum++;
    texAnimEntry->modelId = modelId;
    texAnimEntry->time = 0;
    texAnimEntry->bank = 0;
    texAnimEntry->anmNo = 0;
    texAnimEntry->attr = HU3D_ANIM_ATTR_NONE;
    texAnimEntry->speed = 1;
    return animId;
}

/* Called by model teardown or clients to detach a texture animation and release its data
 * reference. */
void Hu3DAnimKill(HU3D_ANIMID animId)
{
    HU3D_TEXANIM *texAnimEntry = &Hu3DTexAnimData[animId];
    if(texAnimEntry->modelId != HU3D_MODEL_MAX) {
        HSF_DATA *modelHsf = Hu3DData[texAnimEntry->modelId].hsf;
        if(modelHsf) {
            HSF_ATTRIBUTE *attribute;
            s16 attributeIndex;
            for (attribute = modelHsf->attribute, attributeIndex = 0;
                 attributeIndex < modelHsf->attributeNum; attributeIndex++, attribute++) {
                if(attribute->animWorkP) {
                    HU3D_ATTR_ANIM *attributeAnim = attribute->animWorkP;
                    if(attributeAnim->animId == animId) {
                        attributeAnim->attr &= ~HU3D_ATTRANIM_ATTR_ANIM2D;
                        if(attributeAnim->attr == HU3D_ATTRANIM_ATTR_NONE) {
                            attribute->animWorkP = NULL;
                            HuMemDirectFree(attributeAnim);
                        }
                    }
                }
            }
        }
    }
    texAnimEntry->modelId = HU3D_MODELID_NONE;
    if(--texAnimEntry->anim->useNum <= 0) {
        HuMemDirectFree(texAnimEntry->anim);
    }
}

/* Called when a model is destroyed to release each texture animation attached to it. */
void Hu3DAnimModelKill(HU3D_MODELID modelId)
{
    HU3D_TEXANIM *texAnimEntry;
    HU3D_ANIMID animId;
    for (texAnimEntry = &Hu3DTexAnimData[0], animId = 0; animId < HU3D_TEXANIM_MAX;
         animId++, texAnimEntry++) {
        if(texAnimEntry->modelId == modelId) {
            Hu3DAnimKill(animId);
        }
    }
}

/* Called during global 3D cleanup to release all texture animations and scrolling. */
void Hu3DAnimAllKill(void)
{
    HU3D_TEXANIM *texAnimEntry;
    HU3D_ANIMID animId;
    for (texAnimEntry = &Hu3DTexAnimData[0], animId = 0; animId < HU3D_TEXANIM_MAX;
         animId++, texAnimEntry++) {
        if(texAnimEntry->modelId != HU3D_MODELID_NONE) {
            Hu3DAnimKill(animId);
        }
    }
    Hu3DTexScrollAllKill();
}

/* Called by animation clients to add playback flags to a texture-animation slot. */
void Hu3DAnimAttrSet(HU3D_ANIMID animId, u16 attr)
{
    HU3D_TEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    texAnimP->attr |= attr;
}

/* Called by animation clients to clear selected playback flags from a slot. */
void Hu3DAnimAttrReset(HU3D_ANIMID animId, u16 attr)
{
    HU3D_TEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    texAnimP->attr &= ~attr;
}

/* Called by animation clients to set the slot's playback speed multiplier. */
void Hu3DAnimSpeedSet(HU3D_ANIMID animId, float speed)
{
    HU3D_TEXANIM *texAnimP = &Hu3DTexAnimData[animId];

    texAnimP->speed = speed;
}

/* Called by animation clients to select a valid bank and restart at its first frame. */
void Hu3DAnimBankSet(HU3D_ANIMID animId, u16 bank)
{
    HU3D_TEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    if(texAnimP->anim->bankNum <= bank) {
        OSReport("Error: Hu3DAnimBankSet() BankNo Error\n");
        return;
    }
    texAnimP->bank = bank;
    texAnimP->anmNo = texAnimP->time = 0;
}

/* Called by animation clients to select a frame and clear elapsed time within that frame. */
void Hu3DAnmNoSet(HU3D_ANIMID animId, u16 anmNo)
{
    HU3D_TEXANIM *texAnimP = &Hu3DTexAnimData[animId];
    texAnimP->anmNo = anmNo;
    texAnimP->time = 0;
}

/* Called while drawing an HSF attribute to load its current animated texture layer. */
s32 Hu3DAnimSet(HU3D_MODEL *model, HSF_ATTRIBUTE *attribute, s16 textureSlot)
{
    ANIMPAT *pattern;
    HU3D_ATTR_ANIM *attributeAnim = attribute->animWorkP;
    HU3D_TEXANIM *texAnimEntry = &Hu3DTexAnimData[attributeAnim->animId];
    ANIMDATA *animData = texAnimEntry->anim;
    s16 patternNo = animData->bank[texAnimEntry->bank].frame[texAnimEntry->anmNo].pat;
    ANIMLAYER *patternLayer;
    ANIMBMP *bitmap;
    if(patternNo == -1) {
        return FALSE;
    } else {
        s16 wrapS = (attribute->wrapS == TRUE) ? TRUE : FALSE;
        s16 wrapT = (attribute->wrapT == TRUE) ? TRUE : FALSE;
        pattern = &animData->pat[patternNo];
        patternLayer = &pattern->layer[0];
        bitmap = &animData->bmp[patternLayer->bmpNo];
        HuSprTexLoad(texAnimEntry->anim, patternLayer->bmpNo, textureSlot, wrapS, wrapT,
                     (model->attr & HU3D_ATTR_TEX_NEAR) ? GX_NEAR : GX_LINEAR);
        attributeAnim->scale.x = (float)patternLayer->sizeX/bitmap->sizeX;
        attributeAnim->scale.y = (float)patternLayer->sizeY/bitmap->sizeY;
        attributeAnim->trans.x = (float)patternLayer->startX/bitmap->sizeX;
        attributeAnim->trans.y = (float)patternLayer->startY/bitmap->sizeY;

        return TRUE;
    }
}

/* Called once per Hu3D frame after sprite execution to advance animation and texture-scroll
 * state. */
void Hu3DAnimExec(void)
{
    HU3D_TEXSCROLL *scrollEntry;
    HU3D_TEXANIM *animEntry;
    s16 slotIndex;
    for (animEntry = &Hu3DTexAnimData[0], slotIndex = 0; slotIndex < HU3D_TEXANIM_MAX;
         slotIndex++, animEntry++) {
        if(animEntry->modelId == HU3D_MODELID_NONE) {
            continue;
        }
        /* HU3D_ANIM_ATTR_PAUSE lets this slot advance while the global 3D pause is active. */
        if(Hu3DPauseF == FALSE || (animEntry->attr & HU3D_ANIM_ATTR_PAUSE)) {
            ANIMDATA *animData = animEntry->anim;
            ANIMBANK *animBank = &animData->bank[animEntry->bank];
            ANIMFRAME *animFrame = &animBank->frame[animEntry->anmNo];
            /* ANIMON normally stops advancement; a current time -1 frame bypasses it only with
             * LOOP set. At an upcoming sentinel, LOOP holds the preceding frame; otherwise
             * playback restarts. */
            if(!(animEntry->attr & HU3D_ANIM_ATTR_ANIMON)
             || (animFrame->time == -1 && (animEntry->attr & HU3D_ANIM_ATTR_LOOP))) {
                s16 wholeFrameSteps;
                for (wholeFrameSteps = 0; wholeFrameSteps < (int) animEntry->speed * minimumVcount;
                     wholeFrameSteps++) {
                    animEntry->time++;
                    if(animEntry->time >= animFrame->time) {
                        animEntry->anmNo++;
                        animEntry->time -= animFrame->time;
                        if(animEntry->anmNo >= animBank->timeNum) {
                            animEntry->anmNo--;
                        } else if(animFrame[1].time == -1) {
                            if(animEntry->attr & HU3D_ANIM_ATTR_LOOP) {
                                animEntry->anmNo--;
                            } else {
                                animEntry->anmNo = 0;
                            }
                        }
                        animFrame = &animBank->frame[animEntry->anmNo];
                    }
                }
                animEntry->time += (animEntry->speed*minimumVcount)-wholeFrameSteps;
                if(animEntry->time >= animFrame->time) {
                    animEntry->anmNo++;
                    animEntry->time -= animFrame->time;
                    if(animEntry->anmNo >= animBank->timeNum) {
                        animEntry->anmNo--;
                    } else if(animFrame[1].time == -1) {
                        if(animEntry->attr & HU3D_ANIM_ATTR_LOOP) {
                            animEntry->anmNo--;
                        } else {
                            animEntry->anmNo = 0;
                        }
                    }
                }
            }
        }
    }
    for (scrollEntry = &Hu3DTexScrData[0], slotIndex = 0; slotIndex < HU3D_TEXSCROLL_MAX;
         slotIndex++, scrollEntry++) {
        if(scrollEntry->modelId == HU3D_MODELID_NONE) {
            continue;
        }
        if(Hu3DPauseF && !(scrollEntry->attr & HU3D_TEXSCR_ATTR_PAUSEDISABLE)) {
            MTXRotDeg(scrollEntry->texMtx, 'Z', scrollEntry->rot);
            mtxTransCat(scrollEntry->texMtx, scrollEntry->pos.x, scrollEntry->pos.y,
                        scrollEntry->pos.z);
        } else {
            if(scrollEntry->attr & HU3D_TEXSCR_ATTR_POSMOVE) {
                VECAdd(&scrollEntry->pos, &scrollEntry->posMove, &scrollEntry->pos);
                if(scrollEntry->pos.x > 1.0f) {
                    scrollEntry->pos.x -= 1.0f;
                }
                if(scrollEntry->pos.y > 1.0f) {
                    scrollEntry->pos.y -= 1.0f;
                }
                if(scrollEntry->pos.z > 1.0f) {
                    scrollEntry->pos.z -= 1.0f;
                }
                if(scrollEntry->pos.x < -1.0f) {
                    scrollEntry->pos.x += 1.0f;
                }
                if(scrollEntry->pos.y < -1.0f) {
                    scrollEntry->pos.y += 1.0f;
                }
                if(scrollEntry->pos.z < -1.0f) {
                    scrollEntry->pos.z += 1.0f;
                }
            }
            if(scrollEntry->attr & HU3D_TEXSCR_ATTR_ROTMOVE) {
                scrollEntry->rot += scrollEntry->rotMove;
                if(scrollEntry->rot > 360.0f) {
                    scrollEntry->rot -= 360.0f;
                }
                if(scrollEntry->rot < -360.0f) {
                    scrollEntry->rot += 360.0f;
                }
            }
            MTXRotDeg(scrollEntry->texMtx, 'Z', scrollEntry->rot);
            mtxTransCat(scrollEntry->texMtx, scrollEntry->pos.x, scrollEntry->pos.y,
                        scrollEntry->pos.z);
        }
    }
}

/* Attaches a scrolling texture matrix to same-named HSF bitmaps on the model. */
HU3D_TEXSCRID Hu3DTexScrollCreate(HU3D_MODELID modelId, char *bmpName)
{
    HU3D_TEXSCROLL *scrollEntry;
    HU3D_TEXSCRID texScrId;

    HSF_DATA *modelHsf;
    HSF_ATTRIBUTE *attribute;
    s16 attributeIndex;
    s16 matchingBitmapCount;

    for (scrollEntry = &Hu3DTexScrData[0], texScrId = 0; texScrId < HU3D_TEXSCROLL_MAX;
         texScrId++, scrollEntry++) {
        if(scrollEntry->modelId == HU3D_MODELID_NONE) {
            break;
        }
    }
    if(texScrId == HU3D_TEXSCROLL_MAX) {
        OSReport("Error: TexScroll Over\n");
        return HU3D_TEXSCRID_NONE;
    }
    modelHsf = Hu3DData[modelId].hsf;
    for (attribute = modelHsf->attribute, attributeIndex = matchingBitmapCount = 0;
         attributeIndex < modelHsf->attributeNum; attributeIndex++, attribute++) {
        if(strcmp(bmpName, attribute->bitmap->name) == 0) {
            HU3D_ATTR_ANIM *attributeAnim;
            if(!attribute->animWorkP) {
                attributeAnim = HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3D_ATTR_ANIM),
                                                     Hu3DData[modelId].mallocNo);
                attribute->animWorkP = attributeAnim;
                attributeAnim->attr = HU3D_ATTRANIM_ATTR_NONE;
            } else {
                attributeAnim = attribute->animWorkP;
            }
            attributeAnim->attr |= HU3D_ATTRANIM_ATTR_TEXMTX;
            attributeAnim->texScrId = texScrId;
            attributeAnim->scale.x = attributeAnim->scale.y = 1;
            attributeAnim->trans.x = attributeAnim->trans.y = 0;
            matchingBitmapCount++;
        }
    }
    if(matchingBitmapCount == 0) {
        OSReport("Error: Not Found TexAnim Name\n");
        return HU3D_TEXSCRID_NONE;
    }
    scrollEntry->modelId = modelId;
    scrollEntry->attr = HU3D_TEXSCR_ATTR_NONE;
    scrollEntry->pos.x = scrollEntry->pos.y = scrollEntry->pos.z =  0;
    scrollEntry->rot = 0;
    MTXIdentity(scrollEntry->texMtx);
    return texScrId;
}

/* Called by clients or global texture-scroll cleanup to detach a slot from surviving HSF
 * attributes and mark it free. */
void Hu3DTexScrollKill(HU3D_TEXSCRID texScrId)
{
    HU3D_TEXSCROLL *scrollEntry = &Hu3DTexScrData[texScrId];
    HSF_DATA *modelHsf = Hu3DData[scrollEntry->modelId].hsf;
    if(modelHsf) {
        HSF_ATTRIBUTE *attribute;
        s16 attributeIndex;
        for (attribute = modelHsf->attribute, attributeIndex = 0;
             attributeIndex < modelHsf->attributeNum; attributeIndex++, attribute++) {
            if(attribute->animWorkP) {
                HU3D_ATTR_ANIM *attributeAnim = attribute->animWorkP;
                if(attributeAnim->texScrId == texScrId) {
                    attributeAnim->attr &= ~HU3D_ATTRANIM_ATTR_TEXMTX;
                    if(attributeAnim->attr == HU3D_ATTRANIM_ATTR_NONE) {
                        attribute->animWorkP = NULL;
                        HuMemDirectFree(attributeAnim);
                    }
                }
            }
        }
    }
    scrollEntry->modelId = HU3D_MODELID_NONE;
}

/* Called during global 3D cleanup to release every active texture-scroll slot. */
void Hu3DTexScrollAllKill(void)
{
    HU3D_TEXSCROLL *scrollEntry;
    HU3D_TEXSCRID texScrId;
    for (scrollEntry = &Hu3DTexScrData[0], texScrId = 0; texScrId < HU3D_TEXSCROLL_MAX;
         texScrId++, scrollEntry++) {
        if(scrollEntry->modelId != HU3D_MODELID_NONE) {
            Hu3DTexScrollKill(texScrId);
        }
    }
}

/* Called by texture-scroll clients to set normalized translation and stop position motion. */
void Hu3DTexScrollPosSet(HU3D_TEXSCRID texScrId, float posX, float posY, float posZ)
{
    HU3D_TEXSCROLL *scrollEntry = &Hu3DTexScrData[texScrId];
    scrollEntry->attr &= ~HU3D_TEXSCR_ATTR_POSMOVE;
    scrollEntry->pos.x = posX;
    scrollEntry->pos.y = posY;
    scrollEntry->pos.z = posZ;
}

/* Enables position motion and stores each supplied translation delta multiplied by the current
 * minimumVcount for each Hu3DAnimExec update. */
void Hu3DTexScrollPosMoveSet(HU3D_TEXSCRID texScrId, float posX, float posY, float posZ)
{
    HU3D_TEXSCROLL *scrollEntry = &Hu3DTexScrData[texScrId];
    scrollEntry->attr |= HU3D_TEXSCR_ATTR_POSMOVE;
    scrollEntry->posMove.x = posX*minimumVcount;
    scrollEntry->posMove.y = posY*minimumVcount;
    scrollEntry->posMove.z = posZ*minimumVcount;
}

/* Called by texture-scroll clients to set an angle in degrees and stop rotation motion. */
void Hu3DTexScrollRotSet(HU3D_TEXSCRID texScrId, float rot)
{
    HU3D_TEXSCROLL *scrollEntry = &Hu3DTexScrData[texScrId];
    scrollEntry->attr &= ~HU3D_TEXSCR_ATTR_ROTMOVE;
    scrollEntry->rot = rot;
}

/* Enables rotation motion and stores the supplied degree delta multiplied by the current
 * minimumVcount for each Hu3DAnimExec update. */
void Hu3DTexScrollRotMoveSet(HU3D_TEXSCRID texScrId, float rot)
{
    HU3D_TEXSCROLL *scrollEntry = &Hu3DTexScrData[texScrId];
    scrollEntry->attr |= HU3D_TEXSCR_ATTR_ROTMOVE;
    scrollEntry->rotMove = rot*minimumVcount;
}

/* Called by texture-scroll clients to choose whether scrolling continues during global pause. */
void Hu3DTexScrollPauseDisableSet(HU3D_TEXSCRID texScrId, BOOL pauseDisable)
{
    HU3D_TEXSCROLL *scrollEntry = &Hu3DTexScrData[texScrId];
    if(pauseDisable) {
        scrollEntry->attr |= HU3D_TEXSCR_ATTR_PAUSEDISABLE;
    } else {
        scrollEntry->attr &= ~HU3D_TEXSCR_ATTR_PAUSEDISABLE;
    }
}

static void particleFunc(HU3D_MODEL *modelP, Mtx *mtx);

/* Called by effect setup to create a model whose hook draws a fixed pool of animated particles. */
HU3D_MODELID Hu3DParticleCreate(ANIMDATA *animationData, s16 maxParticleCount)
{
    HU3D_MODELID modelId = Hu3DHookFuncCreate(particleFunc);
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *particleData;
    s16 particleIndex;
    HuVecF *vertexBuffer;
    void *displayListBuffer;
    BOOL interruptState;
    Hu3DModelAttrSet(modelId, HU3D_ATTR_PARTICLE);
    model->hookData = particle =
        HuMemDirectMallocNum(HEAP_MODEL, sizeof(HU3D_PARTICLE), model->mallocNo);
    particle->anim = animationData;
    animationData->useNum++;
    particle->maxCnt = maxParticleCount;
    particle->blendMode = HU3D_PARTICLE_BLEND_NORMAL;
    particle->hook = NULL;
    particle->count = 0;
    particle->attr = HU3D_PARTICLE_ATTR_NONE;
    particle->prevCount = 0;
    particle->dataCnt = particle->emitCnt = 0;
    particle->data = particleData = HuMemDirectMallocNum(
        HEAP_MODEL, maxParticleCount * sizeof(HU3D_PARTICLE_DATA), model->mallocNo);
    particle->prevCounter = -1;
    for(particleIndex=0; particleIndex<maxParticleCount; particleIndex++, particleData++) {
        particleData->scale = 0.0f;
        particleData->attr = 0;
        particleData->cameraBit = HU3D_CAM_ALL;
        particleData->zRot = 0;
        particleData->pos.x = ((frand()&0x7F)-64)*20;
        particleData->pos.y = ((frand()&0x7F)-64)*30;
        particleData->pos.z = ((frand()&0x7F)-64)*20;
        particleData->color.r = particleData->color.g = particleData->color.b =
            particleData->color.a = 255;
    }
    particle->vtxBuf = vertexBuffer =
        HuMemDirectMallocNum(HEAP_MODEL, maxParticleCount * sizeof(HuVecF) * 4, model->mallocNo);
    for(particleIndex=0; particleIndex<maxParticleCount*4; particleIndex++, vertexBuffer++) {
        vertexBuffer->x = vertexBuffer->y = vertexBuffer->z = 0;
    }
    particle->dlBuf = displayListBuffer =
        HuMemDirectMallocNum(HEAP_MODEL, (maxParticleCount * 96) + 128, model->mallocNo);
    DCInvalidateRange(displayListBuffer, (maxParticleCount*96)+128);
    if(HuLoadProcModeGet()) {
        interruptState = OSDisableInterrupts();
    }
    /* GX receives a fixed display-list capacity; the backing buffer is sized from the
     * particle count. */
    GXBeginDisplayList(displayListBuffer, PARTICLE_DISPLAY_LIST_MAX_SIZE);
    GXBegin(GX_QUADS, GX_VTXFMT0, maxParticleCount*4);
    for(particleIndex=0; particleIndex<maxParticleCount; particleIndex++) {
        GXPosition1x16(particleIndex*4);
        GXColor1x16(particleIndex);
        GXTexCoord1x16(0);
        GXPosition1x16((particleIndex*4)+1);
        GXColor1x16(particleIndex);
        GXTexCoord1x16(1);
        GXPosition1x16((particleIndex*4)+2);
        GXColor1x16(particleIndex);
        GXTexCoord1x16(2);
        GXPosition1x16((particleIndex*4)+3);
        GXColor1x16(particleIndex);
        GXTexCoord1x16(3);
    }
    GXEnd();
    particle->dlSize = GXEndDisplayList();
    if(HuLoadProcModeGet()) {
        OSRestoreInterrupts(interruptState);
    }
    return modelId;
}

/* Called by effect clients to set the scale of every particle in a model's pool. */
void Hu3DParticleScaleSet(HU3D_MODELID modelId, float scale)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *particleData;
    s16 particleIndex;
    for (particleData = particle->data, particleIndex = 0; particleIndex < particle->maxCnt;
         particleIndex++, particleData++) {
        particleData->scale = scale;
    }
}

/* Called by effect clients to set every particle's rotation around the Z axis in radians. */
void Hu3DParticleZRotSet(HU3D_MODELID modelId, float zRotation)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *particleData;
    s16 particleIndex;
    for (particleData = particle->data, particleIndex = 0; particleIndex < particle->maxCnt;
         particleIndex++, particleData++) {
        particleData->zRot = zRotation;
    }
}

/* Called by effect clients to set the RGB color of every particle; alpha is left unchanged. */
void Hu3DParticleColSet(HU3D_MODELID modelId, u8 red, u8 green, u8 blue)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *particleData;
    s16 particleIndex;
    for (particleData = particle->data, particleIndex = 0; particleIndex < particle->maxCnt;
         particleIndex++, particleData++) {
        particleData->color.r = red;
        particleData->color.g = green;
        particleData->color.b = blue;
    }
}

/* Called by effect clients to set each particle's 8-bit alpha from a normalized level. */
void Hu3DParticleTPLvlSet(HU3D_MODELID modelId, float alphaLevel)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    HU3D_PARTICLE_DATA *particleData;
    u8 alpha;
    s16 particleIndex;
    for (particleData = particle->data, alpha = alphaLevel * 255, particleIndex = 0;
         particleIndex < particle->maxCnt; particleIndex++, particleData++) {
        particleData->color.a = alpha;
    }
}

/* Called by effect clients to select the GX blend mode for the particle model. */
void Hu3DParticleBlendModeSet(HU3D_MODELID modelId, u8 blendMode)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    particle->blendMode = blendMode;
}

/* Installs a draw callback gated by GlobalCounter and global pause, with NOPAUSE allowing paused
 * updates. Shadow draws do not mark the callback as already run. */
void Hu3DParticleHookSet(HU3D_MODELID modelId, HU3D_PARTICLE_HOOK hook)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    particle->hook = hook;
}

/* Called by effect clients to enable selected particle behavior flags. */
void Hu3DParticleAttrSet(HU3D_MODELID modelId, u8 attr)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    particle->attr |= attr;
}

/* Called by effect clients to clear selected particle behavior flags. */
void Hu3DParticleAttrReset(HU3D_MODELID modelId, u8 attr)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    particle->attr &= ~attr;
}

/* Called by effect clients to set the particle model's counter, updated per non-shadow draw. */
void Hu3DParticleCntSet(HU3D_MODELID modelId, s16 count)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    particle->count = count;
}

/* Called by effect clients to enable particle sprite animation and select its starting bank. */
void Hu3DParticleAnimModeSet(HU3D_MODELID modelId, s16 animBank)
{
    HU3D_MODEL *model = &Hu3DData[modelId];
    HU3D_PARTICLE *particle = model->hookData;
    particle->attr |= HU3D_PARTICLE_ATTR_ANIMON;
    particle->animBank = animBank;
    particle->animTime = 0;
    particle->animNo = 0;
    particle->animSpeed = 1;
}

static Vec basePos[] = {
    { -0.5f,  0.5f, 0.0f },
    {  0.5f,  0.5f, 0.0f },
    {  0.5f, -0.5f, 0.0f },
    { -0.5f, -0.5f, 0.0f }
};

static HuVec2f baseST[] = {
    { 0.0f, 0.0f },
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 1.0f },
};

/* Model hook builds and draws particle quads. Its counter increments on each non-shadow draw
* while unpaused and STOPCNT is clear; NOPAUSE does not override this counter pause check. */
static void particleFunc(HU3D_MODEL *modelP, Mtx *mtx)
{
    HuVecF *vtxBuf;
    float scale;
    float x;
    float y;
    s16 bmpFmt;
    s16 dispF;
    s32 i;
    ANIMFRAME *animFrame;
    ANIMPAT *animPat;
    ANIMDATA *anim;
    ANIMBANK *animBank;
    ANIMBMP *animBmp;
    ANIMLAYER *animLayer;
    HU3D_PARTICLE *particleP;
    HU3D_PARTICLE_DATA *particleDataP;
    Mtx mtxInv;
    Mtx mtxPos;
    Mtx mtxRot;
    HuVecF scaleVtx[4];
    HuVecF finalVtx[4];
    HuVecF initVtx[4];
    ROMtx basePosMtx;
    HuVecF unusedVector;

    particleP = modelP->hookData;
    anim = particleP->anim;
    if(HmfInverseMtxF3X3(*mtx, mtxInv) == FALSE) {
        PSMTXIdentity(mtxInv);
    }
    PSMTXReorder(mtxInv, basePosMtx);
    if((Hu3DPauseF == FALSE || (modelP->attr & HU3D_ATTR_NOPAUSE))) {
        if(particleP->hook && particleP->prevCounter != GlobalCounter) {
            HU3D_PARTICLE_HOOK hook = particleP->hook;
            hook(modelP, particleP, *mtx);
        }
    } else if(particleP->prevCounter == -1) {
        return;
    }
    particleDataP = particleP->data;
    vtxBuf = particleP->vtxBuf;
    PSMTXROMultVecArray(basePosMtx, &basePos[0], initVtx, 4);
    /* This component is assigned, but the vector is not read in the draw path. */
    unusedVector.z = 1;

    for(i=0, dispF=FALSE; i<particleP->maxCnt; i++, particleDataP++) {
        if(particleDataP->scale && (particleDataP->cameraBit & Hu3DCameraBit)) {
            if(particleDataP->attr & HU3D_PARTICLE_ATTR_SCALEY) {
                scaleVtx[0].x = basePos[0].x * particleDataP->scale;
                scaleVtx[0].y = basePos[0].y * particleDataP->scaleY;
                scaleVtx[0].z = basePos[0].z;
                scaleVtx[1].x = basePos[1].x * particleDataP->scale;
                scaleVtx[1].y = basePos[1].y * particleDataP->scaleY;
                scaleVtx[1].z = basePos[1].z;
                scaleVtx[2].x = basePos[2].x * particleDataP->scale;
                scaleVtx[2].y = basePos[2].y * particleDataP->scaleY;
                scaleVtx[2].z = basePos[2].z;
                scaleVtx[3].x = basePos[3].x * particleDataP->scale;
                scaleVtx[3].y = basePos[3].y * particleDataP->scaleY;
                scaleVtx[3].z = basePos[3].z;
                PSMTXRotRad(mtxRot, 'Z', particleDataP->zRot);
                PSMTXConcat(mtxInv, mtxRot, mtxPos);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[1], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[2], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[3], &particleDataP->pos, vtxBuf++);
            } else if(!particleDataP->zRot) {
                scale = particleDataP->scale;
                vtxBuf->x = initVtx[0].x * scale + particleDataP->pos.x;
                vtxBuf->y = initVtx[0].y * scale + particleDataP->pos.y;
                vtxBuf->z = initVtx[0].z * scale + particleDataP->pos.z;
                vtxBuf++;
                vtxBuf->x = initVtx[1].x * scale + particleDataP->pos.x;
                vtxBuf->y = initVtx[1].y * scale + particleDataP->pos.y;
                vtxBuf->z = initVtx[1].z * scale + particleDataP->pos.z;
                vtxBuf++;
                vtxBuf->x = initVtx[2].x * scale + particleDataP->pos.x;
                vtxBuf->y = initVtx[2].y * scale + particleDataP->pos.y;
                vtxBuf->z = initVtx[2].z * scale + particleDataP->pos.z;
                vtxBuf++;
                vtxBuf->x = initVtx[3].x * scale + particleDataP->pos.x;
                vtxBuf->y = initVtx[3].y * scale + particleDataP->pos.y;
                vtxBuf->z = initVtx[3].z * scale + particleDataP->pos.z;
                vtxBuf++;
            } else {
                VECScale(&basePos[0], &scaleVtx[0], particleDataP->scale);
                VECScale(&basePos[1], &scaleVtx[1], particleDataP->scale);
                VECScale(&basePos[2], &scaleVtx[2], particleDataP->scale);
                VECScale(&basePos[3], &scaleVtx[3], particleDataP->scale);
                PSMTXRotRad(mtxRot, 'Z', particleDataP->zRot);
                PSMTXConcat(mtxInv, mtxRot, mtxPos);
                PSMTXMultVecArray(mtxPos, scaleVtx, finalVtx, 4);
                VECAdd(&finalVtx[0], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[1], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[2], &particleDataP->pos, vtxBuf++);
                VECAdd(&finalVtx[3], &particleDataP->pos, vtxBuf++);
            }
            dispF = TRUE;
        } else {
            vtxBuf->x = vtxBuf->y = vtxBuf->z = 0.0f;
            vtxBuf++;
            vtxBuf->x = vtxBuf->y = vtxBuf->z = 0.0f;
            vtxBuf++;
            vtxBuf->x = vtxBuf->y = vtxBuf->z = 0.0f;
            vtxBuf++;
            vtxBuf->x = vtxBuf->y = vtxBuf->z = 0.0f;
            vtxBuf++;
        }
    }
    if(dispF) {
        DCFlushRangeNoSync(particleP->vtxBuf, particleP->maxCnt * sizeof(Vec) * 4);
        GXLoadPosMtxImm(*mtx, 0);
        GXSetNumTevStages(1);
        GXSetNumTexGens(1);
        GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        if(shadowModelDrawF != 0) {
            GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ONE, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
            GXSetZMode(0, GX_LEQUAL, 0);
        } else {
            bmpFmt = (particleP->anim->bmp->dataFmt & 0xF);
            if(bmpFmt == ANIM_BMP_I8 || bmpFmt == ANIM_BMP_I4) {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ONE, GX_CC_RASC, GX_CC_ZERO);
            } else {
                GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
            }
            /* Particle depth writes are enabled when ZWRITE_OFF is set and disabled when it is
             * clear. */
            if(modelP->attr & HU3D_ATTR_ZWRITE_OFF) {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
            } else {
                GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
            }
        }
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumChans(1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                      GX_AF_NONE);
        if(particleP->attr & HU3D_PARTICLE_ATTR_ANIMON) {
            animBank = &anim->bank[particleP->animBank];
            animFrame = &animBank->frame[particleP->animNo];
            animPat = &anim->pat[animFrame->pat];
            HuSprTexLoad(particleP->anim, animPat->layer->bmpNo, GX_TEXMAP0, GX_CLAMP, GX_CLAMP,
                         GX_LINEAR);
            /* Sprite time advances on every draw with visible particles, including shadow and
             * additional camera passes, when unpaused or NOPAUSE is set. */
            if(Hu3DPauseF == FALSE || (modelP->attr & HU3D_ATTR_NOPAUSE)) {
                for(i=0; i<(s32)particleP->animSpeed*minimumVcount; i++) {
                    particleP->animTime += 1.0f;
                    if(particleP->animTime >= animFrame->time) {
                        particleP->animNo++;
                        particleP->animTime -= animFrame->time;
                        if(particleP->animNo >= animBank->timeNum || animFrame[1].time == -1) {
                            particleP->animNo = 0;
                        }
                    }
                    animFrame = &animBank->frame[particleP->animNo];
                }
                particleP->animTime += particleP->animSpeed * minimumVcount - i;
                if(particleP->animTime >= animFrame->time) {
                    particleP->animNo++;
                    particleP->animTime -= animFrame->time;
                    if(particleP->animNo >= animBank->timeNum || animFrame[1].time == -1) {
                        particleP->animNo = 0;
                    }
                }
            }
            animLayer = animPat->layer;
            animBmp = &anim->bmp[animLayer->bmpNo];
            x = (float) animLayer->sizeX / animBmp->sizeX;
            y = (float) animLayer->sizeY / animBmp->sizeY;
            PSMTXScale(mtxInv, x, y, 1.0f);
            x = (float) animLayer->startX / animBmp->sizeX;
            y = (float) animLayer->startY / animBmp->sizeY;
            mtxTransCat(mtxInv, x, y, 0.0f);
            GXLoadTexMtxImm(mtxInv, GX_TEXMTX0, GX_MTX2x4);
            GXSetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0);
        } else {
            HuSprTexLoad(particleP->anim, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP, GX_LINEAR);
        }
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetZCompLoc(0);
        switch (particleP->blendMode) {
            case HU3D_PARTICLE_BLEND_NORMAL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_ADDCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
                break;
            case HU3D_PARTICLE_BLEND_INVCOL:
                GXSetBlendMode(GX_BM_BLEND, GX_BL_ZERO, GX_BL_INVDSTCLR, GX_LO_NOOP);
                break;
        }
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
        GXSetArray(GX_VA_POS, particleP->vtxBuf, sizeof(HuVecF));
        GXSetVtxDesc(GX_VA_CLR0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetArray(GX_VA_CLR0, &particleP->data->color, sizeof(HU3D_PARTICLE_DATA));
        GXSetVtxDesc(GX_VA_TEX0, GX_INDEX16);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetArray(GX_VA_TEX0, baseST, sizeof(HuVec2f));
        /* Submit and count the whole pool, including zero-sized quads for hidden particles. */
        GXCallDisplayList(particleP->dlBuf, particleP->dlSize);
        totalPolyCnt += particleP->maxCnt;
    }
    if(shadowModelDrawF == FALSE) {
        if(!(particleP->attr & HU3D_PARTICLE_ATTR_STOPCNT) && Hu3DPauseF == 0) {
            particleP->count++;
        }
        if(particleP->prevCount != 0 && particleP->prevCount <= particleP->count) {
            /* RESETCNT's zero assignment is overwritten below; reaching prevCount always
             * leaves count at prevCount. */
            if(particleP->attr & HU3D_PARTICLE_ATTR_RESETCNT) {
                particleP->count = 0;
            }
            particleP->count = particleP->prevCount;
        }
        particleP->prevCounter = GlobalCounter;
    }
}

#include "game/process.h"

static HUPROCESS *parManProc[HU3D_PARMAN_MAX];

static float jitterTbl[] = {
    1.0f, 0.9f, 0.7f, 0.5f,
    0.5f, 0.7f, 0.9f, 1.0f
};

static void ParManFunc();
static void ParManHook(HU3D_MODEL *modelP, HU3D_PARTICLE *particleP, Mtx mtx);

/* Clears manager process slots during 3D particle-system initialization. */
void Hu3DParManInit(void)
{
    s16 i;
    for(i=0; i<HU3D_PARMAN_MAX; i++) {
        parManProc[i] = NULL;
    }
}

/* Installs the manager's draw callback and initializes its particle pool. */
static void Hu3DParManParticleInit(HU3D_MODELID modelId, s16 ownerParManId, float scale)
{
    HU3D_MODEL *modelP;
    HU3D_PARTICLE *particleP;
    HU3D_PARTICLE_DATA *particleDataP;
    s16 i;
    Hu3DParticleHookSet(modelId, ParManHook);
    modelP = &Hu3DData[modelId];
    particleP = modelP->hookData;
    particleP->dataCnt = ownerParManId;
    for(particleDataP=particleP->data, i=0; i<particleP->maxCnt; i++, particleDataP++) {
        particleDataP->scale = scale;
    }
}

/* Creates a particle model and manager process using the supplied emission parameters. */
HU3D_PARMANID Hu3DParManCreate(ANIMDATA *anim, s16 maxCnt, HU3D_PARMAN_PARAM *param)
{
    HU3D_PARMANID parManId;
    HU3D_MODELID modelId;
    HU3D_PARMAN *parManP;
    for(parManId=0; parManId<HU3D_PARMAN_MAX; parManId++) {
        if(!parManProc[parManId]) {
            break;
        }
    }
    if(parManId == HU3D_PARMAN_MAX) {
        return HU3D_PARMANID_NONE;
    }
    modelId = Hu3DParticleCreate(anim, maxCnt);

    Hu3DParManParticleInit(modelId, parManId, 0.0f);
    parManProc[parManId] = HuPrcCreate(ParManFunc, 0, 4096, 0);
    parManProc[parManId]->property = parManP =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(HU3D_PARMAN), HU_MEMNUM_OVL);
    parManP->modelId = modelId;
    parManP->param = param;
    parManP->attr = HU3D_PARMAN_ATTR_NONE;
    parManP->pos.x = parManP->pos.y = parManP->pos.z = 0;
    parManP->vec.x = 0;
    parManP->vec.y = 1;
    parManP->vec.z = 1;
    parManP->vacuum.x = 0;
    parManP->vacuum.y = 0;
    parManP->vacuum.z = 0;
    parManP->vacuumSpeed = 1;
    parManP->accel = 0;
    parManP->timeLimit = 0;
    parManP->parManId = parManId;
    return parManId;
}

/* Creates a manager process that shares the source manager's particle model. */
HU3D_PARMANID Hu3DParManLink(HU3D_PARMANID linkParManId, HU3D_PARMAN_PARAM *param)
{
    HU3D_PARMANID parManId;
    HU3D_PARMAN *parManP;
    HU3D_PARMAN *linkParManP;
    for(parManId=0; parManId<HU3D_PARMAN_MAX; parManId++) {
        if(!parManProc[parManId]) {
            break;
        }
    }
    if(parManId == HU3D_PARMAN_MAX) {
        return HU3D_PARMANID_NONE;
    }
    linkParManP = parManProc[linkParManId]->property;
    parManProc[parManId] = HuPrcCreate(ParManFunc, 100, 4096, 0);
    parManProc[parManId]->property = parManP =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(HU3D_PARMAN), HU_MEMNUM_OVL);
    parManP->modelId = linkParManP->modelId;
    parManP->param = param;
    parManP->attr = HU3D_PARMAN_ATTR_NONE;
    parManP->pos.x = parManP->pos.y = parManP->pos.z = 0;
    parManP->vec.x = 0;
    parManP->vec.y = 1;
    parManP->vec.z = 1;
    parManP->vacuum.x = 0;
    parManP->vacuum.y = 0;
    parManP->vacuum.z = 0;
    parManP->vacuumSpeed = 1;
    parManP->accel = 0;
    parManP->timeLimit = 0;
    parManP->parManId = parManId;
    return parManId;
}

/* Stops one manager, hides its particles, and kills its model when no peers use it. */
void Hu3DParManKill(HU3D_PARMANID parManId)
{
    HU3D_PARMAN *parManLinkP;
    if(parManProc[parManId]) {
        HU3D_PARMAN *parManP = parManProc[parManId]->property;
        HU3D_MODEL *modelP = &Hu3DData[parManP->modelId];
        HU3D_PARTICLE *particleP = modelP->hookData;
        HU3D_PARTICLE_DATA *particleDataP;
        s16 i;
        for(particleDataP=particleP->data, i=0; i<particleP->maxCnt; i++, particleDataP++) {
            if(particleDataP->parManId == parManId) {
                particleDataP->scale = 0;
            }
        }
        for(i=0; i<HU3D_PARMAN_MAX; i++) {
            if(!parManProc[i]) {
                continue;
            }
            if(i != parManId) {
                parManLinkP = parManProc[i]->property;
                if(parManLinkP->modelId == parManP->modelId) {
                    break;
                }
            }
        }
        if(i == HU3D_PARMAN_MAX) {
            Hu3DModelKill(parManP->modelId);
        }
        /* If this is the callback owner and peers keep the model alive, dataCnt still names
         * this manager's slot after it is cleared below. */
        HuPrcKill(parManProc[parManId]);
        parManProc[parManId] = NULL;
        HuMemDirectFree(parManP);
    }

}

/* Stops every active manager during particle-effect teardown. */
void Hu3DParManAllKill(void)
{
    HU3D_PARMANID parManId;
    for(parManId=0; parManId<HU3D_PARMAN_MAX; parManId++) {
        if(parManProc[parManId]) {
            Hu3DParManKill(parManId);
        }
    }
}

/* Returns the manager state stored in its process property. */
HU3D_PARMAN *Hu3DParManPtrGet(HU3D_PARMANID parManId)
{
    return parManProc[parManId]->property;
}

/* Sets the emission origin in the particle model's coordinates; the model transform places
 * emitted particles in world space. */
void Hu3DParManPosSet(HU3D_PARMANID parManId, float posX, float posY, float posZ)
{
    HU3D_PARMAN *parManP = parManProc[parManId]->property;
    parManP->pos.x = posX;
    parManP->pos.y = posY;
    parManP->pos.z = posZ;
}

/* Sets the direction vector used for particle emission. */
void Hu3DParManVecSet(HU3D_PARMANID parManId, float x, float y, float z)
{
    HU3D_PARMAN *parManP = parManProc[parManId]->property;
    parManP->vec.x = x;
    parManP->vec.y = y;
    parManP->vec.z = z;
}

/* Converts Euler angles to the manager's emission direction vector. */
void Hu3DParManRotSet(HU3D_PARMANID parManId, float rotX, float rotY, float rotZ)
{
    HU3D_PARMAN *parManP = parManProc[parManId]->property;
    Mtx rotMtx;
    mtxRot(rotMtx, rotX, rotY, rotZ);
    parManP->vec.x = rotMtx[0][2];
    parManP->vec.y = rotMtx[1][2];
    parManP->vec.z = rotMtx[2][2];
}

/* Enables behavior flags on a particle manager. */
void Hu3DParManAttrSet(HU3D_PARMANID parManId, s32 attr)
{
    HU3D_PARMAN *parManP = parManProc[parManId]->property;
    parManP->attr |= attr;
}

/* Clears selected behavior flags on a particle manager. */
void Hu3DParManAttrReset(HU3D_PARMANID parManId, s32 attr)
{
    HU3D_PARMAN *parManP = parManProc[parManId]->property;
    parManP->attr &= ~attr;
}

/* Returns the particle model shared by this manager. */
HU3D_MODELID Hu3DParManModelIDGet(HU3D_PARMANID parManId)
{
    HU3D_PARMAN *parManP = parManProc[parManId]->property;
    return parManP->modelId;
}

/* Sets the emission countdown; zero disables countdown updates. This does not clear TIMEUP, so a
 * stopped manager must have that flag cleared separately. */
void Hu3DParManTimeLimitSet(HU3D_PARMANID parManId, s32 timeLimit)
{
    HU3D_PARMAN *parManP = parManProc[parManId]->property;
    parManP->timeLimit = timeLimit;
}

/* Enables vacuum behavior and sets its target position and acceleration speed. */
void Hu3DParManVacumeSet(HU3D_PARMANID parManId, float x, float y, float z, float speed)
{
    HU3D_PARMAN *parManP;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_VACUUM);
    parManP = parManProc[parManId]->property;
    parManP->vacuum.x = x;
    parManP->vacuum.y = y;
    parManP->vacuum.z = z;
    parManP->vacuumSpeed = speed;
}

/* Enables a fixed color-table index for particles emitted by this manager. */
void Hu3DParManColorSet(HU3D_PARMANID parManId, s16 color)
{
    HU3D_PARMAN *parManP;
    Hu3DParManAttrSet(parManId, HU3D_PARMAN_ATTR_SETCOLOR);
    parManP = parManProc[parManId]->property;
    parManP->color = color;
}

/* Sets the render layer on the manager's shared particle model. */
void Hu3DParManLayerSet(HU3D_PARMANID parManId, s16 layer)
{
    Hu3DModelLayerSet(Hu3DParManModelIDGet(parManId), layer);
}

/* Manager process emits particles at the configured rate and sleeps each frame. */
static void ParManFunc()
{
    HUPROCESS *processP;
    HU3D_PARMAN *parManP;
    HU3D_PARMAN_PARAM *param;
    HU3D_MODEL *modelP;
    HU3D_PARTICLE *particleP;
    HU3D_PARTICLE_DATA *particleDataP;
    HU3D_PARTICLE_DATA *particleDataEnd;
    Vec vecDir;
    Vec spawnOffset;
    Vec dir;
    Vec up;
    float c;
    float s;
    float angleStart;
    float upRot;
    float emissionRate;
    float rot;
    s16 colorIdx;
    s16 circleIdx;

    processP = HuPrcCurrentGet();
    parManP = processP->property;
    param = parManP->param;
    modelP = &Hu3DData[parManP->modelId];
    while(1) {
        if(Hu3DPauseF && !(modelP->attr & HU3D_ATTR_NOPAUSE)) {
            HuPrcVSleep();
            continue;
        }
        /* Manager PAUSE freezes particle motion, decay, and age in ParManHook, but scale jitter
         * continues; emission here continues unless TIMEUP is set. */
        particleP = modelP->hookData;
        particleDataP = particleP->data;
        /* RANDTIME90 and RANDTIME70 truncate accelRange before computing their random-span
         * bounds. */
        if(parManP->attr & HU3D_PARMAN_ATTR_RANDTIME90) {
            emissionRate = param->accelRange * 0.9 +
                           frandmod((u32) param->accelRange * 0.1 * 1000.0) / 1000.0f;
        } else if(parManP->attr & HU3D_PARMAN_ATTR_RANDTIME70) {
            emissionRate = param->accelRange * 0.7 +
                           frandmod((u32) param->accelRange * 0.3 * 1000.0) / 1000.0f;
        } else {
            emissionRate = param->accelRange;
        }
        parManP->accel += emissionRate;
        circleIdx = 0;
        particleDataEnd = &particleP->data[particleP->maxCnt];
        if(parManP->attr & HU3D_PARMAN_ATTR_RANDANGLE) {
            angleStart = frandmod((u32)(360.0f/param->accelRange)*100)/100;
        }
        while(parManP->accel >= 1.0f) {
            if(parManP->attr & HU3D_PARMAN_ATTR_TIMEUP) {
                parManP->accel -= 1.0f;
            } else {
                while(particleDataP < particleDataEnd) {
                    if(!particleDataP->scale) {
                        s = param->scaleBase;
                        if(parManP->attr & HU3D_PARMAN_ATTR_RANDSCALE90) {
                            s = s*0.9+(frandmod((u32)(s*0.1*1000.0))/1000.0f);
                        } else if(parManP->attr & HU3D_PARMAN_ATTR_RANDSCALE70) {
                            s = s*0.7+(frandmod((u32)(s*0.3*1000.0))/1000.0f);
                        }
                        particleDataP->scaleBase = s;
                        particleDataP->scale = s;
                        particleDataP->pos = parManP->pos;
                        spawnOffset.x = frandmod((u32)(param->scaleRange*2.0f))-param->scaleRange;
                        spawnOffset.y = frandmod((u32)(param->scaleRange*2.0f))-param->scaleRange;
                        spawnOffset.z = frandmod((u32)(param->scaleRange*2.0f))-param->scaleRange;
                        if(HuMag2Point3D(spawnOffset.x, spawnOffset.y, spawnOffset.z) == 0) {
                            spawnOffset.x = spawnOffset.y = spawnOffset.z = 0;
                        } else {
                            VECNormalize(&spawnOffset, &spawnOffset);
                        }

                        VECScale(&spawnOffset, &spawnOffset, param->scaleRange);
                        VECAdd(&spawnOffset, &particleDataP->pos, &particleDataP->pos);
                        VECNormalize(&parManP->vec, &vecDir);
                        if(parManP->attr & HU3D_PARMAN_ATTR_RANDANGLE) {
                            upRot = angleStart+(360.0f/param->accelRange)*circleIdx;
                            rot = param->angleRange;
                        } else {
                            upRot = frandmod(360);
                            if(param->angleRange) {
                                rot = frandmod((u32)param->angleRange);
                            } else {
                                rot = 0.0f;
                            }
                        }
                        if(vecDir.x * vecDir.x < 0.000001 && vecDir.z * vecDir.z < 0.000001) {
                            up.x = 1.0f;
                            up.y = up.z = 0.0f;
                        } else {
                            if(vecDir.y * vecDir.y > 0.000001) {
                                dir.x = vecDir.x;
                                dir.y = 0.0f;
                                dir.z = vecDir.z;
                            } else {
                                dir.x = vecDir.x;
                                dir.y = 1.0f;
                                dir.z = vecDir.z;
                            }
                            VECCrossProduct(&dir, &vecDir, &up);
                        }
                        VECNormalize(&up, &up);
                        s = HuSin(upRot);
                        c = HuCos(upRot);
                        dir.x = up.x * (vecDir.x * vecDir.x + c * (1.0f - vecDir.x * vecDir.x))
                            + up.y * (vecDir.x * vecDir.y * (1.0f - c) - vecDir.z * s)
                            + up.z * (vecDir.x * vecDir.z * (1.0f - c) + vecDir.y * s);
                        dir.y = up.x * (vecDir.x * vecDir.y * (1.0f - c) + vecDir.z * s)
                            + up.y * (vecDir.y * vecDir.y + c * (1.0f - vecDir.y * vecDir.y))
                            + up.z * (vecDir.y * vecDir.z * (1.0f - c) - vecDir.x * s);
                        dir.z = up.x * (vecDir.x * vecDir.z * (1.0f - c) - vecDir.y * s)
                            + up.y * (vecDir.y * vecDir.z * (1.0f - c) + vecDir.x * s)
                            + up.z * (vecDir.z * vecDir.z + c * (1.0f - vecDir.z * vecDir.z));
                        VECCrossProduct(&dir, &vecDir, &up);
                        s = HuSin(rot);
                        c = HuCos(rot);
                        dir.x = vecDir.x * (up.x * up.x + c * (1.0f - up.x * up.x))
                            + vecDir.y * (up.x * up.y * (1.0f - c) - up.z * s)
                            + vecDir.z * (up.x * up.z * (1.0f - c) + up.y * s);
                        dir.y = vecDir.x * (up.x * up.y * (1.0f - c) + up.z * s)
                            + vecDir.y * (up.y * up.y + c * (1.0f - up.y * up.y))
                            + vecDir.z * (up.y * up.z * (1.0f - c) - up.x * s);
                        dir.z = vecDir.x * (up.x * up.z * (1.0f - c) - up.y * s)
                            + vecDir.y * (up.y * up.z * (1.0f - c) + up.x * s)
                            + vecDir.z * (up.z * up.z + c * (1.0f - up.z * up.z));
                        VECNormalize(&dir, &dir);
                        s = param->speedBase;
                        if(parManP->attr & HU3D_PARMAN_ATTR_RANDSPEED90) {
                            s = s*0.9+frandmod((u32)(s*0.1*1000.0))/1000.0f;
                        } else if(parManP->attr & HU3D_PARMAN_ATTR_RANDSPEED70) {
                            s = s*0.7+frandmod((u32)(s*0.3*1000.0))/1000.0f;
                        } else if(parManP->attr & HU3D_PARMAN_ATTR_RANDSPEED100) {
                            s = frandmod((u32)(s*1000.0f))/1000.0f;
                        }
                        VECScale(&dir, &particleDataP->vel, s);
                        particleDataP->accel = param->gravity;
                        particleDataP->speedDecay = param->speedDecay;
                        if(parManP->attr & HU3D_PARMAN_ATTR_SETCOLOR) {
                            particleDataP->colorIdx = colorIdx = parManP->color;
                        } else {
                            particleDataP->colorIdx = colorIdx = frandmod(param->colorNum);
                        }
                        particleDataP->color = param->colorStart[colorIdx];
                        particleDataP->time = 0;
                        particleDataP->parManId = parManP->parManId;
                        break;
                    } else {
                        particleDataP++;
                    }
                }
                parManP->accel -= 1.0f;
                circleIdx++;
            }
        }
        if(parManP->timeLimit != 0) {
            parManP->timeLimit--;
            if(parManP->timeLimit == 0) {
                parManP->attr |= HU3D_PARMAN_ATTR_TIMEUP;
            }
        }
        HuPrcVSleep();
    }
}

/* Particle draw callback advances motion, color, scale, vacuum, and jitter state. */
static void ParManHook(HU3D_MODEL *modelP, HU3D_PARTICLE *particleP, Mtx mtx)
{
    HU3D_PARMAN_PARAM *param;
    HU3D_PARMAN *parManP;
    HU3D_PARTICLE_DATA *particleDataP;
    GXColor *colorEnd;
    GXColor *colorStart;
    Vec vacuumAccel;
    Vec vacuumDist;
    float weight;
    s16 colorIdx;
    s16 i;

    if(Hu3DPauseF == FALSE || (modelP->attr & HU3D_ATTR_NOPAUSE)) {
        particleDataP = particleP->data;
        for(i=0; i<particleP->maxCnt; i++, particleDataP++) {
            if(particleDataP->scale) {
                parManP = parManProc[particleDataP->parManId]->property;
                param = parManP->param;
                if(parManP->attr & HU3D_PARMAN_ATTR_SCALEJITTER) {
                    particleDataP->scale =
                        particleDataP->scaleBase * jitterTbl[(parManP->jitterNo + i) & 7];
                } else {
                    particleDataP->scale = particleDataP->scaleBase;
                }
                if(!(parManP->attr & HU3D_PARMAN_ATTR_PAUSE)) {
                    s16 time = particleDataP->time;
                    particleDataP->pos.x += particleDataP->vel.x+particleDataP->accel.x;
                    particleDataP->pos.y += particleDataP->vel.y+particleDataP->accel.y;
                    particleDataP->pos.z += particleDataP->vel.z+particleDataP->accel.z;
                    VECScale(&particleDataP->vel, &particleDataP->vel, particleDataP->speedDecay);
                    VECAdd(&param->gravity, &particleDataP->accel, &particleDataP->accel);
                    if(parManP->attr & HU3D_PARMAN_ATTR_VACUUM) {
                        VECSubtract(&parManP->vacuum, &particleDataP->pos, &vacuumAccel);
                        VECNormalize(&vacuumAccel, &vacuumAccel);
                        VECScale(&vacuumAccel, &vacuumAccel, parManP->vacuumSpeed);
                        VECAdd(&vacuumAccel, &particleDataP->accel, &particleDataP->accel);
                        VECAdd(&particleDataP->vel, &particleDataP->accel, &vacuumAccel);
                        VECSubtract(&parManP->vacuum, &particleDataP->pos, &vacuumDist);
                        if(VECSquareMag(&vacuumDist) <= VECSquareMag(&vacuumAccel)) {
                            particleDataP->scale = 0.0f;
                            continue;
                        }
                    }
                    particleDataP->scaleBase *= param->scaleDecay;
                    weight = (float) particleDataP->time / param->maxTime;
                    if(weight > 1.0f) {
                        weight = 1.0f;
                    }
                    OSf32tos16(&particleDataP->colorIdx, &colorIdx);
                    colorStart = &param->colorStart[colorIdx];
                    colorEnd = &param->colorEnd[colorIdx];
                    particleDataP->color.r = colorStart->r+(weight*(colorEnd->r-colorStart->r));
                    particleDataP->color.g = colorStart->g+(weight*(colorEnd->g-colorStart->g));
                    particleDataP->color.b = colorStart->b+(weight*(colorEnd->b-colorStart->b));
                    particleDataP->color.a = colorStart->a+(weight*(colorEnd->a-colorStart->a));
                    if(particleDataP->scale < 0.01 || particleDataP->time >= param->maxTime) {
                        particleDataP->scale = 0.0f;
                    }
                    particleDataP->time++;
                }
            }
        }
        parManP = parManProc[particleP->dataCnt]->property;
        parManP->jitterNo++;
        DCStoreRangeNoSync(particleP->data, particleP->maxCnt*sizeof(HU3D_PARTICLE_DATA));
    }
}

#include "datanum/effect.h"
#include "game/disp.h"

HU3D_WATER Hu3DWaterData;

static float waterTexMtx[2][3] = {
    0.05f, 0, 0,
    0, 0.05f, 0
};

static float waterWaveTexMtx[2][3] = {
    0.1f, 0, 0,
    0, 0.1f, 0
};

static void CopyWaterFb(s16 layerNo);
static void WaterLayerHook(s16 layerNo);

/* Allocates water textures and installs layer callbacks. HU3D_WATER_ANIM_NONE selects built-in
 * textures; NULL omits surface or sky, but rendering requires a bump animation. */
void Hu3DWaterCreate(s16 layerNo, void *animBump, void *animSurface, void *animSky, BOOL mipMapF,
                     HuVecF *posMin, HuVecF *posMax)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    s16 fbWaterW;
    s16 fbWaterH;
    s16 i, j;
    waterP->glowMode = 0;
    if(mipMapF) {
        fbWaterW = HU_FB_WIDTH/2;
        fbWaterH = HU_FB_HEIGHT/2;
    } else {
        fbWaterW = HU_FB_WIDTH;
        fbWaterH = HU_FB_HEIGHT;
    }
    waterP->fbWater = HuMemDirectMallocNum(
        HEAP_MODEL, GXGetTexBufferSize(fbWaterW, fbWaterH, GX_TF_RGB565, GX_FALSE, 0),
        HU_MEMNUM_OVL);
    waterP->fbDisp = HuMemDirectMallocNum(
        HEAP_MODEL, GXGetTexBufferSize(HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, GX_FALSE, 0),
        HU_MEMNUM_OVL);
    if(layerNo >= HU3D_LAYER_HOOK_POST) {
        layerNo -= HU3D_LAYER_HOOK_POST;
    }
    Hu3DLayerHookSet(layerNo, CopyWaterFb);
    waterP->mipMapF = mipMapF;
    waterP->fbWaterW = fbWaterW;
    waterP->fbWaterH = fbWaterH;
    if(animBump == HU3D_WATER_ANIM_NONE) {
        waterP->animBump = HuSprAnimDataRead(EFFECT_ANM_water_bump);
    } else if(animBump) {
        waterP->animBump = HuSprAnimRead(animBump);
    } else {
        waterP->animBump = animBump;
    }
    if(animSurface == HU3D_WATER_ANIM_NONE) {
        waterP->animSurface = HuSprAnimDataRead(EFFECT_ANM_water_surface);
    } else if(animSurface) {
        waterP->animSurface = HuSprAnimRead(animSurface);
    } else {
        waterP->animSurface = animSurface;
    }
    if(animSky == HU3D_WATER_ANIM_NONE) {
        waterP->animSky = HuSprAnimDataRead(EFFECT_ANM_water_sky);
    } else if(animSky) {
        waterP->animSky = HuSprAnimRead(animSky);
    } else {
        waterP->animSky = animSky;
    }
    waterP->posMin = *posMin;
    waterP->posMax = *posMax;
    /* unk7C is never read by the game's code, so it keeps its offset name. */
    waterP->hiliteCol.r = waterP->hiliteCol.g = waterP->hiliteCol.b = 76;
    waterP->hiliteCol.a = 25;
    waterP->glowCol.r = waterP->glowCol.g = waterP->glowCol.b = waterP->glowCol.a = 255;
    waterP->padY = 5;
    waterP->texPos.x = waterP->texPos.y = waterP->texPos.z = 0;
    waterP->texScale.x = 1;
    waterP->texScale.y = 1;
    waterP->texScale.z = 1;
    for(i=0; i<3; i++) {
        for(j=0; j<2; j++) {
            waterP->texMtx[j][i] = waterTexMtx[j][i];
        }
    }
    layerNo += HU3D_LAYER_HOOK_POST;
    waterP->layerNo = layerNo;
    waterP->cameraBit = HU3D_CAM0;
    /* Reset this water-state slot when installing the water effect. */
    waterP->unk7C = 0;
    waterP->animWave = NULL;
    waterP->maxTime = 60;
    for(i=0; i<32; i++) {
        waterP->wave[i].time = -1;
    }
    Hu3DLayerHookSet(layerNo, WaterLayerHook);
}

/* Sets translation of the texture coordinates used to sample the water bump map. */
void Hu3DWaterTexPosSet(float posX, float posY, float posZ)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    waterP->texPos.x = posX;
    waterP->texPos.y = posY;
    waterP->texPos.z = posZ;
}

/* Sets scale of the texture coordinates used to sample the water bump map. */
void Hu3DWaterTexScaleSet(float scaleX, float scaleY, float scaleZ)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    waterP->texScale.x = scaleX;
    waterP->texScale.y = scaleY;
    waterP->texScale.z = scaleZ;
}

/* Sets the world-space Y offset of the framebuffer capture and restore quad; the final water
 * surface stays at its original height. */
void Hu3DWaterPadYSet(float padY)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    waterP->padY = padY;
}

/* Sets the water glow color and GX combine mode. */
void Hu3DWaterGlowSet(s16 glowMode, GXColor *glowCol)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    waterP->glowCol.r = glowCol->r;
    waterP->glowCol.g = glowCol->g;
    waterP->glowCol.b = glowCol->b;
    waterP->glowCol.a = glowCol->a;
    waterP->glowMode = glowMode;
}

/* Replaces the water's 2-by-3 indirect-texture distortion matrix. */
void Hu3DWaterIndTexMtxSet(float texMtx[2][3])
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    s16 i;
    s16 j;
    for(i=0; i<3; i++) {
        for(j=0; j<2; j++) {
            waterP->texMtx[j][i] = texMtx[j][i];
        }
    }
}

/* Sets the RGB intensity of the water highlight from a normalized level. */
void Hu3DWaterHiliteSet(float level)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    waterP->hiliteCol.r = waterP->hiliteCol.g = waterP->hiliteCol.b = level*255;
}

/* Sets the highlight alpha from a normalized transparency level. */
void Hu3DWaterHiliteTPLvlSet(float tpLvl)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    waterP->hiliteCol.a = tpLvl*255;
}

/* Selects which camera view renders the water effect. */
void Hu3DWaterCameraSet(u16 cameraBit)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    waterP->cameraBit = cameraBit;
}

/* Starts a free wave slot at pos; radius and radiusMax specify its initial and final quad
 * widths. */
void Hu3DWaterWaveCreate(HuVecF *pos, float radius, float radiusMax)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    s16 i;
    s16 j;
    /* On first use, load the wave texture and close the effect archive before checking for a
     * free wave slot. */
    if(!waterP->animWave) {
        waterP->animWave = HuSprAnimDataRead(EFFECT_ANM_water_wave);
        HuDataDirClose(DATA_effect);
    }
    for(i=0; i<HU3D_WAVE_MAX; i++) {
        if(waterP->wave[i].time == -1) {
            break;
        }
    }
    if(i == HU3D_WAVE_MAX) {
        return;
    }
    waterP->wave[i].time = 0;
    waterP->wave[i].pos = *pos;
    waterP->wave[i].radius = radius;
    waterP->wave[i].radiusMax = radiusMax;
    /* The selected slot index is overwritten; this initializes matrix entries for slots 0 and 1. */
    for(i=0; i<2; i++) {
        for(j=0; j<3; j++) {
            waterP->wave[i].texMtx[i][j] = waterWaveTexMtx[i][j];
        }
    }
}

/* Pre-water layer callback captures the current framebuffer into the water texture. */
static void CopyWaterFb(s16 layerNo)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    if(!(waterP->cameraBit & Hu3DCameraBit)) {
        return;
    }
    GXSetTexCopySrc(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT);
    if(waterP->mipMapF) {
        GXSetTexCopyDst(HU_FB_WIDTH/2, HU_FB_HEIGHT/2, GX_TF_RGB565, GX_TRUE);
    } else {
        GXSetTexCopyDst(HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, GX_FALSE);
    }
    GXCopyTex(waterP->fbWater, GX_FALSE);
}

static void DrawQuad(HuVecF *min, HuVecF *max);
static void DrawWave(HU3D_WATER *waterP, s16 waveNo);

/* Post-layer callback composites reflection, water surface, highlights, and active waves. */
static void WaterLayerHook(s16 layerNo)
{
    HU3D_WATER *waterP = &Hu3DWaterData;
    HU3D_CAMERA *cameraP = &Hu3DCamera[Hu3DCameraNo];
    Mtx lookAt;
    Mtx texTrans;
    Mtx texScale;
    Mtx texMtx;
    Mtx proj;
    Vec diff;
    Vec dir;
    Vec posMin;
    Vec posMax;
    s32 tevStage;
    float diffLen;
    s32 i;

    if(!(waterP->cameraBit & Hu3DCameraBit)) {
        return;
    }
    Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, GX_FALSE, waterP->fbDisp);
    Hu3DTexLoad(waterP->fbWater, waterP->fbWaterW, waterP->fbWaterH, GX_TF_RGB565, GX_CLAMP,
                GX_CLAMP, TRUE, GX_TEXMAP0);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    MTXConcat(proj, Hu3DCameraMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
    GXSetTexCoordGen(GX_TEXMAP0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXMAP0, GX_TEXCOORD0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXLoadPosMtxImm(Hu3DCameraMtx, GX_PNMTX0);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(GX_TRUE, GX_GREATER, GX_FALSE);
    posMin = waterP->posMin;
    posMax = waterP->posMax;
    posMin.y += waterP->padY;
    posMax.y += waterP->padY;
    DrawQuad(&posMin, &posMax);
    Hu3DFbCopyExec(0, 0, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, waterP->mipMapF, waterP->fbWater);
    Hu3DTexLoad(waterP->fbDisp, HU_FB_WIDTH, HU_FB_HEIGHT, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, TRUE,
                GX_TEXMAP0);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    DrawQuad(&posMin, &posMax);
    Hu3DTexLoad(waterP->fbWater, waterP->fbWaterW, waterP->fbWaterH, GX_TF_RGB565, GX_CLAMP,
                GX_CLAMP, TRUE, GX_TEXMAP0);
    HuSprTexLoad(waterP->animBump, 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    if(waterP->animSky) {
        HuSprTexLoad(waterP->animSky, 0, GX_TEXMAP2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
    if(waterP->animSurface) {
        HuSprTexLoad(waterP->animSurface, 0, GX_TEXMAP3, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
    tevStage = 0;
    MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    MTXConcat(proj, Hu3DCameraMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen(tevStage, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetTevOrder(tevStage, GX_TEXMAP0, GX_TEXCOORD0, GX_COLOR0);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColor(GX_TEVREG0, waterP->hiliteCol);
    if(waterP->animSky) {
        tevStage++;
        VECSubtract(&cameraP->target, &cameraP->pos, &diff);
        diffLen = VECMag(&diff);
        dir.x = 0;
        dir.y = -1;
        dir.z = -0.3f;
        VECNormalize(&dir, &dir);
        VECHalfAngle(&diff, &dir, &diff);
        VECScale(&diff, &diff, diffLen);
        VECAdd(&cameraP->target, &diff, &dir);
        MTXLookAt(lookAt, &dir, &cameraP->up, &cameraP->target);
        MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
        MTXConcat(proj, lookAt, texMtx);
        GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
        GXSetTexCoordGen(tevStage, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);
        GXSetTevOrder(tevStage, GX_TEXMAP1, GX_TEXCOORD2, GX_COLOR0);
        GXSetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
        GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    if(waterP->glowMode & 0xF) {
        tevStage++;
        GXSetTevColor(GX_TEVREG1, waterP->glowCol);
        GXSetTevOrder(tevStage, GX_TEXMAP_NULL, GX_TEXCOORD_NULL, GX_COLOR0);
        if(waterP->glowMode & 0x1) {
            GXSetTevColorIn(tevStage, GX_CC_C1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
            GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        } else if(waterP->glowMode & 0x2) {
            GXSetTevColorIn(tevStage, GX_CC_C1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
            GXSetTevColorOp(tevStage, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        } else if(waterP->glowMode & 0x4) {
            GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_C1, GX_CC_CPREV, GX_CC_ZERO);
            GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        } else if(waterP->glowMode & 0x8) {
            GXSetTevColorIn(tevStage, GX_CC_C1, GX_CC_CPREV, GX_CC_A1, GX_CC_ZERO);
            GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        }
        GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    if(waterP->animSurface) {
        tevStage++;
        GXSetTevOrder(tevStage, GX_TEXMAP2, GX_TEXCOORD3, GX_COLOR0);
        GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_TEXC, GX_CC_A0, GX_CC_CPREV);
        GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    tevStage++;
    GXSetNumTexGens(3);
    GXSetNumTevStages(tevStage);
    MTXTrans(texTrans, waterP->texPos.x, waterP->texPos.y, waterP->texPos.z);
    MTXScale(texScale, waterP->texScale.x, waterP->texScale.y, waterP->texScale.z);
    MTXConcat(texScale, texTrans, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX2, GX_MTX2x4);
    GXSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, waterP->texMtx, -1);
    if(waterP->animSky) {
        GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD2, GX_TEXMAP1);
        GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
        GXSetTevIndWarp(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_TRUE, GX_FALSE, GX_ITM_0);
        GXSetNumIndStages(2);
    } else {
        GXSetNumIndStages(1);
    }
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXLoadPosMtxImm(Hu3DCameraMtx, GX_PNMTX0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    DrawQuad(&waterP->posMin, &waterP->posMax);
    for(i=0; i<HU3D_WAVE_MAX; i++) {
        if(waterP->wave[i].time != -1) {
            DrawWave(waterP, i);
        }
    }
    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
}

/* Initializes and binds a texture object, selecting linear or nearest filtering. */
void Hu3DTexLoad(void *buf, s16 w, s16 h, u32 format, GXTexWrapMode wrapS, GXTexWrapMode wrapT,
                 BOOL filterF, GXTexMapID texMapId)
{
    GXTexObj texObj;
    GXInitTexObj(&texObj, buf, w, h, format, wrapS, wrapT, GX_FALSE);
    GXInitTexObjLOD(&texObj, (filterF) ? GX_LINEAR : GX_NEAR, (filterF) ? GX_LINEAR : GX_NEAR, 0, 0,
                    0, GX_FALSE, GX_FALSE, GX_ANISO_1);
    GXLoadTexObj(&texObj, texMapId);
}

/* Emits a textured quad across the supplied X/Z bounds, using min->y at min->x and max->y at
 * max->x. */
static void DrawQuad(HuVecF *min, HuVecF *max)
{
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetZCompLoc(GX_TRUE);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(min->x, min->y, min->z);
    GXTexCoord2f32(0, 0);
    GXPosition3f32(max->x, max->y, min->z);
    GXTexCoord2f32(1, 0);
    GXPosition3f32(max->x, max->y, max->z);
    GXTexCoord2f32(1, 1);
    GXPosition3f32(min->x, min->y, max->z);
    GXTexCoord2f32(0, 1);
    GXEnd();
}

/* Called by WaterLayerHook for each selected camera to draw a wave, advance its age even during
 * pause, and free its slot when age exceeds maxTime. */
static void DrawWave(HU3D_WATER *waterP, s16 waveNo)
{
    HU3D_WATERWAVE *waveP = &waterP->wave[waveNo];
    HU3D_CAMERA *cameraP = &Hu3DCamera[Hu3DCameraNo];
    s32 tevStage;
    Mtx lookAt;
    Mtx texTrans;
    Mtx texScale;
    Mtx texMtx;
    Mtx proj;
    Vec diff;
    Vec dir;
    HuVecF min;
    HuVecF max;
    float size;
    float len;

    GXColor color;

    Hu3DTexLoad(waterP->fbWater, waterP->fbWaterW, waterP->fbWaterH, GX_TF_RGB565, GX_CLAMP,
                GX_CLAMP, TRUE, GX_TEXMAP0);
    HuSprTexLoad(waterP->animWave, 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    if(waterP->animSky) {
        HuSprTexLoad(waterP->animSky, 0, GX_TEXMAP2, GX_REPEAT, GX_REPEAT, GX_LINEAR);
    }
    tevStage = 0;
    MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    MTXConcat(proj, Hu3DCameraMtx, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen(tevStage, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0);
    GXSetTevOrder(tevStage, GX_TEXMAP0, GX_TEXCOORD0, GX_COLOR0);
    GXSetTevColorIn(tevStage, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevColor(GX_TEVREG0, waterP->hiliteCol);
    if(waterP->animSky) {
        tevStage++;
        VECSubtract(&cameraP->target, &cameraP->pos, &diff);
        len = VECMag(&diff);
        dir.x = 0;
        dir.y = -1;
        dir.z = -0.3f;
        VECNormalize(&dir, &dir);
        VECHalfAngle(&diff, &dir, &diff);
        VECScale(&diff, &diff, len);
        VECAdd(&cameraP->target, &diff, &dir);
        MTXLookAt(lookAt, &dir, &cameraP->up, &cameraP->target);
        MTXLightPerspective(proj, cameraP->fov, cameraP->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
        MTXConcat(proj, lookAt, texMtx);
        GXLoadTexMtxImm(texMtx, GX_TEXMTX1, GX_MTX3x4);
        GXSetTexCoordGen(tevStage, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX1);
        GXSetTevOrder(tevStage, GX_TEXMAP1, GX_TEXCOORD2, GX_COLOR0);
        GXSetTevColorIn(tevStage, GX_CC_CPREV, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
        GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_KONST);
        GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    }
    tevStage++;
    if(waveP->time > 0.6*waterP->maxTime) {
        color.a = 230*(1-((waveP->time-(0.6*waterP->maxTime))/(0.4*waterP->maxTime)));
    } else {
        color.a = 230;
    }
    color.r = color.g = color.b = 16;
    GXSetTevColor(GX_TEVREG1, color);
    GXSetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
    GXSetTevOrder(tevStage, GX_TEXMAP2, GX_TEXCOORD1, GX_COLOR0);
    GXSetTevColorIn(tevStage, GX_CC_C1, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    GXSetTevColorOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(tevStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_A1, GX_CA_ZERO);
    GXSetTevAlphaOp(tevStage, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    tevStage++;
    GXSetNumTexGens(4);
    GXSetNumTevStages(tevStage);
    MTXTrans(texTrans, waterP->texPos.x, waterP->texPos.y, waterP->texPos.z);
    MTXScale(texScale, waterP->texScale.x, waterP->texScale.y, waterP->texScale.z);
    MTXConcat(texScale, texTrans, texMtx);
    GXLoadTexMtxImm(texMtx, GX_TEXMTX3, GX_MTX2x4);
    /* Wave sampling uses matrix 2 left by WaterLayerHook; the matrix just loaded into matrix 3
     * is not selected here. */
    GXSetTexCoordGen(GX_TEXCOORD3, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX2);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD3, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_0);
    GXSetIndTexMtx(GX_ITM_0, waveP->texMtx, -1);
    if(waterP->animSky) {
        GXSetIndTexOrder(GX_INDTEXSTAGE1, GX_TEXCOORD3, GX_TEXMAP1);
        GXSetIndTexCoordScale(GX_INDTEXSTAGE1, GX_ITS_1, GX_ITS_1);
        GXSetTevIndWarp(GX_TEVSTAGE1, GX_INDTEXSTAGE1, GX_TRUE, GX_FALSE, GX_ITM_0);
        GXSetNumIndStages(2);
    } else {
        GXSetNumIndStages(1);
    }
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXLoadPosMtxImm(Hu3DCameraMtx, GX_PNMTX0);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetNumChans(0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 0, GX_DF_CLAMP, GX_AF_NONE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    len = HuSin(((float)waveP->time/waterP->maxTime)*90.0);
    max = waveP->pos;
    min = max;
    size = (0.5*waveP->radius)+(0.5*(len*(waveP->radiusMax-waveP->radius)));
    min.x -= size;
    min.z -= size;
    max.x += size;
    max.z += size;
    DrawQuad(&min, &max);
    waveP->time++;
    if(waveP->time > waterP->maxTime) {
        waveP->time = -1;
    }
}

typedef struct ModelDieWork_s {
    HU3D_MODELID modelId; /* Model whose motion controls its automatic lifetime. */
    u32 memoryFileId; /* Memory-file identity captured when the model's lifetime process is
                       * created. */
} MODEL_DIE_WORK;

static void ModelDieFunc(void);

/* Creates a model whose process kills it after its motion ends. */
HU3D_MODELID Hu3DModelDieCreate(void *data)
{
    HU3D_MODELID modelId = Hu3DModelCreate(data);
    MODEL_DIE_WORK *work;
    HUPROCESS *process;
    HU3D_MODEL *modelP;

    modelP = &Hu3DData[modelId];
    Hu3DModelAttrSet(modelId, HU3D_ATTR_DIE);
    modelP->hookData = process = HuPrcCreate(ModelDieFunc, 100, 4096, 0);
    process->property = work =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(MODEL_DIE_WORK), HU_MEMNUM_OVL);
    work->modelId = modelId;
    work->memoryFileId = HuMemMemoryFileGet(Hu3DData[modelId].hsf);
    return modelId;
}

/* Links a model and gives the instance the same motion-ended lifetime behavior. */
HU3D_MODELID Hu3DModelLinkDieCreate(HU3D_MODELID linkMdlId)
{
    HU3D_MODELID modelId = Hu3DModelLink(linkMdlId);
    MODEL_DIE_WORK *work;
    HUPROCESS *process;
    HU3D_MODEL *modelP;

    modelP = &Hu3DData[modelId];
    Hu3DModelAttrSet(modelId, HU3D_ATTR_DIE);
    modelP->hookData = process = HuPrcCreate(ModelDieFunc, 100, 4096, 0);
    process->property = work =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(MODEL_DIE_WORK), HU_MEMNUM_OVL);
    work->modelId = modelId;
    work->memoryFileId = HuMemMemoryFileGet(Hu3DData[modelId].hsf);
    return modelId;
}

/* Frees the lifetime process data and stops the process attached to the model. */
void Hu3DModelDieKill(HU3D_MODELID modelId)
{
    HU3D_MODEL *modelP = &Hu3DData[modelId];
    HUPROCESS *process = modelP->hookData;
    HuMemDirectFree(process->property);
    HuPrcKill(process);
}

/* Process watches the model motion and file identity, then destroys the model on completion. */
static void ModelDieFunc(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    MODEL_DIE_WORK *work = process->property;
    HU3D_MODELID modelId = work->modelId;
    u32 memoryFileId = work->memoryFileId;
    while(1) {
        if(!Hu3DData[modelId].hsf) {
            break;
        }
        if(memoryFileId != HuMemMemoryFileGet(Hu3DData[modelId].hsf)) {
            break;
        }
        if(Hu3DMotionEndCheck(modelId)) {
            Hu3DModelKill(modelId);
            while(1) {
                HuPrcVSleep();
            }
        } else {
            HuPrcVSleep();
        }
    }
    HuMemDirectFree(work);
    HuPrcEnd();
    while(1) {
        HuPrcVSleep();
    }
}
