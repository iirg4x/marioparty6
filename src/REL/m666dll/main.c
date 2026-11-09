/* Starts and advances the Shoot Yer Mouth Off minigame sequence. */
#include "REL/m666dll/state.h"
#include "REL/m666dll/player.h"
#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "game/hu3d.h"
#include "game/mic.h"
#include "REL/m666dll/recovered.h"

M666SoundWork lbl_1_bss_DC[9];
M666Bss98Work lbl_1_bss_98;
M666PlayerRecord lbl_1_bss_8[3];
HUPROCESS *lbl_1_bss_4;
u16 lbl_1_bss_2;
s16 lbl_1_bss_0;

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_D8(void);
void fn_1_144(void *sequenceMode, s16 frameNo);
void fn_1_190(s16 mode, s16 frame);
void fn_1_1D0(void *sequenceMode, s16 frameNo);
void fn_1_244(void *sequenceMode, short frameNo);
void fn_1_2B4(void);
void fn_1_2DC(s16 mode, s16 frameNo);
void fn_1_2FC(void);
void fn_1_300(void);

/* The module loader calls this to run constructors and start the minigame sequence. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* The module loader calls this when unloading to run the module destructors. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

MGSEQ_PARAM lbl_1_data_0 = {
    60, 1, (MGSEQ_FUNC)fn_1_D8, (MGSEQ_FUNC)fn_1_144,
    (MGSEQ_FUNC)fn_1_190, (MGSEQ_FUNC)fn_1_1D0, (MGSEQ_FUNC)fn_1_244,
    (MGSEQ_FUNC)fn_1_2B4, (MGSEQ_FUNC)fn_1_2DC, (MGSEQ_FUNC)fn_1_2FC,
    (MGSEQ_FUNC)fn_1_300
};

void fn_1_A0(void) {
    lbl_1_bss_4 = fn_80080418();
    fn_80070788(&lbl_1_data_0);
}

extern void fn_1_8BC(void);
char lbl_1_data_28[20] = "/mic/ctx/m666_words";

/* Opens the microphone selection and starts the play scene on the sequence start callback. */
void fn_1_D8(void) {
    HuMCInit(0);
    HuMCThresholdSet(100);
    HuMCSelWinCreate(-10001.0f, -10002.0f);
    lbl_1_bss_0 = HuMCContextCreate(lbl_1_data_28);
    fn_1_8BC();
    fn_80071A9C();
    fn_800856A8();
}

/* Updates the microphone choice on the selection callback, then advances its sequence. */
void fn_1_144(void *sequenceMode, s16 frameNo)
{
    if (frameNo == 0) {
        HuMCSelWinItemSet(MESSNUM(MESS_MIC_MIN_665, 4),
                          9,
                          lbl_1_bss_98.padNo);
    }
    fn_1_4AC();
}

/* Starts the minigame music when the sequence's music callback reaches frame zero. */
void fn_1_190(s16 mode, s16 frame)
{
    if (frame == 0) {
        lbl_1_bss_DF0.soundHandle = HuAudBGMPlay(MSM_STREAM_JAZZY);
    }
}

extern void fn_1_4D20(void);
extern void fn_1_3C9C(void);
extern s16 fn_1_D7DC(void);

/* Registers the microphone response callback on the first gameplay frame and advances the scene. */
void fn_1_1D0(void *sequenceMode, s16 frameNo) {
    if (frameNo == 0) {
        HuMCListenerCreate(lbl_1_bss_0, (void (*)(u16 *))fn_1_324,
                            (u8)lbl_1_bss_98.padNo);
    }
    fn_1_4D20();
    fn_1_3C9C();
    fn_800856A8();
    if (fn_1_D7DC() != 0) {
        fn_80071A9C();
    }
}

extern void fn_1_92C(void);

/* Clears listener and sequence state on the cleanup callback before updating actors. */
void fn_1_244(void *sequenceMode, short frameNo) {
    if (frameNo == 0) {
        HuAudSStreamFadeOut(lbl_1_bss_DF0.soundHandle, 100);
        HuMCListenerKill();
        lbl_1_bss_DF0.sequenceState = 0;
        lbl_1_bss_DF0.timer = 0;
        fn_1_92C();
    }
    fn_1_3C9C();
    fn_800856A8();
}

void fn_1_78C(void);

/* Advances the result sequence, player movement, and minigame actors each pre-winner frame. */
void fn_1_2B4(void) {
    fn_1_78C();
    fn_1_3C9C();
    fn_800856A8();
}

void fn_1_2DC(s16 mode, s16 frameNo) {
    fn_1_3C9C();
}

void fn_1_2FC(void) {
}

void fn_1_300(void) {
    HuMCResponseGet();
    HuMCClose();
}

typedef struct M666RecognizedWordIndex {
    s16 firstWordIndex;
} M666RecognizedWordIndex;

struct M666MicrophoneResponse {
    s16 status;
    u16 recognitionScore;
    s16 wordCount;
    M666RecognizedWordIndex *recognizedWords;
    s32 word;
};

/* Handles the microphone listener response: scores below 3000 select event 100; otherwise the first
 * recognized word selects events 1-10. */
void fn_1_324(M666MicrophoneResponse *response) {
    if (response->status != 0 || response->wordCount == 0) {
        return;
    }

    if (response->recognitionScore < 3000) {
        lbl_1_bss_2 = 100;
        return;
    }

    lbl_1_bss_2 = 0;
    switch (response->recognizedWords->firstWordIndex) {
    case 0: lbl_1_bss_2 = 1; break;
    case 1: lbl_1_bss_2 = 2; break;
    case 2: lbl_1_bss_2 = 3; break;
    case 3: lbl_1_bss_2 = 4; break;
    case 4: lbl_1_bss_2 = 5; break;
    case 5: lbl_1_bss_2 = 6; break;
    case 6: lbl_1_bss_2 = 7; break;
    case 7: lbl_1_bss_2 = 8; break;
    case 8: lbl_1_bss_2 = 9; break;
    case 9: lbl_1_bss_2 = 10; break;
    default: lbl_1_bss_2 = 0; break;
    }
    OSReport("Result %d\n", response->word);
}

void fn_1_17C8(s16 motionIndex, f32 blend, u32 flags);
void fn_1_1004(s16 motionIndex, f32 blend);
void fn_1_9ED8(void);
void fn_1_B30C(void);
void fn_1_7CA0(void);
void fn_1_7D24(void);
void fn_1_7E9C(void);
s16 fn_1_1080(void);
void fn_1_143C(void);
void fn_1_1680(void);
void fn_1_8164(void);
void fn_1_F8C(void);
void fn_1_80E0(void);

/* Advances the microphone selection sequence from fn_1_144, including its timed effects. */
void fn_1_4AC(void) {
    s16 effectIds[14] = { MSM_SE_CHARVOICE_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_LUIGI_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_PEACH_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_YOSHI_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_WARIO_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_DAISY_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_WALUIGI_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_KINOPIO_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_TERESA_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_MINIKOOPA_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_KINOPIKO_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_MINIKOOPAR_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_MINIKOOPAG_HAPPY_CHEER,
                          MSM_SE_CHARVOICE_MINIKOOPAB_HAPPY_CHEER };

    switch (lbl_1_bss_DF0.sequenceState) {
    case 0:
        fn_80071B08();
        lbl_1_bss_DF0.sequenceState++;
        return;
    case 1:
        fn_1_17C8(11, 8.0f, ((1 << 30) | (1 << 0)));
        lbl_1_bss_DF0.sequenceState++;
        return;
    case 2:
        if (lbl_1_bss_DF0.timer++ < 80) {
            return;
        }
        HuAudFXPlay(effectIds[lbl_1_bss_98.charNo]);
        fn_1_1004(2, 8.0f);
        fn_1_9ED8();
        fn_1_B30C();
        lbl_1_bss_DF0.sequenceState++;
        lbl_1_bss_DF0.timer = 0;
        return;
    case 3:
        if (lbl_1_bss_DF0.timer++ >= 60) {
            fn_1_17C8(10, 8.0f, 0);
            lbl_1_bss_DF0.sequenceState++;
            lbl_1_bss_DF0.timer = 0;
        }
        if (lbl_1_bss_DF0.timer == 40) {
            fn_1_7CA0();
        }
        if (lbl_1_bss_DF0.timer == 30) {
            HuAudFXPlay(MSM_SE_MG666_LANE_CENTER_CROSSING);
            HuAudFXPlay(MSM_SE_MG666_BOMB_HOOK);
            return;
        }
        break;
    case 4:
        if (lbl_1_bss_DF0.timer++ > 60) {
            lbl_1_bss_DF0.sequenceState++;
            lbl_1_bss_DF0.timer = 0;
        }
        if (lbl_1_bss_DF0.timer == 45) {
            fn_1_17C8(0, 8.0f, ((1 << 30) | (1 << 0)));
            return;
        }
        break;
    case 5:
        fn_1_7D24();
        lbl_1_bss_DF0.sequenceState = 0;
        lbl_1_bss_DF0.timer = 0;
        fn_80071A9C();
        break;
    }
}

/* fn_1_2B4 calls this after gameplay to turn the models and select result motions and winners. */
void fn_1_78C(void) {
    switch (lbl_1_bss_DF0.sequenceState) {
    case 0:
        fn_80071B08();
        if (lbl_1_bss_DF0.microphonePlayerWon == 0) {
            fn_1_17C8(1, 8.0f, ((1 << 30) | (1 << 0)));
        }
        lbl_1_bss_DF0.sequenceState++;
        break;
    case 1:
        fn_1_7E9C();
        if (lbl_1_bss_DF0.microphonePlayerWon == 0) {
            fn_1_143C();
        }
        if (fn_1_1080() != 0) {
            lbl_1_bss_DF0.sequenceState++;
        }
        break;
    case 2:
        if (lbl_1_bss_DF0.microphonePlayerWon == 0) {
            fn_1_1004(5, 8.0f);
            fn_1_1680();
            fn_1_8164();
        } else {
            fn_1_1004(4, 8.0f);
            fn_1_F8C();
            fn_1_80E0();
        }
        fn_80071A9C();
        break;
    }
}

void fn_1_F8C(void);

typedef struct {
    u8 unk_00[4]; /* No code names this field. */
    s16 rootModelId;
    u8 unk06[2]; /* No code names this field. */
    u8 particleStorage[96];
    s16 state;
    s16 status;
    u8 stateData[8];
    f32 initialAlpha;
} M666SequenceItemView;

typedef struct {
    M666SequenceItemView items[3];
} M666SequenceGroupView;

typedef struct {
    s16 modelId;
    s16 active;
    f32 scale;
    f32 alpha;
} M666SequenceModelView;

typedef struct {
    Point3d position;
} M666SequencePosition;

extern M666SequenceGroupView lbl_1_bss_148[3];
extern M666SequencePosition lbl_1_data_210[9];
extern f32 lbl_1_data_27C[9];

/* Creates the nine lane models and eight particle models for each lane. */
void fn_1_A1CC(void);

/* Places and activates the first available display model in a sequence item. */
void fn_1_A430(s16 groupIndex, s16 itemIndex);

void fn_1_B30C(void);

/* Starts the first lane with queue position zero when it is ready, with rumble and sound. */
void fn_1_B370(s16 groupIndex);
