/* Shared player, object, and animation state for Stage Fright. */
#ifndef M630_GAME_H
#define M630_GAME_H

/* Per-minigame frame and result state shared by the play and result callbacks. */
typedef struct M630State {
    int frameCount; /* Frames elapsed in the current play or result stage. */
    int actionTimer; /* Counts down scripted movement, or stores a result-phase step. */
    int resultState; /* Counts frames after all three opponent rows reach their result positions;
                      * starts at 0. */
} M630State;

/* Character model, position, controller, motion, and hit-reaction state. */
typedef struct M630Player {
    s16 charNo; /* Character slot used by the character-motion API. */
    s16 modelId; /* 3D model handle for this character. */
    s16 motionId[9]; /* Idle, movement, hit, victory, and action motion handles. */
    s16 currentMotion; /* Motion handle at setup; movement code then stores 0 idle, 1 moving, or 2
                        * fast moving. */
    HuVecF pos; /* Character position in world units. */
    float rotationY; /* Character heading in degrees. */
    int padNo; /* Controller assigned to this character. */
    int hitState; /* 0 active, -1 hit, -2 recovery, -3 human hit motion, 1 blink recovery. */
    int hitTimerFrames; /* Frames spent in the opponent recovery or blink phase. */
} M630Player;

/* Computer-controlled player settings and its action-selection state. */
typedef struct M630Com {
    int difficulty; /* Computer difficulty index, 0 through 3. */
    int type; /* 0 for the human; nonzero enables computer action selection. */
    int unusedWord; /* Initialized to zero and not read by this minigame. */
    int unusedWord2; /* Initialized to zero and not read by this minigame. */
    int moveDirection; /* Horizontal drift direction selected by the easy-computer routine. */
    int moveTimerFrames; /* Remaining frames in that horizontal drift. */
} M630Com;

/* Three opponent rows, each with fifteen moving objects and effect state. */
typedef struct M630ModelRow {
    s16 model[15]; /* Main moving-object model handles. */
    HuVecF pos[15]; /* Object positions in world units. */
    float transparency[15]; /* Current translucency level, from 0.0 to 1.0. */
    int itemState[15]; /* 0 idle, 1 traveling, 2 queued, -1 hit, -2 passed player. */
    int phaseState; /* Opponent action animation phase, 0 when idle. */
    int currentIndex; /* Next object slot to launch, in the range 0 through 14. */
    s16 secondaryModel[15]; /* Companion effect model handles. */
    int playerIndex[15]; /* Character slot that hit an object. */
} M630ModelRow;

/* One decorative model and its two motions and alternate-motion state. */
typedef struct M630MotionRecord {
    s16 model; /* Decorative model handle. */
    s16 motionA; /* Base looping motion. */
    s16 motionB; /* Brief alternate motion. */
    unsigned char unusedBytes[18]; /* Unread bytes retained in the record. */
    int motionState; /* 0 for base motion, 1 while returning from the alternate motion. */
    unsigned char unusedTail[4]; /* Unread trailing bytes retained in the record. */
} M630MotionRecord;

typedef struct M630MovingModel {
    s16 model; /* Moving platform or row model handle. */
    unsigned char reservedBytes[2]; /* Alignment bytes not read by the minigame. */
    s16 secondaryModel; /* Side decoration model handle. */
    HuVecF pos; /* Moving row position in world units. */
    int direction; /* 0 moves toward +X; 1 moves toward -X. */
} M630MovingModel;

typedef struct M630RotatingModel {
    s16 model; /* Spinner model handle. */
    float rotation; /* Rotation around the X axis in degrees. */
} M630RotatingModel;

/* Direction and retained bytes used by player 0's brief slide animation on a loss. */
typedef struct M630Bss744 {
    float losingPlayerSlideStep; /* Horizontal movement per result callback, in world units. */
    unsigned char unusedBytes[8]; /* Unread bytes retained in the record. */
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
