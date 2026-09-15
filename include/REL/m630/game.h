#ifndef M630_GAME_H
#define M630_GAME_H

/* Field names remain unknown; word accesses and the 12-byte allocation agree. */
typedef struct M630State {
    int unk_0;
    int unk_4;
    int unk_8;
} M630State;

/* Allocation stride and fields are independently visible in fn_1_1EB0. */
typedef struct M630Player {
    s16 charNo;
    s16 modelId;
    s16 motionId[9];
    s16 currentMotion;
    HuVecF pos;
    float rotationY;
    int padNo;
    int unk_2C;
    int unk_30;
} M630Player;

typedef struct M630Com {
    int difficulty;
    int type;
    int unk_8;
    int unk_C;
    int unk_10;
    int unk_14;
} M630Com;

/* Consumed layout: three records, 15 indexed models and vectors per record.
 * Natural alignment accounts for +0x1e and +0x172; no filler objects. */
typedef struct M630ModelRow {
    s16 model[15];
    HuVecF pos[15];
    float tpLvl[15];
    int itemState[15];
    int phaseState;
    int currentIndex;
    s16 secondaryModel[15];
    int playerIndex[15];
} M630ModelRow;

/* The target uses only the named scalar fields of this 32-byte record.
 * The two remaining intervals have no recovered purpose. */
typedef struct M630MotionRecord {
    s16 model;
    s16 motionA;
    s16 motionB;
    unsigned char unknown_06[18];
    int motionState;
    unsigned char unknown_1C[4];
} M630MotionRecord;

typedef struct M630MovingModel {
    s16 model;
    unsigned char unknown_02[2];
    s16 secondaryModel;
    HuVecF pos;
    int direction;
} M630MovingModel;

typedef struct M630RotatingModel {
    s16 model;
    float rotation;
} M630RotatingModel;

/* Consumed view of twelve bytes at BSS+0x744. Only the first float is
 * referenced; the remaining bytes' original declaration is unknown. */
typedef struct M630Bss744 {
    float unk_0;
    unsigned char unknown_04[8];
} M630Bss744;

extern HUPROCESS *lbl_1_bss_0;
extern int lbl_1_bss_8;
extern int lbl_1_bss_C[3];
extern int lbl_1_bss_170;
extern int lbl_1_bss_824;
extern M630State lbl_1_bss_828;
extern M630Player lbl_1_bss_754[4];
extern M630Com lbl_1_bss_174[4];
extern M630ModelRow lbl_1_bss_1D4[3];
extern M630MotionRecord lbl_1_bss_28[10];
extern M630MovingModel lbl_1_bss_6FC[3];
extern M630RotatingModel lbl_1_bss_6E4[3];
extern HuVecF lbl_1_data_4C[4];
extern float lbl_1_data_7C[4][2];

void fn_1_1EB0(void);
void fn_1_25E0(void);
int fn_1_24F0(void);
void fn_1_75C8(void);
void fn_1_78A4(M630Player *player);
void fn_1_7C34(Mtx matrix, int cameraId);
int fn_1_7EF4(HuVecF *src, HuVecF *dst);
s16 fn_1_7724(s16 effect, s16 model, s16 camera);
int fn_1_3B2C(int player, float targetX);

#endif
