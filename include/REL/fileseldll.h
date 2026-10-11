/* Shared File Select slot, keyboard, and save-window declarations. */
#ifndef _FILESELDLL_H
#define _FILESELDLL_H

#include "game/hu3d.h"
#include "game/window.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include "game/card.h"
#include "game/process.h"

#include "datanum/filesel.h"

#define FILESEL_RESULT_CANCEL -1234
#define FILESEL_FILENAME_LENGTH 17
#define FILECARD_MESSID_NONE -1

enum {
    FILECARD_FLAG_NOSAVE = 1 << 0,
    FILECARD_FLAG_FORMAT = 1 << 1,
    FILECARD_FLAG_RETRY = 1 << 2,
    FILECARD_FLAG_MENU = 1 << 3,
    FILECARD_FLAG_CANCEL = 1 << 4,
    FILECARD_FLAG_ALL = FILECARD_FLAG_NOSAVE | FILECARD_FLAG_FORMAT |
                        FILECARD_FLAG_RETRY | FILECARD_FLAG_MENU |
                        FILECARD_FLAG_CANCEL,
    FILE_MESS_NOCARD = 0,
    FILE_MESS_FATAL_ERROR,
    FILE_MESS_NOENT,
    FILE_MESS_INSSPACE,
    FILE_MESS_FULL,
    FILE_MESS_BROKEN,
    FILE_MESS_FORMAT_ERROR,
    FILE_MESS_WRONG_DEVICE,
    FILE_MESS_INVALID,
    FILE_MESS_SERIAL_INVALID,
    FILE_MESS_NOSAVE_CHOICE,
    FILE_MESS_REINSERT,
    FILE_MESS_FORMAT_UNMOUNT,
    FILESEL_WIPE_DURATION = 60,
    FILESEL_AUDIO_FADE_DURATION = 1000,
};

typedef struct FileselBox_s {
    /* Nonzero when the panel is marked occupied; scene setup also uses this for placeholders. */
    s32 hasSave;
    /* Sprite group used to draw the slot's text and card details. */
    HUSPR_GROUPID spriteGroup;
    /* Window used for this slot's text. */
    HUWINID window;
    /* 3D model of the save slot. */
    HU3D_MODELID slotModel;
    /* 3D model that displays the player name. */
    HU3D_MODELID textModel;
    /* First status model used for slot decoration. */
    HU3D_MODELID stateModel1;
    /* Second status model used for slot decoration. */
    HU3D_MODELID stateModel2;
    /* Third status model used for slot decoration. */
    HU3D_MODELID stateModel3;
    /* Current visual state of the slot animation. */
    s16 visualState;
    /* Saved board pattern variant; -1 for a new save or an empty slot. */
    s16 patternVariant;
    /* Saved star count shown on the slot. */
    s16 displayNumber;
    /* Animation applied to the slot frame. */
    HU3D_ANIMID frameAnimation;
    /* Animation applied to the board pattern. */
    HU3D_ANIMID patternAnimation;
    /* Calendar date copied from the save data. */
    OSCalendarTime saveTime;
    /* Null-terminated player name copied from the save data. */
    char fileName[FILESEL_FILENAME_LENGTH];
    /* Animation data that presents the captured save-slot panel on the text model. */
    ANIMDATA *cardTextureAnim;
    /* Texture buffer receiving the rendered save-slot panel. */
    void *cardTextureBuffer;
    /* Decimal slot number text inserted into its message window. */
    u8 slotNumberText[2];
    /* Motion mode set by selection and transition flows and read by the slot's child process. */
    u8 motionState;
    /* Base slot position in model coordinates, before waveOffset is added. */
    Vec basePosition;
    /* Per-frame wave displacement in model coordinates. */
    Vec waveOffset;
    /* Child process that updates this slot's motion. */
    HUPROCESS *motionProcess;
} FILESEL_BOX;

typedef struct NameKeyboard_s {
    /* Key center X position in screen pixels. */
    s16 x;
    /* Key center Y position in screen pixels. */
    s16 y;
    /* Character code or page/delete/confirm command. */
    u16 val;
} NAMEKEYBOARD;

extern FILESEL_BOX lbl_1_bss_D8[];
extern HUWINID lbl_1_bss_2DE;
extern HUWINID lbl_1_bss_2E0;
extern HUWINID lbl_1_bss_3AC[];
extern HUSPR_GROUPID lbl_1_bss_42E;
extern NAMEKEYBOARD *keyboard;
extern char nameMes[];

void fn_1_36C4(s16 boxNo, s16 slotNo);

void FileCommonInit(void);
s32 FileBoxInit(HUWINID sourceWindow);
s32 FileTestOpen(void);
s32 FileClear(HUWINID sourceWindow);
s32 FileSaveMesOpen(HUWINID sourceWindow, s32 message);
HUWINID FileCardMesOpen(u32 message, u32 insertMessage1, u32 insertMessage2, s16 posY);
void FileCardMesClose(HUWINID window);
HUWINID FileStatusMesOpen(u32 message, u32 insertMessage1, u32 insertMessage2, s16 posY);
void FileStatusMesClose(HUWINID window);

void NameEnterInit(void);
s32 NameEnterMain(s16 boxNo);
void NameEnterClose(void);

#endif
