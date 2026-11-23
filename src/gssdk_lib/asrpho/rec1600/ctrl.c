/* Recognizer architecture control routes and state transitions. */
#include "gssdk/engine.h"

/* Routes an incoming command to 1 destination. */
struct _CNTRL1 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[2]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 2 destinations. */
struct _CNTRL2 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[3]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 3 destinations. */
struct _CNTRL3 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[4]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 4 destinations. */
struct _CNTRL4 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[5]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 5 destinations. */
struct _CNTRL5 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[6]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 6 destinations. */
struct _CNTRL6 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[7]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 8 destinations. */
struct _CNTRL8 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[9]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 9 destinations. */
struct _CNTRL9 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[10]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 10 destinations. */
struct _CNTRL10 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[11]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 12 destinations. */
struct _CNTRL12 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[13]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 13 destinations. */
struct _CNTRL13 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[14]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 14 destinations. */
struct _CNTRL14 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[15]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 15 destinations. */
struct _CNTRL15 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[16]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Routes an incoming command to 26 destinations. */
struct _CNTRL26 {
    u8 ControlID; /* Command that selects this route. */
    u8 SyncBlock; /* Runs the scheduler after forwarding a control when set. */
    u16 NextState; /* State or relative transition applied before dispatch. */
    ControlCallback CallBackFunc; /* Optional handler called after forwarding. */
    struct _CTRL OutputControl[27]; /* Destinations followed by a record with ControlID set to 0. */
};

/* Ignores an architecture flush in the current state. */
static const struct _CNTRL0 iARCHI_FLUSH_IGNORE = {
    0x03, 0x00, 0xFFFF, 0,
    {
        { 0x00, 0x00 },
    },
};

/* Flushes buffered input from sleep into recognition recovery. */
static const struct _CNTRL5 iARCHI_FLUSH_SLEEP_TORECOVER = {
    0x03, 0x28, 0x0005, 0,
    {
        { 0x01, 0x02 },
        { 0x07, 0x01 },
        { 0x0C, 0x01 },
        { 0x16, 0x65 },
        { 0x20, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Flushes buffered input from spelling sleep into spelling recovery. */
static const struct _CNTRL5 iARCHI_FLUSH_SLEEP_TOSPELLRECOVER = {
    0x03, 0x28, 0x0009, 0,
    {
        { 0x01, 0x02 },
        { 0x07, 0x01 },
        { 0x0C, 0x01 },
        { 0x16, 0x65 },
        { 0x20, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Flushes buffered input from isolated-word sleep into recovery. */
static const struct _CNTRL5 iARCHI_FLUSH_SLEEP_TOISOWORDRECOVER = {
    0x03, 0x28, 0x000D, 0,
    {
        { 0x01, 0x02 },
        { 0x07, 0x01 },
        { 0x0C, 0x01 },
        { 0x16, 0x65 },
        { 0x23, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Flushes the buffered utterance before recording. */
static const struct _CNTRL5 iARCHI_FLUSH_RECORDSLEEP = {
    0x03, 0x28, 0x0012, 0,
    {
        { 0x01, 0x02 },
        { 0x07, 0x01 },
        { 0x0C, 0x01 },
        { 0x16, 0x65 },
        { 0x21, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Flushes queued speech features and advances the architecture state. */
static const struct _CNTRL15 iARCHI_FLUSH = {
    0x03, 0x28, 0xFFFE, 0,
    {
        { 0x01, 0x02 },
        { 0x07, 0x01 },
        { 0x0C, 0x01 },
        { 0x0D, 0x02 },
        { 0x0E, 0x02 },
        { 0x0F, 0x02 },
        { 0x13, 0x03 },
        { 0x14, 0x02 },
        { 0x16, 0x65 },
        { 0x1A, 0xC8 },
        { 0x1C, 0xC9 },
        { 0x20, 0x04 },
        { 0x21, 0x04 },
        { 0x23, 0x04 },
        { 0x24, 0x03 },
        { 0x00, 0x00 },
    },
};

/* Returns to idle. */
static const struct _CNTRL0 iARCHI_GOTOIDLE = {
    0x07, 0x00, 0x0002, 0,
    {
        { 0x00, 0x00 },
    },
};

/* Prepares the speech blocks for detection and advances the state. */
static const struct _CNTRL10 iARCHI_GOTOSLEEP = {
    0x01, 0x00, 0xFFFE, 0,
    {
        { 0x20, 0x03 },
        { 0x23, 0x03 },
        { 0x24, 0x02 },
        { 0x01, 0x01 },
        { 0x0B, 0x07 },
        { 0x14, 0x01 },
        { 0x16, 0x64 },
        { 0x17, 0xC8 },
        { 0x1A, 0xC9 },
        { 0x1C, 0xC8 },
        { 0x00, 0x00 },
    },
};

/* Prepares the speech blocks for a spelling request. */
static const struct _CNTRL9 iARCHI_GOTOSPELLSLEEP = {
    0x06, 0x00, 0x0007, 0,
    {
        { 0x20, 0x03 },
        { 0x24, 0x02 },
        { 0x01, 0x01 },
        { 0x0B, 0x07 },
        { 0x14, 0x01 },
        { 0x16, 0x64 },
        { 0x17, 0xC8 },
        { 0x1A, 0xC9 },
        { 0x1C, 0xC8 },
        { 0x00, 0x00 },
    },
};

/* Prepares the speech blocks for isolated-word recognition. */
static const struct _CNTRL9 iARCHI_GOTOISOWORDSLEEP = {
    0x08, 0x00, 0x000B, 0,
    {
        { 0x23, 0x03 },
        { 0x24, 0x02 },
        { 0x01, 0x01 },
        { 0x0B, 0x07 },
        { 0x14, 0x01 },
        { 0x16, 0x64 },
        { 0x17, 0xC8 },
        { 0x1A, 0xC9 },
        { 0x1C, 0xC8 },
        { 0x00, 0x00 },
    },
};

/* Enables detection of the start of speech. */
static const struct _CNTRL10 iARCHI_START_DETECTION = {
    0x33, 0x00, 0xFFFF, 0,
    {
        { 0x27, 0x64 },
        { 0x0C, 0xFA },
        { 0x0D, 0xFA },
        { 0x0E, 0xFA },
        { 0x0F, 0xFA },
        { 0x17, 0xFA },
        { 0x19, 0xFA },
        { 0x1F, 0x18 },
        { 0x20, 0x26 },
        { 0x23, 0x26 },
        { 0x00, 0x00 },
    },
};

/* Disables detection of the start of speech. */
static const struct _CNTRL10 iARCHI_NO_START_DETECTION = {
    0x34, 0x00, 0xFFFF, 0,
    {
        { 0x27, 0x64 },
        { 0x0C, 0xF9 },
        { 0x0D, 0xF9 },
        { 0x0E, 0xF9 },
        { 0x0F, 0xF9 },
        { 0x17, 0xF9 },
        { 0x19, 0xF9 },
        { 0x1F, 0x17 },
        { 0x20, 0x25 },
        { 0x23, 0x25 },
        { 0x00, 0x00 },
    },
};

/* Disables the male model. */
static const struct _CNTRL10 iARCHI_DISABLE_M = {
    0x3D, 0x00, 0xFFFF, 0,
    {
        { 0x03, 0xFF },
        { 0x08, 0xFF },
        { 0x0D, 0xFF },
        { 0x10, 0xFF },
        { 0x2C, 0xFF },
        { 0x34, 0xFF },
        { 0x37, 0xFF },
        { 0x3D, 0xFF },
        { 0x31, 0xFF },
        { 0x51, 0xFF },
        { 0x00, 0x00 },
    },
};

/* Disables the female model. */
static const struct _CNTRL10 iARCHI_DISABLE_F = {
    0x3E, 0x00, 0xFFFF, 0,
    {
        { 0x04, 0xFF },
        { 0x09, 0xFF },
        { 0x0E, 0xFF },
        { 0x11, 0xFF },
        { 0x2D, 0xFF },
        { 0x35, 0xFF },
        { 0x38, 0xFF },
        { 0x3E, 0xFF },
        { 0x32, 0xFF },
        { 0x52, 0xFF },
        { 0x00, 0x00 },
    },
};

/* Disables the child model. */
static const struct _CNTRL10 iARCHI_DISABLE_C = {
    0x3F, 0x00, 0xFFFF, 0,
    {
        { 0x05, 0xFF },
        { 0x0A, 0xFF },
        { 0x0F, 0xFF },
        { 0x12, 0xFF },
        { 0x2E, 0xFF },
        { 0x36, 0xFF },
        { 0x39, 0xFF },
        { 0x3F, 0xFF },
        { 0x33, 0xFF },
        { 0x53, 0xFF },
        { 0x00, 0x00 },
    },
};

/* Disables the input for the male model. */
static const struct _CNTRL2 iARCHI_DISABLE_INPUT_M = {
    0x6C, 0x00, 0xFFFF, 0,
    {
        { 0x40, 0x09 },
        { 0x13, 0x0A },
        { 0x00, 0x00 },
    },
};

/* Disables the input for the female model. */
static const struct _CNTRL2 iARCHI_DISABLE_INPUT_F = {
    0x6D, 0x00, 0xFFFF, 0,
    {
        { 0x41, 0x09 },
        { 0x13, 0x0A },
        { 0x00, 0x00 },
    },
};

/* Disables the input for the child model. */
static const struct _CNTRL2 iARCHI_DISABLE_INPUT_C = {
    0x6E, 0x00, 0xFFFF, 0,
    {
        { 0x42, 0x09 },
        { 0x13, 0x0A },
        { 0x00, 0x00 },
    },
};

/* Enables the male model. */
static const struct _CNTRL13 iARCHI_ENABLE_M = {
    0x40, 0x00, 0xFFFF, 0,
    {
        { 0x03, 0xF8 },
        { 0x08, 0xF8 },
        { 0x0D, 0xF8 },
        { 0x10, 0xF8 },
        { 0x2C, 0xF8 },
        { 0x34, 0xF8 },
        { 0x37, 0xF8 },
        { 0x3D, 0xF8 },
        { 0x40, 0x0B },
        { 0x31, 0xF8 },
        { 0x51, 0xF8 },
        { 0x13, 0x02 },
        { 0x03, 0x01 },
        { 0x00, 0x00 },
    },
};

/* Enables the female model. */
static const struct _CNTRL13 iARCHI_ENABLE_F = {
    0x41, 0x00, 0xFFFF, 0,
    {
        { 0x04, 0xF8 },
        { 0x09, 0xF8 },
        { 0x0E, 0xF8 },
        { 0x11, 0xF8 },
        { 0x2D, 0xF8 },
        { 0x35, 0xF8 },
        { 0x38, 0xF8 },
        { 0x3E, 0xF8 },
        { 0x41, 0x0B },
        { 0x32, 0xF8 },
        { 0x52, 0xF8 },
        { 0x13, 0x02 },
        { 0x04, 0x01 },
        { 0x00, 0x00 },
    },
};

/* Enables the child model. */
static const struct _CNTRL13 iARCHI_ENABLE_C = {
    0x42, 0x00, 0xFFFF, 0,
    {
        { 0x05, 0xF8 },
        { 0x0A, 0xF8 },
        { 0x0F, 0xF8 },
        { 0x12, 0xF8 },
        { 0x2E, 0xF8 },
        { 0x36, 0xF8 },
        { 0x39, 0xF8 },
        { 0x3F, 0xF8 },
        { 0x42, 0x0B },
        { 0x33, 0xF8 },
        { 0x53, 0xF8 },
        { 0x13, 0x02 },
        { 0x05, 0x01 },
        { 0x00, 0x00 },
    },
};

/* Stops feeding silence estimates to spectral subtraction. */
static const struct _CNTRL3 iARCHI_STOP_SPECSUB_SIL_QUEUE = {
    0x6A, 0x00, 0xFFFF, 0,
    {
        { 0x08, 0x04 },
        { 0x09, 0x04 },
        { 0x0A, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Resumes feeding silence estimates to spectral subtraction. */
static const struct _CNTRL3 iARCHI_START_SPECSUB_SIL_QUEUE = {
    0x6B, 0x00, 0xFFFF, 0,
    {
        { 0x08, 0x03 },
        { 0x09, 0x03 },
        { 0x0A, 0x03 },
        { 0x00, 0x00 },
    },
};

/* Advances the scheduler state without notifying blocks. */
static const struct _CNTRL0 iARCHI_GOTONEXTSTATE = {
    0x02, 0x00, 0xFFFE, 0,
    {
        { 0x00, 0x00 },
    },
};

/* Accepts the start-control command without forwarding it. */
static const struct _CNTRL0 iARCHI_START_ARCHI_CONTROL = {
    0x22, 0x00, 0xFFFF, 0,
    {
        { 0x00, 0x00 },
    },
};

/* Accepts the stop-control command without forwarding it. */
static const struct _CNTRL0 iARCHI_STOP_ARCHI_CONTROL = {
    0x23, 0x00, 0xFFFF, 0,
    {
        { 0x00, 0x00 },
    },
};

/* Restores session data in the speech blocks. */
static const struct _CNTRL12 iARCHI_PUT_SESSION = {
    0x26, 0x00, 0xFFFF, 0,
    {
        { 0x06, 0x05 },
        { 0x0B, 0x05 },
        { 0x08, 0x02 },
        { 0x09, 0x02 },
        { 0x0A, 0x02 },
        { 0x0D, 0x04 },
        { 0x0E, 0x04 },
        { 0x0F, 0x04 },
        { 0x13, 0x06 },
        { 0x27, 0x69 },
        { 0x1F, 0xFB },
        { 0x1F, 0x0A },
        { 0x00, 0x00 },
    },
};

/* Collects session data from the speech blocks. */
static const struct _CNTRL12 iARCHI_GET_SESSION = {
    0x27, 0x00, 0xFFFF, 0,
    {
        { 0x06, 0x06 },
        { 0x0B, 0x06 },
        { 0x08, 0x01 },
        { 0x09, 0x01 },
        { 0x0A, 0x01 },
        { 0x0D, 0x03 },
        { 0x0E, 0x03 },
        { 0x0F, 0x03 },
        { 0x13, 0x07 },
        { 0x27, 0x6A },
        { 0x1F, 0xFB },
        { 0x1F, 0x0B },
        { 0x00, 0x00 },
    },
};

/* Reads the distance-calculation callbacks. */
static const struct _CNTRL1 iARCHI_GET_DISTCALCFUNCS = {
    0x55, 0x00, 0xFFFF, 0,
    {
        { 0x1F, 0x1B },
        { 0x00, 0x00 },
    },
};

/* Installs the distance-calculation callbacks. */
static const struct _CNTRL3 iARCHI_SET_DISTCALCFUNCS = {
    0x56, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0x24 },
        { 0x23, 0x24 },
        { 0x24, 0x06 },
        { 0x00, 0x00 },
    },
};

/* Queries the current microphone signal level. */
static const struct _CNTRL1 iARCHI_GET_PSIGNALLEVEL = {
    0x04, 0x00, 0xFFFF, 0,
    {
        { 0x02, 0x01 },
        { 0x00, 0x00 },
    },
};

/* Queries the signal-to-noise ratio. */
static const struct _CNTRL1 iARCHI_GET_SNR = {
    0x05, 0x00, 0xFFFF, 0,
    {
        { 0x0B, 0x01 },
        { 0x00, 0x00 },
    },
};

/* Queries the start time of detected speech. */
static const struct _CNTRL1 iARCHI_GET_PBEGINTIME = {
    0x2A, 0x00, 0xFFFF, 0,
    {
        { 0x27, 0x65 },
        { 0x00, 0x00 },
    },
};

/* Queries the extra-event score. */
static const struct _CNTRL1 iARCHI_GET_EXTRAEVENTSCORE = {
    0x63, 0x00, 0xFFFF, 0,
    {
        { 0x24, 0x01 },
        { 0x00, 0x00 },
    },
};

/* Sets the voice activity parameters. */
static const struct _CNTRL1 iARCHI_VAD_SET_PARAM = {
    0x50, 0x00, 0xFFFF, 0,
    {
        { 0x0B, 0x03 },
        { 0x00, 0x00 },
    },
};

/* Sets the speech trigger parameters. */
static const struct _CNTRL1 iARCHI_TRIGGER_SET_PARAM = {
    0x51, 0x00, 0xFFFF, 0,
    {
        { 0x27, 0x67 },
        { 0x00, 0x00 },
    },
};

/* Sets the gender classifier parameters. */
static const struct _CNTRL1 iARCHI_GENDER_SET_PARAM = {
    0x52, 0x00, 0xFFFF, 0,
    {
        { 0x13, 0x08 },
        { 0x00, 0x00 },
    },
};

/* Sets the distance calculation parameters. */
static const struct _CNTRL2 iARCHI_DISTCALC_SET_PARAM = {
    0x53, 0x00, 0xFFFF, 0,
    {
        { 0x1F, 0xFB },
        { 0x1F, 0x0F },
        { 0x00, 0x00 },
    },
};

/* Sets the N-best decoder parameters. */
static const struct _CNTRL2 iARCHI_NBESTDYNPROG_SET_PARAM = {
    0x54, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0x1A },
        { 0x23, 0x1A },
        { 0x00, 0x00 },
    },
};

/* Reads the voice activity parameters. */
static const struct _CNTRL1 iARCHI_VAD_GET_PARAM = {
    0x46, 0x00, 0xFFFF, 0,
    {
        { 0x0B, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Reads the speech trigger parameters. */
static const struct _CNTRL1 iARCHI_TRIGGER_GET_PARAM = {
    0x47, 0x00, 0xFFFF, 0,
    {
        { 0x27, 0x66 },
        { 0x00, 0x00 },
    },
};

/* Reads the gender classifier parameters. */
static const struct _CNTRL1 iARCHI_GENDER_GET_PARAM = {
    0x48, 0x00, 0xFFFF, 0,
    {
        { 0x13, 0x09 },
        { 0x00, 0x00 },
    },
};

/* Reads the distance calculation parameters. */
static const struct _CNTRL2 iARCHI_DISTCALC_GET_PARAM = {
    0x49, 0x00, 0xFFFF, 0,
    {
        { 0x1F, 0xFB },
        { 0x1F, 0x0D },
        { 0x00, 0x00 },
    },
};

/* Reads the N-best decoder parameters. */
static const struct _CNTRL1 iARCHI_NBESTDYNPROG_GET_PARAM = {
    0x4A, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0x1B },
        { 0x00, 0x00 },
    },
};

/* Loads language data into the speech graph. */
static const struct _CNTRL14 iARCHI_LOADLANGUAGE = {
    0x25, 0x00, 0x0001, 0,
    {
        { 0x13, 0x01 },
        { 0x1E, 0x01 },
        { 0x20, 0xFB },
        { 0x21, 0xFB },
        { 0x22, 0xFB },
        { 0x23, 0xFB },
        { 0x24, 0xFB },
        { 0x1F, 0xFB },
        { 0x1F, 0x10 },
        { 0x20, 0x22 },
        { 0x21, 0x22 },
        { 0x22, 0x07 },
        { 0x23, 0x22 },
        { 0x24, 0x07 },
        { 0x00, 0x00 },
    },
};

/* Unloads the active language. */
static const struct _CNTRL1 iARCHI_UNLOADLANGUAGE = {
    0x5F, 0x00, 0x0000, 0,
    {
        { 0x1F, 0x1E },
        { 0x00, 0x00 },
    },
};

/* Changes the recognition context and returns to idle. */
static const struct _CNTRL2 iARCHI_CHANGECTX_TOIDLE = {
    0x24, 0x00, 0x0002, 0,
    {
        { 0x20, 0x23 },
        { 0x23, 0x23 },
        { 0x00, 0x00 },
    },
};

/* Updates the recognition context. */
static const struct _CNTRL2 iARCHI_CHANGECTX = {
    0x24, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0x23 },
        { 0x23, 0x23 },
        { 0x00, 0x00 },
    },
};

/* Installs the extra-event context. */
static const struct _CNTRL1 iARCHI_SETEXEVCTX = {
    0x66, 0x00, 0xFFFF, 0,
    {
        { 0x24, 0x08 },
        { 0x00, 0x00 },
    },
};

/* Informs the recognizers that a context was deleted. */
static const struct _CNTRL2 iARCHI_CTXDELETED = {
    0x29, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0x27 },
        { 0x23, 0x27 },
        { 0x00, 0x00 },
    },
};

/* Sets the active-word mask. */
static const struct _CNTRL1 iARCHI_SETACTWORDS = {
    0x19, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0x1C },
        { 0x00, 0x00 },
    },
};

/* Reads the active-word mask. */
static const struct _CNTRL1 iARCHI_GETACTWORDS = {
    0x1A, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0x1D },
        { 0x00, 0x00 },
    },
};

/* Sets the microphone gain. */
static const struct _CNTRL1 iARCHI_MIKE_SET = {
    0x35, 0x00, 0xFFFF, 0,
    {
        { 0x0B, 0x02 },
        { 0x00, 0x00 },
    },
};

/* Starts user-word training and enters the prompt state. */
static const struct _CNTRL5 iARCHI_USERWORDTRAININGSTART = {
    0x0E, 0x00, 0x000F, 0,
    {
        { 0x22, 0xF8 },
        { 0x21, 0xF8 },
        { 0x46, 0xF8 },
        { 0x47, 0xF8 },
        { 0x24, 0xFF },
        { 0x00, 0x00 },
    },
};

/* Stops the user-word training blocks. */
static const struct _CNTRL4 iARCHI_USERWORDTRAININGSTOP = {
    0x10, 0x00, 0xFFFF, 0,
    {
        { 0x22, 0xFF },
        { 0x21, 0xFF },
        { 0x46, 0xFF },
        { 0x47, 0xFF },
        { 0x00, 0x00 },
    },
};

/* Initializes recognition data for the user word. */
static const struct _CNTRL1 iARCHI_INITUSERWORD = {
    0x0F, 0x00, 0xFFFF, 0,
    {
        { 0x21, 0x17 },
        { 0x00, 0x00 },
    },
};

/* Starts recording after the application accepts the training prompt. */
static const struct _CNTRL8 iARCHI_USERTRAIN_OK_START = {
    0x0A, 0x00, 0xFFFE, (ControlCallback)cbNewUserWordUtterance,
    {
        { 0x14, 0x01 },
        { 0x01, 0x01 },
        { 0x0B, 0x07 },
        { 0x16, 0x64 },
        { 0x17, 0xC8 },
        { 0x1A, 0xC9 },
        { 0x1C, 0xC8 },
        { 0x21, 0x03 },
        { 0x00, 0x00 },
    },
};

/* Starts recording the last training utterance. */
static const struct _CNTRL8 iARCHI_USERTRAIN_OK_LASTUTT_START = {
    0x16, 0x00, 0xFFFE, (ControlCallback)cbNewUserWordUtterance,
    {
        { 0x01, 0x01 },
        { 0x0B, 0x07 },
        { 0x14, 0x01 },
        { 0x16, 0x64 },
        { 0x17, 0xC8 },
        { 0x1A, 0xC9 },
        { 0x1C, 0xC8 },
        { 0x21, 0x03 },
        { 0x00, 0x00 },
    },
};

/* Reports an accepted utterance and returns to the prompt. */
static const struct _CNTRL0 iARCHI_USERTRAIN_OK_ACCEPT = {
    0x0A, 0x00, 0x000F, (ControlCallback)cbUtteranceAccepted,
    {
        { 0x00, 0x00 },
    },
};

/* Reports acceptance of the last training utterance. */
static const struct _CNTRL0 iARCHI_USERTRAIN_OK_LASTUTT_ACCEPT = {
    0x16, 0x00, 0xFFFE, (ControlCallback)cbLastUtteranceAccepted,
    {
        { 0x00, 0x00 },
    },
};

/* Completes user-word training after the final confirmation. */
static const struct _CNTRL0 iARCHI_USERTRAIN_OK_LASTUTT_CONFIRM = {
    0x16, 0x00, 0x0014, (ControlCallback)cbDoUserWordTraining,
    {
        { 0x00, 0x00 },
    },
};

/* Reports the rejected utterance and returns to the prompt. */
static const struct _CNTRL0 iARCHI_USERTRAIN_NOK_ONACCEPT = {
    0x0C, 0x00, 0x000F, (ControlCallback)cbUtteranceNotAccepted,
    {
        { 0x00, 0x00 },
    },
};

/* Cancels training after a rejected prompt or confirmation. */
static const struct _CNTRL0 iARCHI_USERTRAIN_NOK_ONCONFIRMORSTART = {
    0x0C, 0x00, 0x0001, (ControlCallback)cbCancelUserWordTraining,
    {
        { 0x00, 0x00 },
    },
};

/* Cancels user-word training and returns to data-ready. */
static const struct _CNTRL0 iARCHI_USERTRAIN_CANCEL = {
    0x0B, 0x00, 0x0001, (ControlCallback)cbCancelUserWordTraining,
    {
        { 0x00, 0x00 },
    },
};

/* Initializes the user-word scorer. */
static const struct _CNTRL1 iARCHI_INITSCOREUSERWORD = {
    0x13, 0x00, 0xFFFF, 0,
    {
        { 0x22, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Routes a parameter update to the user-word scorer. */
static const struct _CNTRL1 iARCHI_SETPARAMSCOREUSERWORD = {
    0x17, 0x00, 0xFFFF, 0,
    {
        { 0x22, 0x05 },
        { 0x00, 0x00 },
    },
};

/* Routes a parameter query to the user-word scorer. */
static const struct _CNTRL1 iARCHI_GETPARAMSCOREUSERWORD = {
    0x17, 0x00, 0xFFFF, 0,
    {
        { 0x22, 0x06 },
        { 0x00, 0x00 },
    },
};

/* Enables adaptation to silence. */
static const struct _CNTRL4 iARCHI_SILADATPTON = {
    0x15, 0x00, 0xFFFF, 0,
    {
        { 0x1F, 0x02 },
        { 0x1F, 0x17 },
        { 0x20, 0x25 },
        { 0x23, 0x25 },
        { 0x00, 0x00 },
    },
};

/* Disables adaptation to silence. */
static const struct _CNTRL4 iARCHI_SILADAPTOFF = {
    0x11, 0x00, 0xFFFF, 0,
    {
        { 0x1F, 0x03 },
        { 0x1F, 0x18 },
        { 0x20, 0x26 },
        { 0x23, 0x26 },
        { 0x00, 0x00 },
    },
};

/* Labels the start frame for second-pass recognition. */
static const struct _CNTRL2 iARCHI_LABEL_START_SECONDPASS = {
    0x1F, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0x03 },
        { 0x24, 0xFC },
        { 0x00, 0x00 },
    },
};

/* Labels the stop frame for second-pass recognition. */
static const struct _CNTRL2 iARCHI_LABEL_STOP_SECONDPASS = {
    0x20, 0x00, 0xFFFF, 0,
    {
        { 0x24, 0xFB },
        { 0x20, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Labels the start frame for user-word training. */
static const struct _CNTRL1 iARCHI_LABEL_START_UW = {
    0x1F, 0x00, 0xFFFF, 0,
    {
        { 0x22, 0x01 },
        { 0x00, 0x00 },
    },
};

/* Labels the stop frame for user-word training. */
static const struct _CNTRL1 iARCHI_LABEL_STOP_UW = {
    0x20, 0x00, 0xFFFF, 0,
    {
        { 0x22, 0x02 },
        { 0x00, 0x00 },
    },
};

/* Disables the pitch-processing blocks. */
static const struct _CNTRL9 iARCHI_NO_PITCH_PROCESSING = {
    0x5D, 0x00, 0xFFFF, 0,
    {
        { 0x15, 0xFC },
        { 0x16, 0xFC },
        { 0x17, 0xFC },
        { 0x18, 0xFC },
        { 0x19, 0xFC },
        { 0x1A, 0xFC },
        { 0x1B, 0xFC },
        { 0x1C, 0xFC },
        { 0x1D, 0x65 },
        { 0x00, 0x00 },
    },
};

/* Enables the pitch-processing blocks. */
static const struct _CNTRL26 iARCHI_DO_PITCH_PROCESSING = {
    0x5E, 0x00, 0xFFFF, 0,
    {
        { 0x15, 0xF8 },
        { 0x16, 0xF8 },
        { 0x17, 0xF8 },
        { 0x18, 0xF8 },
        { 0x19, 0xF8 },
        { 0x1A, 0xF8 },
        { 0x1B, 0xF8 },
        { 0x1C, 0xF8 },
        { 0x48, 0xF8 },
        { 0x49, 0xF8 },
        { 0x4A, 0xF8 },
        { 0x4B, 0xF8 },
        { 0x4C, 0xF8 },
        { 0x4D, 0xF8 },
        { 0x4E, 0xF8 },
        { 0x4F, 0xF8 },
        { 0x56, 0xF8 },
        { 0x15, 0xFB },
        { 0x16, 0xFB },
        { 0x17, 0xFB },
        { 0x18, 0xFB },
        { 0x19, 0xFB },
        { 0x1A, 0xFB },
        { 0x1B, 0xFB },
        { 0x1C, 0xFB },
        { 0x1D, 0x97 },
        { 0x00, 0x00 },
    },
};

/* Enables extra-event detection. */
static const struct _CNTRL1 iARCHI_ENABLE_EXTRAEVENTDETECTION = {
    0x64, 0x00, 0xFFFF, 0,
    {
        { 0x24, 0xF8 },
        { 0x00, 0x00 },
    },
};

/* Disables extra-event detection. */
static const struct _CNTRL1 iARCHI_DISABLE_EXTRAEVENTDETECTION = {
    0x65, 0x00, 0xFFFF, 0,
    {
        { 0x24, 0xFF },
        { 0x00, 0x00 },
    },
};

/* Reports that the gender classifier disabled the male model. */
static const struct _CNTRL12 iGENDER_DISABLE_M = {
    0x0D, 0x00, 0xFFFF, (ControlCallback)cbGenderDisable,
    {
        { 0x03, 0xFF },
        { 0x08, 0xFF },
        { 0x0D, 0xFF },
        { 0x10, 0xFF },
        { 0x2C, 0xFF },
        { 0x34, 0xFF },
        { 0x37, 0xFF },
        { 0x3D, 0xFF },
        { 0x31, 0xFF },
        { 0x51, 0xFF },
        { 0x40, 0xFF },
        { 0x13, 0x0A },
        { 0x00, 0x00 },
    },
};

/* Reports that the gender classifier disabled the female model. */
static const struct _CNTRL12 iGENDER_DISABLE_F = {
    0x0E, 0x00, 0xFFFF, (ControlCallback)cbGenderDisable,
    {
        { 0x04, 0xFF },
        { 0x09, 0xFF },
        { 0x0E, 0xFF },
        { 0x11, 0xFF },
        { 0x2D, 0xFF },
        { 0x35, 0xFF },
        { 0x38, 0xFF },
        { 0x3E, 0xFF },
        { 0x32, 0xFF },
        { 0x52, 0xFF },
        { 0x41, 0xFF },
        { 0x13, 0x0A },
        { 0x00, 0x00 },
    },
};

/* Reports that the gender classifier disabled the child model. */
static const struct _CNTRL12 iGENDER_DISABLE_C = {
    0x0F, 0x00, 0xFFFF, (ControlCallback)cbGenderDisable,
    {
        { 0x05, 0xFF },
        { 0x0A, 0xFF },
        { 0x0F, 0xFF },
        { 0x12, 0xFF },
        { 0x2E, 0xFF },
        { 0x36, 0xFF },
        { 0x39, 0xFF },
        { 0x3F, 0xFF },
        { 0x33, 0xFF },
        { 0x53, 0xFF },
        { 0x42, 0xFF },
        { 0x13, 0x0A },
        { 0x00, 0x00 },
    },
};

/* Ignores a speech-start event in the current state. */
static const struct _CNTRL0 iTRIGGER_START_DETECTED_IGNORE = {
    0xC8, 0x00, 0xFFFF, 0,
    {
        { 0x00, 0x00 },
    },
};

/* Reports speech onset and advances the architecture state. */
static const struct _CNTRL9 iTRIGGER_START_DETECTED_TO_NEXTSTATE = {
    0xC8, 0x00, 0xFFFE, (ControlCallback)cbStartDetected,
    {
        { 0x0C, 0xF9 },
        { 0x0D, 0xF9 },
        { 0x0E, 0xF9 },
        { 0x0F, 0xF9 },
        { 0x17, 0xF9 },
        { 0x19, 0xF9 },
        { 0x1F, 0x17 },
        { 0x20, 0x25 },
        { 0x23, 0x25 },
        { 0x00, 0x00 },
    },
};

/* Ignores a terminal-silence event in the current state. */
static const struct _CNTRL0 iTRIGGER_TS_DETECTED_IGNORE = {
    0xC9, 0x00, 0xFFFF, 0,
    {
        { 0x00, 0x00 },
    },
};

/* Reports terminal silence and advances the architecture state. */
static const struct _CNTRL15 iTRIGGER_TS_DETECTED_TO_NEXTSTATE = {
    0xC9, 0x28, 0xFFFE, (ControlCallback)cbTSDetected,
    {
        { 0x01, 0x02 },
        { 0x07, 0x01 },
        { 0x0C, 0x01 },
        { 0x0D, 0x02 },
        { 0x0E, 0x02 },
        { 0x0F, 0x02 },
        { 0x13, 0x03 },
        { 0x14, 0x02 },
        { 0x16, 0x65 },
        { 0x1A, 0xC8 },
        { 0x1C, 0xC9 },
        { 0x21, 0x04 },
        { 0x20, 0x04 },
        { 0x23, 0x04 },
        { 0x24, 0x03 },
        { 0x00, 0x00 },
    },
};

/* Reports a garbled-sound event from the speech trigger. */
static const struct _CNTRL0 iTRIGGER_GARBLED_SOUND_DETECTED = {
    0xCA, 0x00, 0xFFFF, (ControlCallback)cbTOSAbnormalCond,
    {
        { 0x00, 0x00 },
    },
};

/* Asks the application for the current microphone gain. */
static const struct _CNTRL0 iVAD_ASK_CURRENT_GAIN = {
    0x0A, 0x00, 0xFFFF, (ControlCallback)cbAskCurrentGain,
    {
        { 0x00, 0x00 },
    },
};

/* Requests a change to the microphone gain. */
static const struct _CNTRL0 iVAD_CHANGE_MIKE = {
    0x0B, 0x00, 0xFFFF, (ControlCallback)cbGainRequest,
    {
        { 0x00, 0x00 },
    },
};

/* Reports an abnormal input condition from voice activity detection. */
static const struct _CNTRL0 iVAD_EVENTRECEIVED = {
    0x0C, 0x00, 0xFFFF, (ControlCallback)cbTOSAbnormalCond,
    {
        { 0x00, 0x00 },
    },
};

/* Ignores a terminal-silence event from the N-best decoder. */
static const struct _CNTRL0 iNBESTDYNPROG_TSDETECTED_IGNORE = {
    0x08, 0x00, 0xFFFF, 0,
    {
        { 0x00, 0x00 },
    },
};

/* Advances after the N-best decoder finds terminal silence. */
static const struct _CNTRL14 iNBESTDYNPROG_TSDETECTED_TONEXTSTATE = {
    0x08, 0x28, 0xFFFE, (ControlCallback)cbTSAfterStartDetected,
    {
        { 0x01, 0x02 },
        { 0x07, 0x01 },
        { 0x0C, 0x01 },
        { 0x0D, 0x02 },
        { 0x0E, 0x02 },
        { 0x0F, 0x02 },
        { 0x13, 0x03 },
        { 0x14, 0x02 },
        { 0x16, 0x65 },
        { 0x1A, 0xC8 },
        { 0x1C, 0xC9 },
        { 0x20, 0x04 },
        { 0x23, 0x04 },
        { 0x24, 0x03 },
        { 0x00, 0x00 },
    },
};

/* Starts an enabled second pass for the N-best recognition result. */
static const struct _CNTRL0 iNBESTDYNPROG_WORDRECOGNISED = {
    0x09, 0x00, 0xFFFF, (ControlCallback)cbDoSecondPassIfEnabled,
    {
        { 0x00, 0x00 },
    },
};

/* Starts an enabled second pass for an isolated-word result. */
static const struct _CNTRL0 iISOWORDTOKENDP_WORDRECOGNISED = {
    0x09, 0x00, 0xFFFF, (ControlCallback)cbDoSecondPassIfEnabled,
    {
        { 0x00, 0x00 },
    },
};

/* Reports an empty first pass and returns to idle. */
static const struct _CNTRL0 iNBESTDYNPROG_NOWORDRECOGNISED = {
    0x0A, 0x00, 0x0002, (ControlCallback)cbNothingRecognised,
    {
        { 0x00, 0x00 },
    },
};

/* Reports the second-pass result and returns to idle. */
static const struct _CNTRL0 iNBESTDYNPROG_WORDRECOGNISED_SECONDPASS = {
    0x09, 0x00, 0x0002, (ControlCallback)cbTOSRecognised,
    {
        { 0x00, 0x00 },
    },
};

/* Reports an empty second pass and returns to idle. */
static const struct _CNTRL0 iNBESTDYNPROG_NOWORDRECOGNISED_SECONDPASS = {
    0x0A, 0x00, 0x0002, (ControlCallback)cbNothingRecognised,
    {
        { 0x00, 0x00 },
    },
};

/* Reports the result of the spelling second pass. */
static const struct _CNTRL0 iNBESTDYNPROG_WORDRECOGNISED_SPELLSECONDPASS = {
    0x09, 0x00, 0xFFFF, (ControlCallback)cbSpellRecognised,
    {
        { 0x00, 0x00 },
    },
};

/* Reports an empty spelling second pass. */
static const struct _CNTRL0 iNBESTDYNPROG_NOWORDRECOGNISED_SPELLSECONDPASS = {
    0x0A, 0x00, 0xFFFF, (ControlCallback)cbSpellNothingRecognised,
    {
        { 0x00, 0x00 },
    },
};

/* Reports a user-word training result. */
static const struct _CNTRL0 iDPGENUW_RESULTREADY = {
    0x0C, 0x00, 0xFFFF, cbUserWordResultAvailable,
    {
        { 0x00, 0x00 },
    },
};

/* Advances recording after the user-word generator detects terminal silence. */
static const struct _CNTRL12 iDPGENUW_TSDETECTED_TONEXTSTATE = {
    0x08, 0x28, 0xFFFE, (ControlCallback)cbTSAfterStartDetected,
    {
        { 0x01, 0x02 },
        { 0x07, 0x01 },
        { 0x0C, 0x01 },
        { 0x0D, 0x02 },
        { 0x0E, 0x02 },
        { 0x0F, 0x02 },
        { 0x13, 0x03 },
        { 0x14, 0x02 },
        { 0x16, 0x65 },
        { 0x1A, 0xC8 },
        { 0x1C, 0xC9 },
        { 0x21, 0x04 },
        { 0x00, 0x00 },
    },
};

/* Reports a user-word score. */
static const struct _CNTRL0 iDPSCOREUW_RESULTREADY = {
    0x0C, 0x00, 0xFFFF, cbScoreUserWordResultAvailable,
    {
        { 0x00, 0x00 },
    },
};

/* Routes features to the N-best decoder. */
static const struct _CNTRL5 iARCHI_SWITCH_TO_NBESTDP = {
    0x57, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0xFB },
        { 0x21, 0xFC },
        { 0x22, 0xFC },
        { 0x1F, 0xFC },
        { 0x23, 0xFC },
        { 0x00, 0x00 },
    },
};

/* Routes features to the tree decoder. */
static const struct _CNTRL5 iARCHI_SWITCH_TO_TREEDP = {
    0x5A, 0x00, 0xFFFF, 0,
    {
        { 0x23, 0xFB },
        { 0x20, 0xFC },
        { 0x21, 0xFC },
        { 0x22, 0xFC },
        { 0x1F, 0xFC },
        { 0x00, 0x00 },
    },
};

/* Routes features to the user-word generator. */
static const struct _CNTRL6 iARCHI_SWITCH_TO_GENUWDP = {
    0x58, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0xFC },
        { 0x21, 0xFB },
        { 0x22, 0xFC },
        { 0x1F, 0xFB },
        { 0x23, 0xFC },
        { 0x21, 0x22 },
        { 0x00, 0x00 },
    },
};

/* Routes features to the user-word scorer. */
static const struct _CNTRL6 iARCHI_SWITCH_TO_SCOREUWDP = {
    0x59, 0x00, 0xFFFF, 0,
    {
        { 0x20, 0xFC },
        { 0x21, 0xFC },
        { 0x22, 0xFB },
        { 0x1F, 0xFB },
        { 0x23, 0xFC },
        { 0x22, 0x07 },
        { 0x00, 0x00 },
    },
};

/* Resizes the distance queues. */
static const struct _CNTRL1 iARCHI_RESIZEDISTANCEQUEUES = {
    0x60, 0x00, 0xFFFF, 0,
    {
        { 0x1F, 0x1F },
        { 0x00, 0x00 },
    },
};

/* Destroys the isolated-word decoder. */
static const struct _CNTRL1 iARCHI_DESTRUCTISODP = {
    0x61, 0x00, 0xFFFF, 0,
    {
        { 0x23, 0xFF },
        { 0x00, 0x00 },
    },
};

/* Destroys the extra-event decoder. */
static const struct _CNTRL1 iARCHI_DESTRUCTEXTRAEVENTDP = {
    0x67, 0x00, 0xFFFF, 0,
    {
        { 0x24, 0xFF },
        { 0x00, 0x00 },
    },
};

/* Creates the isolated-word decoder and installs its language. */
static const struct _CNTRL2 iARCHI_CONSTRUCTISODPANDLOADLANG = {
    0x62, 0x00, 0xFFFF, 0,
    {
        { 0x23, 0xF8 },
        { 0x23, 0x22 },
        { 0x00, 0x00 },
    },
};

/* Prepares the speech blocks for the training prompt. */
static const struct _CNTRL5 iARCHI_GOTO_PROMPT = {
    0x69, 0x00, 0x000F, 0,
    {
        { 0x20, 0xFC },
        { 0x21, 0xFB },
        { 0x22, 0xFC },
        { 0x1F, 0xFB },
        { 0x23, 0xFC },
        { 0x00, 0x00 },
    },
};

/* Asks the application for the current microphone gain while initializing or waiting for
 * recognition. */
static const struct _CNTRL0 *const VAD_BOOT_DATAREADY_IDLE[] = {
    &iVAD_ASK_CURRENT_GAIN,
    0,
};

/* Advances after the N-best decoder finds terminal silence while running first-pass recognition. */
static const struct _CNTRL0 *const NBESTDYNPROG_RUN[] = {
    (const struct _CNTRL0 *)&iNBESTDYNPROG_TSDETECTED_TONEXTSTATE,
    0,
};

/* Advances recording after the user-word generator detects terminal silence while recording the
 * utterance. */
static const struct _CNTRL0 *const DPGENUW_RECORD[] = {
    (const struct _CNTRL0 *)&iDPGENUW_TSDETECTED_TONEXTSTATE,
    0,
};

/* Reports a user-word training result while accepting the recorded utterance. */
static const struct _CNTRL0 *const DPGENUW_ACCEPT[] = {
    &iDPGENUW_RESULTREADY,
    0,
};

/* Reports a user-word score while waiting for a recognition context. */
static const struct _CNTRL0 *const DPSCOREUW_DATAREADY[] = {
    &iDPSCOREUW_RESULTREADY,
    0,
};

/* Architecture commands available while initializing the engine. */
static const struct _CNTRL0 *const ARCHI_BOOT[] = {
    (const struct _CNTRL0 *)&iARCHI_LOADLANGUAGE,
    (const struct _CNTRL0 *)&iARCHI_GET_PSIGNALLEVEL,
    (const struct _CNTRL0 *)&iARCHI_GET_PBEGINTIME,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_VAD_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_TRIGGER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_GENDER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_DISTCALC_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_NBESTDYNPROG_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_VAD_GET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_GENDER_GET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_TRIGGER_GET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_DISTCALC_GET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_NBESTDYNPROG_GET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_SILADAPTOFF,
    (const struct _CNTRL0 *)&iARCHI_CTXDELETED,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_EXTRAEVENTDETECTION,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_EXTRAEVENTDETECTION,
    0,
};

/* Architecture commands available while waiting for a recognition context. */
static const struct _CNTRL0 *const ARCHI_DATAREADY[] = {
    (const struct _CNTRL0 *)&iARCHI_PUT_SESSION,
    (const struct _CNTRL0 *)&iARCHI_GET_SESSION,
    (const struct _CNTRL0 *)&iARCHI_NO_PITCH_PROCESSING,
    (const struct _CNTRL0 *)&iARCHI_DO_PITCH_PROCESSING,
    (const struct _CNTRL0 *)&iARCHI_LOADLANGUAGE,
    (const struct _CNTRL0 *)&iARCHI_UNLOADLANGUAGE,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_NBESTDP,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_TREEDP,
    (const struct _CNTRL0 *)&iARCHI_CHANGECTX_TOIDLE,
    (const struct _CNTRL0 *)&iARCHI_CTXDELETED,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_SILADAPTOFF,
    &iARCHI_START_ARCHI_CONTROL,
    &iARCHI_STOP_ARCHI_CONTROL,
    (const struct _CNTRL0 *)&iARCHI_SILADATPTON,
    (const struct _CNTRL0 *)&iARCHI_VAD_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_TRIGGER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_GENDER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_DISTCALC_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_NBESTDYNPROG_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_USERWORDTRAININGSTART,
    (const struct _CNTRL0 *)&iARCHI_USERWORDTRAININGSTOP,
    (const struct _CNTRL0 *)&iARCHI_INITSCOREUSERWORD,
    (const struct _CNTRL0 *)&iARCHI_SETPARAMSCOREUSERWORD,
    (const struct _CNTRL0 *)&iARCHI_GETPARAMSCOREUSERWORD,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_SCOREUWDP,
    (const struct _CNTRL0 *)&iARCHI_LABEL_START_UW,
    (const struct _CNTRL0 *)&iARCHI_LABEL_STOP_UW,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_M,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_F,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_C,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_INPUT_M,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_INPUT_F,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_INPUT_C,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_M,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_F,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_C,
    (const struct _CNTRL0 *)&iARCHI_GET_DISTCALCFUNCS,
    (const struct _CNTRL0 *)&iARCHI_SET_DISTCALCFUNCS,
    (const struct _CNTRL0 *)&iARCHI_CONSTRUCTISODPANDLOADLANG,
    (const struct _CNTRL0 *)&iARCHI_DESTRUCTISODP,
    0,
};

/* Architecture commands available while waiting for a recognition request. */
static const struct _CNTRL0 *const ARCHI_IDLE[] = {
    &iARCHI_FLUSH_IGNORE,
    (const struct _CNTRL0 *)&iARCHI_SILADATPTON,
    (const struct _CNTRL0 *)&iARCHI_LOADLANGUAGE,
    (const struct _CNTRL0 *)&iARCHI_PUT_SESSION,
    (const struct _CNTRL0 *)&iARCHI_GET_SESSION,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_USERWORDTRAININGSTART,
    (const struct _CNTRL0 *)&iARCHI_CHANGECTX,
    (const struct _CNTRL0 *)&iARCHI_SETEXEVCTX,
    (const struct _CNTRL0 *)&iARCHI_CTXDELETED,
    (const struct _CNTRL0 *)&iARCHI_GOTOSLEEP,
    (const struct _CNTRL0 *)&iARCHI_GOTOSPELLSLEEP,
    (const struct _CNTRL0 *)&iARCHI_GOTOISOWORDSLEEP,
    (const struct _CNTRL0 *)&iARCHI_SETACTWORDS,
    (const struct _CNTRL0 *)&iARCHI_GETACTWORDS,
    (const struct _CNTRL0 *)&iARCHI_VAD_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_TRIGGER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_GENDER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_DISTCALC_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_NBESTDYNPROG_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_NBESTDP,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_TREEDP,
    (const struct _CNTRL0 *)&iARCHI_GET_EXTRAEVENTSCORE,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_EXTRAEVENTDETECTION,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_EXTRAEVENTDETECTION,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_M,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_F,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_C,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_INPUT_M,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_INPUT_F,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_INPUT_C,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_M,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_F,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_C,
    (const struct _CNTRL0 *)&iARCHI_GET_DISTCALCFUNCS,
    (const struct _CNTRL0 *)&iARCHI_SET_DISTCALCFUNCS,
    (const struct _CNTRL0 *)&iARCHI_CONSTRUCTISODPANDLOADLANG,
    (const struct _CNTRL0 *)&iARCHI_DESTRUCTISODP,
    (const struct _CNTRL0 *)&iARCHI_DESTRUCTEXTRAEVENTDP,
    0,
};

/* Architecture commands available while waiting for speech onset. */
static const struct _CNTRL0 *const ARCHI_SLEEP[] = {
    &iARCHI_GOTONEXTSTATE,
    (const struct _CNTRL0 *)&iARCHI_FLUSH_SLEEP_TORECOVER,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_START_DETECTION,
    (const struct _CNTRL0 *)&iARCHI_GETACTWORDS,
    (const struct _CNTRL0 *)&iARCHI_STOP_SPECSUB_SIL_QUEUE,
    (const struct _CNTRL0 *)&iARCHI_START_SPECSUB_SIL_QUEUE,
    0,
};

/* Architecture commands available while waiting for spelling input. */
static const struct _CNTRL0 *const ARCHI_SPELLSLEEP[] = {
    &iARCHI_GOTONEXTSTATE,
    (const struct _CNTRL0 *)&iARCHI_FLUSH_SLEEP_TOSPELLRECOVER,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_START_DETECTION,
    (const struct _CNTRL0 *)&iARCHI_GETACTWORDS,
    (const struct _CNTRL0 *)&iARCHI_STOP_SPECSUB_SIL_QUEUE,
    (const struct _CNTRL0 *)&iARCHI_START_SPECSUB_SIL_QUEUE,
    0,
};

/* Architecture commands available while waiting for isolated-word input. */
static const struct _CNTRL0 *const ARCHI_ISOWORDSLEEP[] = {
    &iARCHI_GOTONEXTSTATE,
    (const struct _CNTRL0 *)&iARCHI_FLUSH_SLEEP_TOISOWORDRECOVER,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_START_DETECTION,
    (const struct _CNTRL0 *)&iARCHI_GETACTWORDS,
    (const struct _CNTRL0 *)&iARCHI_STOP_SPECSUB_SIL_QUEUE,
    (const struct _CNTRL0 *)&iARCHI_START_SPECSUB_SIL_QUEUE,
    0,
};

/* Architecture commands available while running first-pass recognition. */
static const struct _CNTRL0 *const ARCHI_RUN[] = {
    (const struct _CNTRL0 *)&iARCHI_FLUSH,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_NO_START_DETECTION,
    (const struct _CNTRL0 *)&iARCHI_GETACTWORDS,
    (const struct _CNTRL0 *)&iARCHI_STOP_SPECSUB_SIL_QUEUE,
    (const struct _CNTRL0 *)&iARCHI_START_SPECSUB_SIL_QUEUE,
    0,
};

/* Architecture commands available while finishing the first pass. */
static const struct _CNTRL0 *const ARCHI_RECOVER[] = {
    &iARCHI_GOTONEXTSTATE,
    &iARCHI_GOTOIDLE,
    &iARCHI_FLUSH_IGNORE,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    0,
};

/* Architecture commands available while running the recognition second pass. */
static const struct _CNTRL0 *const ARCHI_SECONDPASS[] = {
    (const struct _CNTRL0 *)&iARCHI_LABEL_START_SECONDPASS,
    (const struct _CNTRL0 *)&iARCHI_LABEL_STOP_SECONDPASS,
    (const struct _CNTRL0 *)&iARCHI_SILADAPTOFF,
    &iARCHI_FLUSH_IGNORE,
    (const struct _CNTRL0 *)&iARCHI_NBESTDYNPROG_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_TRIGGER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_CHANGECTX,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_NBESTDP,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_TREEDP,
    0,
};

/* Architecture commands available while running the spelling second pass. */
static const struct _CNTRL0 *const ARCHI_SPELLSECONDPASS[] = {
    (const struct _CNTRL0 *)&iARCHI_LABEL_START_SECONDPASS,
    (const struct _CNTRL0 *)&iARCHI_LABEL_STOP_SECONDPASS,
    (const struct _CNTRL0 *)&iARCHI_SILADAPTOFF,
    &iARCHI_FLUSH_IGNORE,
    (const struct _CNTRL0 *)&iARCHI_NBESTDYNPROG_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_TRIGGER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_CHANGECTX,
    (const struct _CNTRL0 *)&iARCHI_GET_EXTRAEVENTSCORE,
    &iARCHI_GOTOIDLE,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    0,
};

/* Architecture commands available while prompting for a training utterance. */
static const struct _CNTRL0 *const ARCHI_PROMPT[] = {
    (const struct _CNTRL0 *)&iARCHI_INITUSERWORD,
    (const struct _CNTRL0 *)&iARCHI_USERTRAIN_OK_START,
    (const struct _CNTRL0 *)&iARCHI_USERTRAIN_OK_LASTUTT_START,
    &iARCHI_USERTRAIN_CANCEL,
    &iARCHI_USERTRAIN_NOK_ONCONFIRMORSTART,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_GENUWDP,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_RESIZEDISTANCEQUEUES,
    0,
};

/* Architecture commands available while waiting for the training utterance. */
static const struct _CNTRL0 *const ARCHI_RECORDSLEEP[] = {
    (const struct _CNTRL0 *)&iARCHI_FLUSH_RECORDSLEEP,
    &iARCHI_GOTONEXTSTATE,
    (const struct _CNTRL0 *)&iARCHI_START_DETECTION,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_STOP_SPECSUB_SIL_QUEUE,
    (const struct _CNTRL0 *)&iARCHI_START_SPECSUB_SIL_QUEUE,
    0,
};

/* Architecture commands available while recording the utterance. */
static const struct _CNTRL0 *const ARCHI_RECORD[] = {
    (const struct _CNTRL0 *)&iARCHI_NO_START_DETECTION,
    (const struct _CNTRL0 *)&iARCHI_FLUSH,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_STOP_SPECSUB_SIL_QUEUE,
    (const struct _CNTRL0 *)&iARCHI_START_SPECSUB_SIL_QUEUE,
    0,
};

/* Architecture commands available while accepting the recorded utterance. */
static const struct _CNTRL0 *const ARCHI_ACCEPT[] = {
    (const struct _CNTRL0 *)&iARCHI_GET_SESSION,
    &iARCHI_USERTRAIN_OK_ACCEPT,
    &iARCHI_USERTRAIN_OK_LASTUTT_ACCEPT,
    &iARCHI_USERTRAIN_NOK_ONACCEPT,
    &iARCHI_USERTRAIN_CANCEL,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    0,
};

/* Architecture commands available while confirming training. */
static const struct _CNTRL0 *const ARCHI_CONFIRM[] = {
    &iARCHI_USERTRAIN_OK_LASTUTT_CONFIRM,
    &iARCHI_USERTRAIN_NOK_ONCONFIRMORSTART,
    &iARCHI_USERTRAIN_CANCEL,
    (const struct _CNTRL0 *)&iARCHI_GET_SNR,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_GOTO_PROMPT,
    0,
};

/* Architecture commands available while making the trained user word available. */
static const struct _CNTRL0 *const ARCHI_UWTRAINED[] = {
    (const struct _CNTRL0 *)&iARCHI_PUT_SESSION,
    (const struct _CNTRL0 *)&iARCHI_GET_SESSION,
    (const struct _CNTRL0 *)&iARCHI_NO_PITCH_PROCESSING,
    (const struct _CNTRL0 *)&iARCHI_DO_PITCH_PROCESSING,
    (const struct _CNTRL0 *)&iARCHI_LOADLANGUAGE,
    (const struct _CNTRL0 *)&iARCHI_UNLOADLANGUAGE,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_NBESTDP,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_TREEDP,
    (const struct _CNTRL0 *)&iARCHI_CHANGECTX_TOIDLE,
    (const struct _CNTRL0 *)&iARCHI_CTXDELETED,
    (const struct _CNTRL0 *)&iARCHI_MIKE_SET,
    (const struct _CNTRL0 *)&iARCHI_SILADAPTOFF,
    &iARCHI_START_ARCHI_CONTROL,
    &iARCHI_STOP_ARCHI_CONTROL,
    (const struct _CNTRL0 *)&iARCHI_SILADATPTON,
    (const struct _CNTRL0 *)&iARCHI_VAD_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_TRIGGER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_GENDER_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_DISTCALC_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_NBESTDYNPROG_SET_PARAM,
    (const struct _CNTRL0 *)&iARCHI_USERWORDTRAININGSTART,
    (const struct _CNTRL0 *)&iARCHI_USERWORDTRAININGSTOP,
    (const struct _CNTRL0 *)&iARCHI_INITSCOREUSERWORD,
    (const struct _CNTRL0 *)&iARCHI_SETPARAMSCOREUSERWORD,
    (const struct _CNTRL0 *)&iARCHI_GETPARAMSCOREUSERWORD,
    (const struct _CNTRL0 *)&iARCHI_SWITCH_TO_SCOREUWDP,
    (const struct _CNTRL0 *)&iARCHI_LABEL_START_UW,
    (const struct _CNTRL0 *)&iARCHI_LABEL_STOP_UW,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_M,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_F,
    (const struct _CNTRL0 *)&iARCHI_DISABLE_C,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_M,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_F,
    (const struct _CNTRL0 *)&iARCHI_ENABLE_C,
    (const struct _CNTRL0 *)&iARCHI_GET_DISTCALCFUNCS,
    (const struct _CNTRL0 *)&iARCHI_SET_DISTCALCFUNCS,
    (const struct _CNTRL0 *)&iARCHI_GOTO_PROMPT,
    (const struct _CNTRL0 *)&iARCHI_CONSTRUCTISODPANDLOADLANG,
    (const struct _CNTRL0 *)&iARCHI_DESTRUCTISODP,
    0,
};

/* Commands from the gender classifier while holding the selected gender model. */
static const struct _CNTRL0 *const GENDER_FROZEN[] = {
    (const struct _CNTRL0 *)&iGENDER_DISABLE_M,
    (const struct _CNTRL0 *)&iGENDER_DISABLE_F,
    (const struct _CNTRL0 *)&iGENDER_DISABLE_C,
    0,
};

/* Commands from the speech trigger while waiting for a recognition request. */
static const struct _CNTRL0 *const TRIGGER_IDLE[] = {
    &iTRIGGER_GARBLED_SOUND_DETECTED,
    &iTRIGGER_TS_DETECTED_IGNORE,
    0,
};

/* Commands from the speech trigger while waiting for speech onset. */
static const struct _CNTRL0 *const TRIGGER_SLEEP[] = {
    (const struct _CNTRL0 *)&iTRIGGER_START_DETECTED_TO_NEXTSTATE,
    &iTRIGGER_GARBLED_SOUND_DETECTED,
    0,
};

/* Commands from the speech trigger while running first-pass recognition. */
static const struct _CNTRL0 *const TRIGGER_RUN[] = {
    (const struct _CNTRL0 *)&iTRIGGER_TS_DETECTED_TO_NEXTSTATE,
    &iTRIGGER_GARBLED_SOUND_DETECTED,
    0,
};

/* Commands from the speech trigger while finishing the first pass. */
static const struct _CNTRL0 *const TRIGGER_RECOVER[] = {
    &iTRIGGER_START_DETECTED_IGNORE,
    &iTRIGGER_TS_DETECTED_IGNORE,
    &iTRIGGER_GARBLED_SOUND_DETECTED,
    0,
};

/* Commands from the speech trigger while recording the utterance. */
static const struct _CNTRL0 *const TRIGGER_RECORD[] = {
    (const struct _CNTRL0 *)&iTRIGGER_TS_DETECTED_TO_NEXTSTATE,
    &iTRIGGER_GARBLED_SOUND_DETECTED,
    0,
};

/* Commands from the speech trigger while waiting for the training utterance. */
static const struct _CNTRL0 *const TRIGGER_RECORDSLEEP[] = {
    (const struct _CNTRL0 *)&iTRIGGER_START_DETECTED_TO_NEXTSTATE,
    &iTRIGGER_GARBLED_SOUND_DETECTED,
    0,
};

/* Commands from the speech trigger while accepting the recorded utterance. */
static const struct _CNTRL0 *const TRIGGER_ACCEPT[] = {
    &iTRIGGER_START_DETECTED_IGNORE,
    &iTRIGGER_TS_DETECTED_IGNORE,
    &iTRIGGER_GARBLED_SOUND_DETECTED,
    0,
};

/* Commands from the voice activity detector while recognizing or training. */
static const struct _CNTRL0 *const VAD_OTHER_STATES[] = {
    &iVAD_CHANGE_MIKE,
    &iVAD_EVENTRECEIVED,
    0,
};

/* Commands from the N-best decoder while finishing the first pass. */
static const struct _CNTRL0 *const NBESTDYNPROG_RECOVER[] = {
    &iNBESTDYNPROG_TSDETECTED_IGNORE,
    &iNBESTDYNPROG_WORDRECOGNISED,
    &iNBESTDYNPROG_NOWORDRECOGNISED,
    0,
};

/* Commands from the isolated-word decoder while finishing the first pass. */
static const struct _CNTRL0 *const ISOWORDTOKENDP_RECOVER[] = {
    &iNBESTDYNPROG_TSDETECTED_IGNORE,
    &iISOWORDTOKENDP_WORDRECOGNISED,
    &iNBESTDYNPROG_NOWORDRECOGNISED,
    0,
};

/* Commands from the N-best decoder while running the recognition second pass. */
static const struct _CNTRL0 *const NBESTDYNPROG_SECONDPASS[] = {
    &iNBESTDYNPROG_WORDRECOGNISED_SECONDPASS,
    &iNBESTDYNPROG_NOWORDRECOGNISED_SECONDPASS,
    0,
};

/* Commands from the N-best decoder while running the spelling second pass. */
static const struct _CNTRL0 *const NBESTDYNPROG_SPELLSECONDPASS[] = {
    &iNBESTDYNPROG_WORDRECOGNISED_SPELLSECONDPASS,
    &iNBESTDYNPROG_NOWORDRECOGNISED_SPELLSECONDPASS,
    0,
};

/* Dispatches architecture and voice activity detector commands while initializing the engine. */
static const struct _INBLOCK STD_BOOT[] = {
    { 0x00, ARCHI_BOOT },
    { 0x0B, VAD_BOOT_DATAREADY_IDLE },
    { 0, 0 },
};

/* Dispatches architecture and voice activity detector, user-word scorer and gender classifier
 * commands while waiting for a recognition context. */
static const struct _INBLOCK STD_DATAREADY[] = {
    { 0x00, ARCHI_DATAREADY },
    { 0x0B, VAD_BOOT_DATAREADY_IDLE },
    { 0x22, DPSCOREUW_DATAREADY },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector and gender classifier
 * commands while waiting for a recognition request. */
static const struct _INBLOCK STD_IDLE[] = {
    { 0x00, ARCHI_IDLE },
    { 0x27, TRIGGER_IDLE },
    { 0x0B, VAD_BOOT_DATAREADY_IDLE },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector and gender classifier
 * commands while waiting for speech onset. */
static const struct _INBLOCK STD_SLEEP[] = {
    { 0x00, ARCHI_SLEEP },
    { 0x27, TRIGGER_SLEEP },
    { 0x0B, VAD_OTHER_STATES },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector, N-best decoder and gender
 * classifier commands while running first-pass recognition. */
static const struct _INBLOCK STD_RUN[] = {
    { 0x00, ARCHI_RUN },
    { 0x27, TRIGGER_RUN },
    { 0x0B, VAD_OTHER_STATES },
    { 0x20, NBESTDYNPROG_RUN },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector, N-best decoder and gender
 * classifier commands while finishing the first pass. */
static const struct _INBLOCK STD_RECOVER[] = {
    { 0x00, ARCHI_RECOVER },
    { 0x27, TRIGGER_RECOVER },
    { 0x0B, VAD_OTHER_STATES },
    { 0x20, NBESTDYNPROG_RECOVER },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and N-best decoder and speech trigger commands while running the
 * recognition second pass. */
static const struct _INBLOCK STD_SECONDPASS[] = {
    { 0x00, ARCHI_SECONDPASS },
    { 0x20, NBESTDYNPROG_SECONDPASS },
    { 0x27, TRIGGER_RECOVER },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger and voice activity detector commands while waiting
 * for spelling input. */
static const struct _INBLOCK STD_SPELLSLEEP[] = {
    { 0x00, ARCHI_SPELLSLEEP },
    { 0x27, TRIGGER_SLEEP },
    { 0x0B, VAD_OTHER_STATES },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector, N-best decoder and gender
 * classifier commands while recognizing spelling input. */
static const struct _INBLOCK STD_SPELLRUN[] = {
    { 0x00, ARCHI_RUN },
    { 0x27, TRIGGER_RUN },
    { 0x0B, VAD_OTHER_STATES },
    { 0x20, NBESTDYNPROG_RUN },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, N-best decoder, voice activity detector and gender
 * classifier commands while finishing the spelling first pass. */
static const struct _INBLOCK STD_SPELLRECOVER[] = {
    { 0x00, ARCHI_RECOVER },
    { 0x27, TRIGGER_RECOVER },
    { 0x20, NBESTDYNPROG_RECOVER },
    { 0x0B, VAD_OTHER_STATES },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and N-best decoder and speech trigger commands while running the
 * spelling second pass. */
static const struct _INBLOCK STD_SPELLSECONDPASS[] = {
    { 0x00, ARCHI_SPELLSECONDPASS },
    { 0x20, NBESTDYNPROG_SPELLSECONDPASS },
    { 0x27, TRIGGER_RECOVER },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger and voice activity detector commands while waiting
 * for isolated-word input. */
static const struct _INBLOCK STD_ISOWORDSLEEP[] = {
    { 0x00, ARCHI_ISOWORDSLEEP },
    { 0x27, TRIGGER_SLEEP },
    { 0x0B, VAD_OTHER_STATES },
    { 0, 0 },
};

/* Dispatches commands to architecture, the speech trigger, the voice activity detector,
 * the isolated-word decoder and the gender classifier during isolated-word recognition. */
static const struct _INBLOCK STD_ISOWORDRUN[] = {
    { 0x00, ARCHI_RUN },
    { 0x27, TRIGGER_RUN },
    { 0x0B, VAD_OTHER_STATES },
    { 0x23, NBESTDYNPROG_RUN },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector, isolated-word decoder and
 * gender classifier commands while finishing isolated-word recognition. */
static const struct _INBLOCK STD_ISOWORDRECOVER[] = {
    { 0x00, ARCHI_RECOVER },
    { 0x27, TRIGGER_RECOVER },
    { 0x0B, VAD_OTHER_STATES },
    { 0x23, ISOWORDTOKENDP_RECOVER },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture commands while prompting for a training utterance. */
static const struct _INBLOCK STD_PROMPT[] = {
    { 0x00, ARCHI_PROMPT },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector and gender classifier
 * commands while waiting for the training utterance. */
static const struct _INBLOCK STD_RECORDSLEEP[] = {
    { 0x00, ARCHI_RECORDSLEEP },
    { 0x27, TRIGGER_RECORDSLEEP },
    { 0x0B, VAD_OTHER_STATES },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector, user-word generator and
 * gender classifier commands while recording the utterance. */
static const struct _INBLOCK STD_RECORD[] = {
    { 0x00, ARCHI_RECORD },
    { 0x27, TRIGGER_RECORD },
    { 0x0B, VAD_OTHER_STATES },
    { 0x21, DPGENUW_RECORD },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and speech trigger, voice activity detector, user-word generator and
 * gender classifier commands while accepting the recorded utterance. */
static const struct _INBLOCK STD_ACCEPT[] = {
    { 0x00, ARCHI_ACCEPT },
    { 0x27, TRIGGER_ACCEPT },
    { 0x0B, VAD_OTHER_STATES },
    { 0x21, DPGENUW_ACCEPT },
    { 0x13, GENDER_FROZEN },
    { 0, 0 },
};

/* Dispatches architecture and voice activity detector commands while confirming training. */
static const struct _INBLOCK STD_CONFIRM[] = {
    { 0x00, ARCHI_CONFIRM },
    { 0x0B, VAD_OTHER_STATES },
    { 0, 0 },
};

/* Dispatches architecture and voice activity detector and user-word scorer commands while making
 * the trained user word available. */
static const struct _INBLOCK STD_UWTRAINED[] = {
    { 0x00, ARCHI_UWTRAINED },
    { 0x0B, VAD_BOOT_DATAREADY_IDLE },
    { 0x22, DPSCOREUW_DATAREADY },
    { 0, 0 },
};

/* Control sources available in each state of this scheduler mode. */
static const struct _INBLOCK *const STD_[] = {
    STD_BOOT,
    STD_DATAREADY,
    STD_IDLE,
    STD_SLEEP,
    STD_RUN,
    STD_RECOVER,
    STD_SECONDPASS,
    STD_SPELLSLEEP,
    STD_SPELLRUN,
    STD_SPELLRECOVER,
    STD_SPELLSECONDPASS,
    STD_ISOWORDSLEEP,
    STD_ISOWORDRUN,
    STD_ISOWORDRECOVER,
    STD_SECONDPASS,
    STD_PROMPT,
    STD_RECORDSLEEP,
    STD_RECORD,
    STD_ACCEPT,
    STD_CONFIRM,
    STD_UWTRAINED,
    0,
};

/* Control routes indexed by recognition mode, then scheduler state. */
const struct _INBLOCK *const *const ControlTable[] = {
    STD_,
    0,
    0,
};
