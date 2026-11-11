#ifndef MICQUIZISHIDLL_SELECTION_SCENE_H
#define MICQUIZISHIDLL_SELECTION_SCENE_H

#include "game/object.h"
#include "game/sprite.h"

typedef struct QuizSelectionEntry {
    s32 slotIndex;
    s32 partnerIndex;
    s32 selected;
} QuizSelectionEntry;

typedef struct QuizCharacterModel {
    u8 unknown_00[4]; /* No code names this field. */
    s32 characterId;
    s32 state;
    u8 unknown_0C[4]; /* No code names this field. */
    s16 modelId;
    s16 motions[3];
    f32 positionX;
    f32 positionY;
    f32 positionZ;
} QuizCharacterModel;

typedef struct QuizMenuCharacterMotionData {
    s32 dataNumbers[3];
} QuizMenuCharacterMotionData;

typedef struct QuizCharacterEntry {
    s32 unknown_00;
    s32 voiceId;
    s32 unknown_08;
} QuizCharacterEntry;

typedef struct Rectangle {
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} Rectangle;

typedef struct TexturePair {
    ANIMDATA *animation[2];
} TexturePair;

typedef struct QuizSelectionTable {
    QuizSelectionEntry entries[20];
    u8 unknown_F0[4]; /* No code names this field. */
} QuizSelectionTable;

typedef struct QuizModelList {
    s16 models[20];
    u8 unknown_28[4]; /* No code names this field. */
} QuizModelList;

typedef struct QuizObjectStorage {
    OMOBJ *object;
    u8 unknown_04[4]; /* No code names this field. */
} QuizObjectStorage;

extern s16 lbl_1_bss_1900[0x14];
extern QuizModelList lbl_1_bss_1928;
extern QuizSelectionTable lbl_1_bss_1954;
extern s32 lbl_1_bss_1A48;
extern s16 lbl_1_bss_1A4C[0x14];
extern TexturePair lbl_1_bss_1A74[20];
extern s16 lbl_1_bss_1B14;
extern s16 lbl_1_bss_1B16;
extern OMOBJ *lbl_1_bss_1B18;
extern OMOBJ *lbl_1_bss_1B1C;
extern QuizObjectStorage lbl_1_bss_1B20;

#endif
