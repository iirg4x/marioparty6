extern "C" { typedef struct QuizAnswerIds { s32 values[20]; } QuizAnswerIds; }
extern "C" { typedef struct QuizSoundIds { s32 values[17]; } QuizSoundIds; }
extern "C" { s32 lbl_1_data_0[7] = { 0xA4007A, 0xA4007B, 0xA4007C, 0xA4007D, 0xA4007E, 0xA4007F, 0xA40080 }; }

s32 lbl_1_data_1C[0xB] = { 0x9B0000, 0x9B0001, 0x9B0002, 0x9B0003, 0x9B0004, 0x9B0005, 0x9B0006, 0x9B0007, 0x9B0008, 0x9B0009, 0x9B000A };
s32 lbl_1_data_48[5] = { 10, 20, 30, 40, 50 };
u32 lbl_1_data_5C[0x11] = { 0x21000C, 0x21000D, 0x21000E, 0x21000F, 0x210010, 0x210011, 0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 0x490016 };
u32 lbl_1_data_A0[18] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, MESSNUM(MESS_QUIZ_CHARA, 12), MESSNUM(MESS_QUIZ_CHARA, 13), MESSNUM(MESS_QUIZ_CHARA, 14), MESSNUM(MESS_QUIZ_CHARA, 15), MESSNUM(MESS_QUIZ_CHARA, 16), MESSNUM(MESS_QUIZ_CHARA, 17), MESSNUM(MESS_MIC_RETYPING, 22) };
u32 lbl_1_data_E8[0x11] = { 0, 1, 2, 3, 4, 5, 6, 7, 9, 10, 0x21000C, 0x21000D, 0x21000E, 0x21000F, 0x210010, 0x210011, 0x490016 };
u32 lbl_1_data_12C[0x14] = { 0x210022, 0x210023, 0x210024, 0x210025, 0x210026, 0x210027, 0x210028, 0x210029, 0x21002A, 0x21002B, 0x21002C, 0x21002D, 0x21002E, 0x21002F, 0x210030, 0x210031, 0x210032, 0x210033, 0x210034, 0x210035 };
u32 lbl_1_data_17C[0x14] = { 0x21003A, 0x21003B, 0x21003C, 0x21003D, 0x21003E, 0x21003F, 0x210040, 0x210041, 0x210042, 0x210043, 0x210044, 0x210045, 0x210046, 0x210047, 0x210048, 0x210049, 0x21004A, 0x21004B, 0x21004C, 0x21004D };
u32 lbl_1_data_1CC[2] = { 0x210037, 0x210038 };
u32 lbl_1_data_1D4[8] = { 0x210019, 0x21001A, 0x21001B, 0x21001C, 0x21001D, 0x21001E, 0x21001F, 0x210020 };
s32 lbl_1_data_1F4 = -1;
s32 lbl_1_data_1F8 = 0xFFFFFFFF;
s32 lbl_1_data_1FC = -1;
/* Unknown initialized bytes at .data 0x200..0x208. */
u8 lbl_1_data_200[8] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
s16 lbl_1_data_208 = -1;
s16 lbl_1_data_20A = -1;
s16 lbl_1_data_20C = -1;
s16 lbl_1_data_20E = -1;
s16 lbl_1_data_210[4] = { -1, -1, -1, -1 };
s16 lbl_1_data_218 = -1;
s16 lbl_1_data_21A = -1;
struct _struct_lbl_1_data_21C_0x8 lbl_1_data_21C[0xC] = { { 222.0f, 40.0f }, { 357.0f, 55.0f }, { 227.0f, 74.0f }, { 345.0f, 80.0f }, { 0.00000000000000000000000000000000000000266323f, 0.000000000000000000000000000000000000002663231f }, { 0.000000000000000000000000000000000000002663232f, 0.000000000000000000000000000000000000002663234f }, { 0.000000000000000000000000000000000000002663236f, 0.000000000000000000000000000000000000002663238f }, { 0.000000000000000000000000000000000000002663239f, 0.00000000000000000000000000000000000000266324f }, { 0.000000000000000000000000000000000000002663245f, 0.000000000000000000000000000000000000002663246f }, { 0.000000000000000000000000000000000000002663248f, 0.000000000000000000000000000000000000002663249f }, { 0.00000000000000000000000000000000000000266325f, 0.000000000000000000000000000000000000002663252f }, { 0.000000000000000000000000000000000000002663253f, 0.000000000000000000000000000000000000002663255f } };

extern "C++" void fn_1_8394(s32 count)
{
    lbl_1_bss_4B8 = count;
}

extern "C++" void fn_1_83A4(void)
{
    lbl_1_bss_4A0 = 1;
}

extern "C++" void fn_1_83B8(void) {
    fn_1_16988(2);
    lbl_1_bss_4E4 = 1;
    lbl_1_bss_4AC = 1;
}

extern "C++" s32 fn_1_83FC(void)
{
    return lbl_1_bss_4AC;
}

void fn_1_16988(s32);

extern "C++" void fn_1_840C(void) {
    s32 playerIndex;
    u8 disabledPlayerMask;

    disabledPlayerMask = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_102B0(playerIndex) == 0) {
            disabledPlayerMask |= 1 << playerIndex;
        }
        playerIndex += 1;
    }
    HuWinDisablePlayerReset(lbl_1_bss_32A, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_328, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_326, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_324, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_34E, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_34C, 0xFF);
    HuWinDisablePlayerSet(lbl_1_bss_32A, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_328, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_326, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_324, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_34E, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_34C, disabledPlayerMask);
}

extern "C++" void fn_1_8560(void) {
    GXColor colorA = lbl_1_rodata_10;
    GXColor colorB = lbl_1_rodata_14;
    s32 playerIndex;
    u8 disabledPlayerMask;

    lbl_1_bss_32A = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 6, 4);
    lbl_1_bss_328 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 7, 3);
    lbl_1_bss_326 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 0x17, 5);
    lbl_1_bss_324 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 0xD, 0);

    disabledPlayerMask = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_102B0(playerIndex) == 0) {
            disabledPlayerMask |= 1 << playerIndex;
        }
    }

    HuWinDisablePlayerReset(lbl_1_bss_32A, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_328, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_326, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_324, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_34E, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_34C, 0xFF);
    HuWinDisablePlayerSet(lbl_1_bss_32A, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_328, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_326, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_324, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_34E, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_34C, disabledPlayerMask);
    HuWinAttrSet(lbl_1_bss_32A, 0x4000);
    HuWinAttrSet(lbl_1_bss_328, 0x4000);
    HuWinAttrSet(lbl_1_bss_326, 0x4000);
    HuWinAttrSet(lbl_1_bss_324, 0x4000);

    lbl_1_bss_322 = HuWinCreate(16.0f, 312.0f, 0x220, 0x20, 0);
    HuWinBGTPLvlSet(lbl_1_bss_322, 0.0f);
    HuWinMesSpeedSet(lbl_1_bss_322, 0);
    HuWinAttrSet(lbl_1_bss_322, 0x800);
    HuWinDispOff(lbl_1_bss_32A);
    HuWinDispOff(lbl_1_bss_328);
    HuWinDispOff(lbl_1_bss_326);
    HuWinDispOff(lbl_1_bss_324);
    HuWinPriSet(lbl_1_bss_32A, 0x64);
    HuWinPriSet(lbl_1_bss_328, 0x64);
    HuWinPriSet(lbl_1_bss_326, 0x64);
    HuWinPriSet(lbl_1_bss_324, 0x64);
    HuWinPriSet(lbl_1_bss_322, 0x64);
    HuWinCallbackSet(lbl_1_bss_32A, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_328, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_326, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_324, fn_1_19FB0);
    lbl_1_bss_31E = -1;
}

s32 fn_1_102B0(s32);

extern s16 lbl_1_bss_34C;

extern s16 lbl_1_bss_34E;

extern "C++" s16 fn_1_89C0(s32 windowNo) {
    s16 windowId;

    switch (windowNo) {
    case 0:
        windowId = lbl_1_bss_32A;
        break;
    case 1:
        windowId = lbl_1_bss_328;
        break;
    case 2:
        windowId = lbl_1_bss_326;
        break;
    case 3:
        windowId = lbl_1_bss_324;
        break;
    }
    return windowId;
}

extern s16 lbl_1_bss_322;

extern "C" {
void fn_1_19FB0(HUWINID winId, u32 answerId, s16 soundIndex);
}

extern "C++" void fn_1_8A3C(void) {
    f32 amplitude;
    f32 angle;
    f32 sineOffset;

    amplitude = 362.0f;
    HuSprAttrReset(lbl_1_bss_410, 0, 4);
    angle = 0.0f;
    if (lbl_1_bss_480 == 0) {
        do {
            HuPrcVSleep();
            sineOffset = (f32) ((f64) amplitude * sin((M_PI * (f64) angle) / 180.0));
            HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484 + sineOffset);
            angle += 3.0f;
        } while (!(angle >= 90.0f));
        lbl_1_bss_484 = 210.0f;
        HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484);
        lbl_1_bss_480 = 1;
    }
}

/* fn_1_18964: see copies/ (exact copy of an analog) */

/* fn_1_18DD0: see copies/ (exact copy of an analog) */

/* fn_1_19118: see copies/ (exact copy of an analog) */

extern s16 lbl_1_bss_410;

extern s32 lbl_1_bss_480;

extern f32 lbl_1_bss_484;

extern f32 lbl_1_bss_488;

extern "C++" void fn_1_8BA8(void) {
    f32 amplitude;
    f32 angle;
    f32 sineOffset;

    amplitude = -362.0f;
    angle = 0.0f;
    if (lbl_1_bss_480 != 0) {
        do {
            HuPrcVSleep();
            sineOffset = (f32) ((f64) amplitude * sin((M_PI * (f64) angle) / 180.0));
            HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484 + sineOffset);
            angle += 3.0f;
        } while (!(angle >= 90.0f));
        lbl_1_bss_484 = -152.0f;
        HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484);
        HuSprAttrSet(lbl_1_bss_410, 0, 4);
        lbl_1_bss_480 = 0;
    }
}

extern "C++" void fn_1_8D14(u32 messageId, s16 messageSpeed, u8 windowNo)
{
    s16 window = fn_1_89C0(windowNo);

    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

extern s16 lbl_1_bss_382;

extern s16 lbl_1_bss_328;

extern s16 lbl_1_bss_32A;

static void fn_1_20B74_inlined(s32 windowNo);

static void fn_1_20D08_inlined(s32 windowNo, u32 messageId, s16 insertNo);

static void fn_1_20A50_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);

static void fn_1_20C94_inlined(void);

static s16 fn_1_206FC_inlined(s32 windowNo);

static void fn_1_20B24_inlined(void);

void fn_1_1850C(s32 primaryMessageIndex, s32 secondaryMessageIndex);

extern s16 lbl_1_bss_CE;

extern s16 lbl_1_bss_328;

extern s16 lbl_1_bss_D2[4];

static void fn_1_1278_inlined(s32 windowNo);

static void fn_1_140C_inlined(s32 windowNo, u32 messageId, s16 insertNo);

static void fn_1_1154_inlined(u32 messageId, s16 messageSpeed, u8 windowNo);

static void fn_1_1398_inlined(void);

static s16 fn_1_E00_inlined(s32 windowNo);

static void fn_1_1228_inlined(void);

void fn_1_18964(s32 primaryMessageIndex, s32 secondaryMessageIndex);

static void fn_1_1278_inlined_11558_inlined(s32 windowNo);

static void fn_1_140C_inlined_11558_inlined(s32 windowNo, u32 messageId, s16 insertNo);

static void fn_1_1154_inlined_11558_inlined(u32 messageId, s16 messageSpeed, u8 windowNo);

static void fn_1_1398_inlined_11558_inlined(void);

static s16 fn_1_E00_inlined_11558_inlined(s32 windowNo);

static void fn_1_1228_inlined_11558_inlined(void);

void fn_1_19118(s32 messageIndex);

static void fn_1_1278_inlined_8BC8_inlined(s32 windowNo);

static void fn_1_140C_inlined_8BC8_inlined(s32 windowNo, u32 messageId, s16 insertNo);

static void fn_1_1154_inlined_8BC8_inlined(u32 messageId, s16 messageSpeed, u8 windowNo);

static void fn_1_1398_inlined_8BC8_inlined(void);

static s16 fn_1_E00_inlined_8BC8_inlined(s32 windowNo);

static void fn_1_1228_inlined_8BC8_inlined(void);

void fn_1_109C8(u32 message);

static void fn_1_8BC8_inlined(u32 message);

static void fn_1_8E08_inlined(u32 message);

void fn_1_17248(s32 mode, s32 wait);

extern "C++" void fn_1_8DE8(void) {
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

extern "C++" void fn_1_8E38(s32 windowNo)
{
    s16 window = fn_1_89C0(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_8DE8();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

extern "C" void fn_1_8F58(void) {
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

extern "C++" short fn_1_8F98(short choice) {
    return HuWinChoiceGet(lbl_1_bss_31E,choice);
}

extern "C++" void fn_1_8FCC(s32 windowNo, u32 messageId, s16 insertNo) {
    HuWinInsertMesSet(fn_1_89C0(windowNo), messageId, insertNo);
}

extern s16 lbl_1_bss_320;

extern "C++" void fn_1_9080(void) {
    HuWinHomeClear(lbl_1_bss_320);
}

extern "C++" void fn_1_90AC(void) {
    HuWinHomeClear(lbl_1_bss_322);
}

/* extern */
/* extern */

extern "C++" void fn_1_90D8(s32 visible) {

    if (visible)
    {
        HuWinDispOn(lbl_1_bss_322);
        return;
    }
    HuWinDispOff(lbl_1_bss_322);
}

extern s32 lbl_1_data_1F4;

extern "C++" void fn_1_9128(u32 messageType) {
    u32 messageId;

    if (lbl_1_data_1F4 != (s32) messageType) {
        switch (messageType) {
        case 0:
            messageId = 0x10017;
            break;
        case 1:
            messageId = 0x10018;
            break;
        case 2:
            messageId = 0x10019;
            break;
        case 3:
            messageId = 0x1001A;
            break;
        case 4:
            messageId = 0x1001B;
            break;
        case 7:
            messageId = 0x1001E;
            break;
        case 8:
            messageId = 0x1001F;
            break;
        case 9:
            messageId = 0x1001E;
            break;
        case 10:
            messageId = 0x1001F;
            break;
        }
        HuWinMesSet(lbl_1_bss_322, messageId);
        lbl_1_data_1F4 = messageType;
    }
}

/* extern */

extern "C++" void fn_1_9214(s32 messageType) {

    s32 messageId;
    switch (messageType)
    {
    case 5 :
        messageId = 0x1001C;
        break;
    case 6 :
        messageId = 0x1001D;
        break;
    }
    HuWinMesSet(lbl_1_bss_320, messageId);
}

extern ANIMDATA *lbl_1_bss_14;

extern f32 lbl_1_bss_18;

extern "C++" void fn_1_9280(s16 layerNo)
{
    Mtx44 projection;
    Mtx textureMatrix;

    if (lbl_1_bss_14) {
        GXInvalidateTexAll();
        C_MTXOrtho(projection, 0.0f, 480.0f,
                   0.0f, 576.0f, 0.0f, 10.0f);
        GXSetProjection(projection, GX_ORTHOGRAPHIC);

        if (RenderMode->field_rendering != 0) {
            GXSetViewportJitter(0.0f, 0.0f, 640.0f, 480.0f,
                                0.0f, 1.0f, VIGetNextField());
        } else {
            GXSetViewport(0.0f, 0.0f, 640.0f, 480.0f,
                          0.0f, 1.0f);
        }

        GXSetScissor(0, 0, 640, 480);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                       GX_LO_NOOP);
        GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                      GX_DF_CLAMP, GX_AF_SPOT);
        GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL,
                      GX_DF_CLAMP, GX_AF_SPOT);

        PSMTXIdentity(textureMatrix);
        GXLoadPosMtxImm(textureMatrix, GX_PNMTX0);
        PSMTXTrans(textureMatrix, -lbl_1_bss_18, -lbl_1_bss_18, 0.0f);
        GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX2x4);
        HuSprTexLoad(lbl_1_bss_14, 0, GX_TEXMAP0,
                     GX_REPEAT, GX_REPEAT, GX_LINEAR);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                          GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0,
                      GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                        GX_CC_TEXC);
        GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_TEXA);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);

        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2f32(0.0f, 0.0f);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition2f32(576.0f, 0.0f);
        GXTexCoord2f32(36.0f, 0.0f);
        GXPosition2f32(576.0f, 480.0f);
        GXTexCoord2f32(36.0f, 30.0f);
        GXPosition2f32(0.0f, 480.0f);
        GXTexCoord2f32(0.0f, 30.0f);

        lbl_1_bss_18 += 0.02f;
        if (lbl_1_bss_18 >= 0.999f) {
            lbl_1_bss_18 = 0.0f;
        }
    }
}

extern s32 lbl_1_bss_1C;

extern f32 lbl_1_bss_18;

void fn_1_9868(void);

extern "C" {
void fn_1_19BD4(s32);
}

extern "C++" void fn_1_9784(HUPROCESS *parentProcess) {
    f32 perspective;
    f32 field10;
    f32 fieldC;
    s32 camera;
    s32 matchedBit;

    lbl_1_bss_348 = fn_8000DAC0(fn_1_9868, -0x218, 0x2000, 0, fn_8000DD9C());
    HuPrcSetStat(lbl_1_bss_348, 0xC);
    fn_1_19BD4(0);
    camera = 0;
    while (camera < 0x10) {
        fn_80027D8C(1 << camera, &perspective, &field10, &fieldC);
        if (-1.0f != perspective) {
            matchedBit = 1 << camera;
        }
        camera += 1;
    }
    lbl_1_bss_3E0 = matchedBit;
    lbl_1_bss_4EC = parentProcess;
}

extern "C" {
extern "C" s16 HuMCSelModeGet(void);
}

extern s32 lbl_1_bss_48C;

extern "C" {
extern "C" s32 omPadErrChk(u8 playerIndex);
}

void fn_1_9D5C(void);

void fn_1_9E44(void);

extern "C++" void fn_1_9868(void) {
    u8 playerIndex;
    u8 selectedPlayer;
    s32 playerSelected;
    u8 disabledPlayers;
    s16 windowNo;
    u32 homeMessageId;
    u32 confirmMessageId;

    for (;;) {
        playerSelected = 0;
        for (;;) {
            HuPrcVSleep();
            if (WipeCheck() != 0) continue;
            if (fn_8003B774() == -1) continue;
            if (GameMesStatGet(-1) != 0) continue;
            if (HuMCSelModeGet() == 1) continue;
            if (lbl_802C0234 + fn_8003B748() != 0) continue;
            if (lbl_1_bss_47C->pauseAllowed == 0) continue;

            for (playerIndex = 0; playerIndex < 4U; playerIndex++) {
                if (omPadErrChk((u8) playerIndex) == 0 && (lbl_802C0008[playerIndex] & 0x1000) != 0) {
                    selectedPlayer = playerIndex;
                    lbl_1_bss_47C->pauseDisplayActive = 1;
                    WipeCreate(2, 0, 0xA);
                    while (WipeCheck() != 0) {
                        HuPrcVSleep();
                    }
                    HuWinDispOff(lbl_1_bss_322);
                    fn_1_9D5C();
                    HuWinHomeClear(lbl_1_bss_320);
                    homeMessageId = MESSNUM(MESS_SYS_GUIDE, 28);
                    HuWinMesSet(lbl_1_bss_320, homeMessageId);
                    playerSelected = 1;
                    WipeCreate(1, 0, 0xA);
                    while (WipeCheck() != 0) {
                        HuPrcVSleep();
                    }
                    break;
                }
            }
            if (playerSelected != 0) {
                break;
            }
        }

        for (;;) {
            HuPrcVSleep();
            if ((lbl_802C0008[selectedPlayer] & 0x1000) != 0) {
                HuAudFXPlay(MSM_SE_CMN_START_BUTTON_CONFIRM);
                WipeCreate(2, 0, 0xA);
                while (WipeCheck() != 0) {
                    HuPrcVSleep();
                }
                fn_1_9E44();
                HuWinHomeClear(lbl_1_bss_320);
                HuWinDispOn(lbl_1_bss_322);
                lbl_1_bss_47C->pauseDisplayActive = 0;
                WipeCreate(1, 0, 0xA);
                while (WipeCheck() != 0) {
                    HuPrcVSleep();
                }
                break;
            }
            if ((lbl_802C0008[selectedPlayer] & 0x400) == 0) {
                continue;
            }

            HuAudFXPlay(MSM_SE_CMN_CHOICE_CANCEL);
            HuWinPosSet(lbl_1_bss_320, 16.0f, 312.0f);
            disabledPlayers = 0;
            for (playerIndex = 0; playerIndex < 4U; playerIndex++) {
                if (playerIndex != selectedPlayer) {
                    disabledPlayers |= 1 << playerIndex;
                }
            }
            if (lbl_1_bss_47C->presenterLayout == 0) {
                windowNo = lbl_1_bss_34E;
            } else {
                windowNo = lbl_1_bss_34C;
            }
            HuWinDisablePlayerSet(windowNo, disabledPlayers);
            confirmMessageId = MESSNUM(MESS_SYS_GUIDE, 29);
            HuWinMesSet(lbl_1_bss_320, confirmMessageId);
            HuWinExOpen(windowNo);
            HuWinAttrSet(windowNo, 0x10);
            if (lbl_1_bss_47C->presenterLayout == 0) {
                HuWinMesSet(windowNo, MESSNUM(MESS_MICQUIZ, 47));
            } else {
                HuWinMesSet(windowNo, MESSNUM(MESS_MICQUIZ, 101));
            }
            HuWinMesSpeedSet(windowNo, 1);
            HuWinMesWait(windowNo);
            if (HuWinChoiceGet(windowNo, -1) == 0) {
                HuAudFXPlay(MSM_SE_CMN_START_BUTTON_CONFIRM);
                WipeCreate(2, 0, 0xA);
                while (WipeCheck() != 0) {
                    HuPrcVSleep();
                }
                HuWinExClose(windowNo);
                HuWinHomeClear(lbl_1_bss_320);
                fn_1_9E44();
                HuWinDispOn(lbl_1_bss_322);
                lbl_1_bss_47C->pauseDisplayActive = 0;
                HuWinDisablePlayerReset(windowNo, 0xF);
                WipeCreate(1, 0, 0xA);
                while (WipeCheck() != 0) {
                    HuPrcVSleep();
                }
                break;
            }

            HuAudSStreamFadeOut(lbl_1_bss_48C, 0x258);
            WipeCreate(2, 0, 0x3C);
            while (WipeCheck() != 0) {
                HuPrcVSleep();
            }
            fn_1_9E44();
            HuMCClose();
            HuMCSessionClose();
            HuWinDisablePlayerReset(windowNo, 0xF);
            if (lbl_1_bss_47C->compactPauseDisplay != 0) {
                fn_8003A5E4(1, 1);
            } else {
                fn_8003A5E4(2, 1);
            }
        }
    }
}

extern s32 lbl_1_bss_340;

extern s32 lbl_1_bss_344;

extern "C++" void fn_1_9D5C(void) {
    lbl_1_bss_340 = (s32) omSysPauseEnableFlag;
    lbl_1_bss_344 = (s32) fn_8003B748();
    lbl_802C0234 = 1;
    omSysPauseEnable(0U);
    fn_8002583C((s16) lbl_1_bss_3E0, 0xF, fn_1_9280);
    if (lbl_1_bss_47C->compactPauseDisplay != 0) {
        fn_1_E8A8();
        fn_1_DDF8();
    } else {
        fn_1_E4E8();
    }
    fn_1_DF4C();
    fn_8003B674(1);
    HuPrcAllPause(1);
    Hu3DPauseSet(1);
    HuSprPauseSet(1);
    fn_80006F3C();
    HuAudFXPauseAll(1);
    HuAudSStreamPauseAll(1);
    HuAudFXPlay(MSM_SE_CMN_START_BUTTON_CONFIRM);
}

extern "C++" void fn_1_9E44(void) {
    lbl_802C0234 = 0;
    omSysPauseEnable((u8) lbl_1_bss_340);
    fn_8003B674(0);
    fn_800258A8((s16) lbl_1_bss_3E0, 0xF);
    if (lbl_1_bss_47C->compactPauseDisplay != 0) {
        fn_1_E7EC();
    } else {
        fn_1_E1E4();
    }
    fn_1_E0A0();
    if (lbl_1_bss_344 == 0) {
        HuPrcAllPause(0);
        Hu3DPauseSet(0);
        HuSprPauseSet(0);
        HuAudFXPauseAll(0);
        HuAudSStreamPauseAll(0);
    }
}

extern s16 lbl_1_bss_34E;

extern s16 lbl_1_bss_34C;

extern HUPROCESS *lbl_1_bss_348;

extern s32 lbl_1_bss_344;

extern s32 lbl_1_bss_340;

extern ANIMDATA *lbl_1_bss_3E8;

extern ANIMDATA *lbl_1_bss_3EC;

extern ANIMDATA *lbl_1_bss_3F0;

extern ANIMDATA *lbl_1_bss_3F4[4];

extern s16 lbl_1_bss_570;

extern s32 lbl_1_data_1C[0xB];

extern ANIMDATA *lbl_1_bss_454;

void fn_1_C3FC(void);

void fn_1_C768(void);

s32 fn_1_10498(void);

void fn_1_15A0C(s32 mode);

extern ANIMDATA *lbl_1_bss_3E4;

extern s16 lbl_1_bss_40A;

extern s16 lbl_1_bss_40C;

extern s16 lbl_1_bss_40E;

extern s16 lbl_1_bss_412;

extern s16 lbl_1_bss_414;

extern s16 lbl_1_bss_416;

extern s16 lbl_1_bss_418;

extern s16 lbl_1_bss_422;

extern ANIMDATA *lbl_1_bss_424[2];

extern ANIMDATA *lbl_1_bss_42C;

extern ANIMDATA *lbl_1_bss_430;

extern ANIMDATA *lbl_1_bss_434;

extern ANIMDATA *lbl_1_bss_438;

extern ANIMDATA *lbl_1_bss_43C;

extern ANIMDATA *lbl_1_bss_440;

extern ANIMDATA *lbl_1_bss_444;

extern ANIMDATA *lbl_1_bss_448;

extern ANIMDATA *lbl_1_bss_478;

extern ANIMDATA *lbl_1_bss_470[2];

extern ANIMDATA *lbl_1_bss_46C;

extern ANIMDATA *lbl_1_bss_468;

extern ANIMDATA *lbl_1_bss_464;

extern ANIMDATA *lbl_1_bss_460;

extern ANIMDATA *lbl_1_bss_45C;

extern ANIMDATA *lbl_1_bss_458;

extern ANIMDATA *lbl_1_bss_450;

extern s16 lbl_1_bss_44C;

extern HUPROCESS *lbl_1_bss_2E4[0xA];

extern s16 lbl_1_bss_332;

extern s16 lbl_1_bss_334;

extern ANIMDATA *lbl_1_bss_338;

extern ANIMDATA *lbl_1_bss_33C;

extern s16 lbl_1_data_20A;

extern s16 lbl_1_data_20C;

extern s16 lbl_1_data_20E;

extern s16 lbl_1_data_210[4];

extern s16 lbl_1_data_218;

extern s16 lbl_1_data_21A;

extern struct _struct_lbl_1_data_21C_0x8 lbl_1_data_21C[0xC];

static void fn_1_13F8_inlined(void);

static void fn_1_12A4_inlined(void);

static inline void fn_1_12A4_inlined(void) {
    s32 playerIndex;
    u8 disabledPlayerMask;

    disabledPlayerMask = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_102B0(playerIndex) == 0) {
            disabledPlayerMask |= 1 << playerIndex;
        }
        playerIndex += 1;
    }
    HuWinDisablePlayerReset(lbl_1_bss_32A, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_328, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_326, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_324, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_34E, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_34C, 0xFF);
    HuWinDisablePlayerSet(lbl_1_bss_32A, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_328, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_326, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_324, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_34E, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_34C, disabledPlayerMask);
}

static inline void fn_1_13F8_inlined(void) {
    GXColor colorA = lbl_1_rodata_10;
    GXColor colorB = lbl_1_rodata_14;
    s32 playerIndex;

    lbl_1_bss_32A = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 6, 4);
    lbl_1_bss_328 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 7, 3);
    lbl_1_bss_326 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 0x17, 5);
    lbl_1_bss_324 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 0xD, 0);

    fn_1_12A4_inlined();
    HuWinAttrSet(lbl_1_bss_32A, 0x4000);
    HuWinAttrSet(lbl_1_bss_328, 0x4000);
    HuWinAttrSet(lbl_1_bss_326, 0x4000);
    HuWinAttrSet(lbl_1_bss_324, 0x4000);

    lbl_1_bss_322 = HuWinCreate(16.0f, 312.0f, 0x220, 0x20, 0);
    HuWinBGTPLvlSet(lbl_1_bss_322, 0.0f);
    HuWinMesSpeedSet(lbl_1_bss_322, 0);
    HuWinAttrSet(lbl_1_bss_322, 0x800);
    HuWinDispOff(lbl_1_bss_32A);
    HuWinDispOff(lbl_1_bss_328);
    HuWinDispOff(lbl_1_bss_326);
    HuWinDispOff(lbl_1_bss_324);
    HuWinPriSet(lbl_1_bss_32A, 0x64);
    HuWinPriSet(lbl_1_bss_328, 0x64);
    HuWinPriSet(lbl_1_bss_326, 0x64);
    HuWinPriSet(lbl_1_bss_324, 0x64);
    HuWinPriSet(lbl_1_bss_322, 0x64);
    HuWinCallbackSet(lbl_1_bss_32A, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_328, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_326, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_324, fn_1_19FB0);
    lbl_1_bss_31E = -1;
}

extern "C++" void fn_1_9F08(void) {

    s32 quizMode;

    s32 playerIndex;
    s32 character;

    lbl_1_bss_47C = (MicquizModeWork *)MgModeWork;
lbl_1_bss_570 = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_570, 493.0f, 386.0f);
playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_102B0(playerIndex) == 0) {
            lbl_1_bss_3F4[playerIndex] = HuSprAnimRead(fn_80008DD0(0x9B0000, 0x10000000, 2));
            HuSprGrpMemberSet(lbl_1_bss_570, (s16) (playerIndex + 4),
                HuSprCreate(lbl_1_bss_3F4[playerIndex], 0x30, 0));
            HuSprPosSet(lbl_1_bss_570, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_570, (s16) (playerIndex + 4), 4);
        } else {
            character = lbl_80265728[playerIndex].charNo;
            lbl_1_bss_3F4[playerIndex] = HuSprAnimRead(fn_80008DD0(lbl_1_data_1C[character],
                0x10000000, 2));
            HuSprGrpMemberSet(lbl_1_bss_570, (s16) (playerIndex + 4),
                HuSprCreate(lbl_1_bss_3F4[playerIndex], 0x30, 0));
            HuSprPosSet(lbl_1_bss_570, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_570, (s16) (playerIndex + 4), 4);
        }
        playerIndex += 1;
    }
    lbl_1_bss_3F0 = HuSprAnimRead(fn_80008DD0(0xA2001C, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 0, HuSprCreate(lbl_1_bss_3F0, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 0, 0.0f, 0.0f);
    lbl_1_bss_3EC = HuSprAnimRead(fn_80008DD0(0xA2001D, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 1, HuSprCreate(lbl_1_bss_3EC, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 1, 8.0f, 12.0f);
    lbl_1_bss_3E8 = HuSprAnimRead(fn_80008DD0(0xA2001E, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 2, HuSprCreate(lbl_1_bss_3E8, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 2, 13.0f, 9.0f);
    lbl_1_bss_3E4 = HuSprAnimRead(fn_80008DD0(0xA20020, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 3, HuSprCreate(lbl_1_bss_3E4, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 3, -23.0f, -25.0f);
    lbl_1_bss_434 = HuSprAnimRead(fn_80008DD0(0xA20024, 0x10000000, 2));
    lbl_1_bss_40E = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_40E, 0, HuSprCreate(lbl_1_bss_434, 0x32, 0));
    HuSprPosSet(lbl_1_bss_40E, 0, 288.0f, 68.0f);
    HuSprAttrSet(lbl_1_bss_40E, 0, 4);
    lbl_1_bss_430 = HuSprAnimRead(fn_80008DD0(0xA20025, 0x10000000, 2));
    lbl_1_bss_40C = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_40C, 0, HuSprCreate(lbl_1_bss_430, 0x32, 0));
    HuSprPosSet(lbl_1_bss_40C, 0, 288.0f, 68.0f);
    HuSprBankSet(lbl_1_bss_40C, 0, 1);
    lbl_1_bss_42C = HuSprAnimRead(fn_80008DD0(0xA20026, 0x10000000, 2));
    lbl_1_bss_40A = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_40A, 0, HuSprCreate(lbl_1_bss_42C, 0x32, 0));
    HuSprPosSet(lbl_1_bss_40A, 0, 288.0f, 68.0f);
    HuSprTPLvlSet(lbl_1_bss_40A, 0, 0.0f);
    lbl_1_bss_404[0] = HuSprGrpCreate(2);
    lbl_1_bss_424[0] = HuSprAnimRead(fn_80008DD0(0xA20056, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_404[0], 0, HuSprCreate(lbl_1_bss_424[0], 0x32, 0));
    HuSprPosSet(lbl_1_bss_404[0], 0, -16.0f, 0.0f);
    HuSprBankSet(lbl_1_bss_404[0], 0, 0);
    lbl_1_bss_424[1] = HuSprAnimRead(fn_80008DD0(0xA20056, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_404[0], 1, HuSprCreate(lbl_1_bss_424[1], 0x32, 0));
    HuSprPosSet(lbl_1_bss_404[0], 1, 16.0f, 0.0f);
    HuSprBankSet(lbl_1_bss_404[0], 1, 0);
    HuSprGrpAttrSet(lbl_1_bss_404[0], 4);
    HuSprGrpPosSet(lbl_1_bss_404[0], 271.0f, 68.0f);
    quizMode = fn_1_10498();
    if (quizMode != 8) {
        lbl_1_bss_478 = HuSprAnimRead(fn_80008DD0(0xA20015, 0x10000000, 2));
        lbl_1_bss_422 = HuSprGrpCreate(1);
        HuSprGrpMemberSet(lbl_1_bss_422, 0, HuSprCreate(lbl_1_bss_478, 0x32, 0));
        HuSprPosSet(lbl_1_bss_422, 0, 401.0f, 62.0f);
        HuSprBankSet(lbl_1_bss_422, 0, (s16) (quizMode + 1));
        if (quizMode == 3) {
            lbl_1_bss_470[0] = HuSprAnimRead(fn_80008DD0(0xA20036, 0x10000000, 2));
            lbl_1_bss_470[1] = HuSprAnimRead(fn_80008DD0(0xA20037, 0x10000000, 2));
            lbl_1_bss_450 = HuSprAnimRead(fn_80008DD0(0xA20038, 0x10000000, 2));
            lbl_1_bss_44C = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_bss_44C, 0, HuSprCreate(lbl_1_bss_450, 0x31, 0));
            HuSprPosSet(lbl_1_bss_44C, 0, 401.0f, 62.0f);
            HuSprAttrSet(lbl_1_bss_44C, 0, 4);
        } else if (quizMode == 2) {
            lbl_1_bss_46C = HuSprAnimRead(fn_80008DD0(0xA20039, 0x10000000, 2));
            lbl_1_data_20A = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20A, 0, HuSprCreate(lbl_1_bss_46C, 0x33, 0));
            HuSprPosSet(lbl_1_data_20A, 0, 401.0f, 62.0f);
            lbl_1_bss_2E4[0] = fn_8000DAC0(fn_1_C768, 0x64U, 0x1800U, 0, fn_8000DD9C());
            lbl_1_bss_2E4[0]->property = (void *) lbl_1_data_20A;
        } else if (quizMode == 4) {
            lbl_1_bss_468 = HuSprAnimRead(fn_80008DD0(0xA2003A, 0x10000000, 2));
            lbl_1_data_20C = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20C, 0, HuSprCreate(lbl_1_bss_468, 0x33, 0));
            HuSprPosSet(lbl_1_data_20C, 0, 401.0f, 62.0f);
            lbl_1_bss_2E4[0] = fn_8000DAC0(fn_1_C768, 0x64U, 0x1800U, 0, fn_8000DD9C());
            lbl_1_bss_2E4[0]->property = (void *) lbl_1_data_20C;
            lbl_1_bss_464 = HuSprAnimRead(fn_80008DD0(0xA2003B, 0x10000000, 2));
            lbl_1_data_20E = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20E, 0, HuSprCreate(lbl_1_bss_464, 0x33, 0));
            HuSprPosSet(lbl_1_data_20E, 0, 288.0f, 68.0f);
            lbl_1_bss_460 = HuSprAnimRead(fn_80008DD0(0xA2003C, 0x10000000, 2));
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                lbl_1_data_210[playerIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(lbl_1_data_210[playerIndex], 0, HuSprCreate(lbl_1_bss_460,
                    0x34, 0));
                HuSprPosSet(lbl_1_data_210[playerIndex], 0,
                    lbl_1_data_21C[playerIndex].positionX, lbl_1_data_21C[playerIndex].positionY);
                HuSprAttrSet(lbl_1_data_210[playerIndex], 0, 1);
                lbl_1_bss_2E4[playerIndex + 2] = fn_8000DAC0(fn_1_C3FC, 0x64U, 0x1800U, 0,
                    fn_8000DD9C());
                lbl_1_bss_2E4[playerIndex + 2]->property = (void *) lbl_1_data_210[playerIndex];
            }
            lbl_1_bss_45C = HuSprAnimRead(fn_80008DD0(0xA2003D, 0x10000000, 2));
            lbl_1_data_218 = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_218, 0, HuSprCreate(lbl_1_bss_45C, 0x35, 0));
            HuSprPosSet(lbl_1_data_218, 0, 248.0f, 52.0f);
            fn_8000DAC0(fn_1_C768, 0x64U, 0x1800U, 0, fn_8000DD9C());
            lbl_1_bss_2E4[0] = fn_8000DAC0(fn_1_C1E4, 0x64U, 0x1800U, 0, fn_8000DD9C());
            lbl_1_bss_2E4[0]->property = (void *) lbl_1_data_218;
            lbl_1_bss_458 = HuSprAnimRead(fn_80008DD0(0xA2003E, 0x10000000, 2));
            lbl_1_data_21A = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_21A, 0, HuSprCreate(lbl_1_bss_458, 0x36, 0));
            HuSprPosSet(lbl_1_data_21A, 0, 328.0f, 84.0f);
            lbl_1_bss_2E4[1] = fn_8000DAC0(fn_1_C1E4, 0x64U, 0x1800U, 0, fn_8000DD9C());
            lbl_1_bss_2E4[1]->property = (void *) lbl_1_data_21A;
        } else if (quizMode == 5 || quizMode == 7) {
            lbl_1_bss_468 = HuSprAnimRead(fn_80008DD0(0xA2003A, 0x10000000, 2));
            lbl_1_data_20C = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_data_20C, 0, HuSprCreate(lbl_1_bss_468, 0x33, 0));
            HuSprPosSet(lbl_1_data_20C, 0, 401.0f, 62.0f);
            lbl_1_bss_2E4[0] = fn_8000DAC0(fn_1_C768, 0x64U, 0x1800U, 0, fn_8000DD9C());
            lbl_1_bss_2E4[0]->property = (void *) lbl_1_data_20C;
        } else if (quizMode == 0 || quizMode == 1) {
            lbl_1_bss_454 = HuSprAnimRead(fn_80008DD0(0xA20035, 0x10000000, 2));
        } else if (quizMode == 6) {
            lbl_1_bss_454 = HuSprAnimRead(fn_80008DD0(0xA2003F, 0x10000000, 2));
        }
    }
    lbl_1_bss_448 = HuSprAnimRead(fn_80008DD0(0xA20021, 0x10000000, 2));
    lbl_1_bss_418 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_418, 0, HuSprCreate(lbl_1_bss_448, 0x32, 0));
    HuSprPosSet(lbl_1_bss_418, 0, 506.0f, 65.0f);
    HuSprBankSet(lbl_1_bss_418, 0, 0);
    lbl_1_bss_444 = HuSprAnimRead(fn_80008DD0(0xA2002D, 0x10000000, 2));
    lbl_1_bss_416 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_416, 0, HuSprCreate(lbl_1_bss_444, 0x32, 0));
    HuSprPosSet(lbl_1_bss_416, 0, 504.0f, 99.0f);
    HuSprBankSet(lbl_1_bss_416, 0, 0);
    lbl_1_bss_440 = HuSprAnimRead(fn_80008DD0(0xA2002E, 0x10000000, 2));
    lbl_1_bss_414 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_414, 0, HuSprCreate(lbl_1_bss_440, 0x32, 0));
    HuSprPosSet(lbl_1_bss_414, 0, 504.0f, 99.0f);
    HuSprBankSet(lbl_1_bss_414, 0, 0);
    HuSprGrpAttrSet(lbl_1_bss_414, 4);
    lbl_1_bss_43C = HuSprAnimRead(fn_80008DD0(0xA2002C, 0x10000000, 2));
    lbl_1_bss_412 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_412, 0, HuSprCreate(lbl_1_bss_43C, 0x32, 0));
    HuSprPosSet(lbl_1_bss_412, 0, 102.0f, 67.0f);
    HuSprBankSet(lbl_1_bss_412, 0, (s16) lbl_1_bss_47C->questionColumn);
    lbl_1_bss_33C = HuSprAnimRead(fn_80008DD0(0xA2002A, 0x10000000, 2));
    lbl_1_bss_334 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_334, 0, HuSprCreate(lbl_1_bss_33C, 0x32, 0));
    HuSprPosSet(lbl_1_bss_334, 0, 292.0f, 208.0f);
    HuSprAttrSet(lbl_1_bss_334, 0, 4);
    lbl_1_bss_338 = HuSprAnimRead(fn_80008DD0(0xA2002B, 0x10000000, 2));
    lbl_1_bss_332 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_332, 0, HuSprCreate(lbl_1_bss_338, 0x32, 0));
    HuSprPosSet(lbl_1_bss_332, 0, 292.0f, 208.0f);
    HuSprAttrSet(lbl_1_bss_332, 0, 4);
    lbl_1_bss_438 = HuSprAnimRead(fn_80008DD0(0xA20067, 0x10000000, 2));
    lbl_1_bss_410 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_410, 0, HuSprCreate(lbl_1_bss_438, 0x5DC, 0));
    lbl_1_bss_488 = 285.0f;
    lbl_1_bss_484 = -152.0f;
    HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484);
    HuSprAttrSet(lbl_1_bss_410, 0, 4);
    lbl_1_bss_480 = 0;
    fn_1_13F8_inlined();
    fn_1_D4F4();
    fn_1_15A0C(0);
}

static void fn_1_2029C_inlined_23748_inlined(void);

static void fn_1_20148_inlined_23748_inlined(void);

static inline void fn_1_20148_inlined_23748_inlined(void)
{
    s32 playerIndex;
    u8 disabledPlayerMask;

    disabledPlayerMask = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_102B0(playerIndex) == 0) {
            disabledPlayerMask |= 1 << playerIndex;
        }
    }
    HuWinDisablePlayerReset(lbl_1_bss_32A, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_328, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_326, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_324, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_34E, 0xFF);
    HuWinDisablePlayerReset(lbl_1_bss_34C, 0xFF);
    HuWinDisablePlayerSet(lbl_1_bss_32A, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_328, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_326, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_324, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_34E, disabledPlayerMask);
    HuWinDisablePlayerSet(lbl_1_bss_34C, disabledPlayerMask);
}

static inline void fn_1_2029C_inlined_23748_inlined(void)
{
    GXColor borderColor = lbl_1_rodata_10;
    GXColor backgroundColor = lbl_1_rodata_14;

    lbl_1_bss_32A = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 6, 4);
    lbl_1_bss_328 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 7, 3);
    lbl_1_bss_326 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 0x17, 5);
    lbl_1_bss_324 = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 0xD, 0);

    fn_1_20148_inlined_23748_inlined();

    HuWinAttrSet(lbl_1_bss_32A, 0x4000);
    HuWinAttrSet(lbl_1_bss_328, 0x4000);
    HuWinAttrSet(lbl_1_bss_326, 0x4000);
    HuWinAttrSet(lbl_1_bss_324, 0x4000);

    lbl_1_bss_322 = HuWinCreate(16.0f, 312.0f, 0x220, 0x20, 0);
    HuWinBGTPLvlSet(lbl_1_bss_322, 0.0f);
    HuWinMesSpeedSet(lbl_1_bss_322, 0);
    HuWinAttrSet(lbl_1_bss_322, 0x800);
    HuWinDispOff(lbl_1_bss_32A);
    HuWinDispOff(lbl_1_bss_328);
    HuWinDispOff(lbl_1_bss_326);
    HuWinDispOff(lbl_1_bss_324);
    HuWinPriSet(lbl_1_bss_32A, 0x64);
    HuWinPriSet(lbl_1_bss_328, 0x64);
    HuWinPriSet(lbl_1_bss_326, 0x64);
    HuWinPriSet(lbl_1_bss_324, 0x64);
    HuWinPriSet(lbl_1_bss_322, 0x64);
    HuWinCallbackSet(lbl_1_bss_32A, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_328, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_326, fn_1_19FB0);
    HuWinCallbackSet(lbl_1_bss_324, fn_1_19FB0);
    lbl_1_bss_31E = -1;
}

extern "C++" void fn_1_BA0C(void) {
    s32 characterNo;
    s32 playerNo;

    lbl_1_bss_47C = (MicquizModeWork *)MgModeWork;
    lbl_1_bss_570 = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_570, 493.0f, 386.0f);
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (fn_1_102B0(playerNo) == 0) {
            lbl_1_bss_3F4[playerNo] = HuSprAnimRead(fn_80008DD0(0x9B0000, 0x10000000, 2));
            HuSprGrpMemberSet(lbl_1_bss_570, (s16) (playerNo + 4), HuSprCreate(lbl_1_bss_3F4[playerNo], 0x30, 0));
            HuSprPosSet(lbl_1_bss_570, (s16) (playerNo + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_570, (s16) (playerNo + 4), 4);
        } else {
        characterNo = lbl_80265728[playerNo].charNo;
        lbl_1_bss_3F4[playerNo] = HuSprAnimRead(fn_80008DD0(lbl_1_data_1C[characterNo], 0x10000000, 2));
        HuSprGrpMemberSet(lbl_1_bss_570, (s16) (playerNo + 4), HuSprCreate(lbl_1_bss_3F4[playerNo], 0x30, 0));
        HuSprPosSet(lbl_1_bss_570, (s16) (playerNo + 4), -23.0f, -25.0f);
            break;
        }
    }
    lbl_1_bss_3F0 = HuSprAnimRead(fn_80008DD0(0xA2001C, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 0, HuSprCreate(lbl_1_bss_3F0, 0x31, 1));
    HuSprPosSet(lbl_1_bss_570, 0, 0.0f, 0.0f);
    lbl_1_bss_3EC = HuSprAnimRead(fn_80008DD0(0xA2001D, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 1, HuSprCreate(lbl_1_bss_3EC, 0x31, 1));
    HuSprPosSet(lbl_1_bss_570, 1, 8.0f, 12.0f);
    lbl_1_bss_3E8 = HuSprAnimRead(fn_80008DD0(0xA2001E, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 2, HuSprCreate(lbl_1_bss_3E8, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 2, 13.0f, 9.0f);
    fn_1_2029C_inlined_23748_inlined();
    fn_1_D4F4();
}

extern "C++" void fn_1_C1E4(void) {
    HUPROCESS *process;
    s16 sprite;
    f32 angle;
    f32 randomAngle;
    f32 scale;
    f32 transparency;

    process = fn_8000DD9C();
    sprite = (s16)process->property;
    angle = (f32)(u32)frandmod(0x168);
    randomAngle = (f32)(u32)frandmod(0x168);
    HuSprTPLvlSet(sprite, 0,
        (f32)(0.5 +
        (0.5 * sin((M_PI * (f64)angle) /
        180.0))));
    HuSprScaleSet(sprite, 0,
        (f32)(1.0 +
        (0.5 * sin((M_PI * (f64)angle) /
        180.0))),
        1.0f);
    for (;;) {
        HuPrcVSleep();
        scale = (f32)(1.0 +
            (0.5 * sin((M_PI * (f64)angle) /
            180.0)));
        transparency = (f32)(0.5 +
            (0.5 * sin((M_PI * (f64)angle) /
            180.0)));
        HuSprTPLvlSet(sprite, 0,
            transparency);
        HuSprScaleSet(sprite, 0, scale, 1.0f);
        angle += 0.75f;
    }
}

extern "C++" void fn_1_C3FC(void) {
    f32 scale;
    f32 angle;
    s32 initialDelay;
    s32 hiddenDelay;
    HUPROCESS *currentProcess;
    s16 groupId;

    currentProcess = fn_8000DD9C();
    groupId = (s16) currentProcess->property;
    HuSprAttrSet(groupId, 0, 4);
    HuSprScaleSet(groupId, 0, 0.0f, 0.0f);
    initialDelay = frandmod(0x3C) + 0x3C;
    hiddenDelay = frandmod(0x78) + 0x3C;
    HuPrcSleep(initialDelay);
    angle = 0.0f;
    while (1) {
        HuSprAttrReset(groupId, 0, 4);
        HuSprAttrReset(groupId, 0, 1);
        do {
            HuPrcVSleep();
            scale = (f32) sin((M_PI * (f64) angle) / 180.0);
            HuSprScaleSet(groupId, 0, scale, scale);
            angle += 4.285714f;
        } while (!(angle >= 90.0f));
        angle = 0.0f;
        HuSprAttrSet(groupId, 0, 4);
        HuSprAttrSet(groupId, 0, 1);
        HuPrcSleep(hiddenDelay);
    }
}

extern "C++" void fn_1_C558(void) {
    f32 randomAngle;
    f32 randomAmplitude;
    s32 delay;
    s16 spriteGroup;
    f32 x;
    f32 y;
    f32 angle;
    s32 frame;
    f32 transparency;

    randomAngle = 2.0f * frandf();
    randomAmplitude = frandf();
    delay = frandmod(0xA);
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(lbl_1_bss_454, 0x33, 0));
    HuSprAttrSet(spriteGroup, 0, 4);
    HuSprAttrSet(spriteGroup, 0, 1);
    HuPrcSleep(delay);
    x = (f32)(401.0 +
        (40.0 * ((f64)randomAmplitude *
        sin(M_PI * (f64)randomAngle))));
    y = (f32)(62.0 +
        (40.0 * ((f64)randomAmplitude *
        cos(M_PI * (f64)randomAngle))));
    HuSprPosSet(spriteGroup, 0, x, y);
    HuSprAttrReset(spriteGroup, 0, 4);
    HuSprAttrReset(spriteGroup, 0, 1);

    angle = 0.0f;
    for (frame = 0; frame < 0x3C; frame++) {
        HuPrcVSleep();
        transparency = (f32)(1.0 -
            sin((M_PI * (f64)angle) / 180.0));
        angle += 1.5f;
        if (frame <= 0x32) {
            HuSprTPLvlSet(spriteGroup, 0, transparency);
        }
    }
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_C768(void) {
    f32 scale;
    f32 transparency;
    f32 angle;
    HUPROCESS *process;
    s16 groupId;

    /* angle is not initialized on entry. */
    process = fn_8000DD9C();
    groupId = (s16) process->property;
    while (1) {
        HuPrcVSleep();
        scale = (f32) (1.0 + (0.3f * sin((M_PI * angle) / 180.0)));
        transparency = (f32) (1.0 - sin((M_PI * angle) / 180.0));
        HuSprTPLvlSet(groupId, 0, transparency);
        HuSprScaleSet(groupId, 0, scale, scale);
        angle += 1.5f;
        if (!(angle >= 90.0f)) {
            continue;
        }
        angle = 0.0f;
    }
}

extern "C++" void fn_1_C884(void) {
    s32 character;
    s32 playerIndex;

    lbl_1_bss_4EC = fn_8000DD9C();
    lbl_1_bss_47C = (MicquizModeWork *)MgModeWork;
    lbl_1_bss_570 = HuSprGrpCreate(8);
    HuSprGrpPosSet(lbl_1_bss_570, 493.0f, 386.0f);
    playerIndex = 0;
    while (playerIndex < 4) {
        if (fn_1_102B0(playerIndex) == 0) {
            lbl_1_bss_3F4[playerIndex] = HuSprAnimRead(fn_80008DD0(0x9B0000, 0x10000000, 2));
            HuSprGrpMemberSet(lbl_1_bss_570, (s16) (playerIndex + 4),
                HuSprCreate(lbl_1_bss_3F4[playerIndex], 0x30, 0));
            HuSprPosSet(lbl_1_bss_570, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_570, (s16) (playerIndex + 4), 4);
        } else {
            character = lbl_80265728[playerIndex].charNo;
            lbl_1_bss_3F4[playerIndex] = HuSprAnimRead(fn_80008DD0(lbl_1_data_1C[character],
                0x10000000, 2));
            HuSprGrpMemberSet(lbl_1_bss_570, (s16) (playerIndex + 4),
                HuSprCreate(lbl_1_bss_3F4[playerIndex], 0x30, 0));
            HuSprPosSet(lbl_1_bss_570, (s16) (playerIndex + 4), -23.0f, -25.0f);
            HuSprAttrSet(lbl_1_bss_570, (s16) (playerIndex + 4), 4);
        }
        playerIndex += 1;
    }
    lbl_1_bss_3F0 = HuSprAnimRead(fn_80008DD0(0xA2001C, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 0, HuSprCreate(lbl_1_bss_3F0, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 0, 0.0f, 0.0f);
    lbl_1_bss_3EC = HuSprAnimRead(fn_80008DD0(0xA2001D, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 1, HuSprCreate(lbl_1_bss_3EC, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 1, 8.0f, 12.0f);
    lbl_1_bss_3E8 = HuSprAnimRead(fn_80008DD0(0xA2001E, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 2, HuSprCreate(lbl_1_bss_3E8, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 2, 13.0f, 9.0f);
    lbl_1_bss_3E4 = HuSprAnimRead(fn_80008DD0(0xA20020, 0x10000000, 2));
    HuSprGrpMemberSet(lbl_1_bss_570, 3, HuSprCreate(lbl_1_bss_3E4, 0x31, 0));
    HuSprPosSet(lbl_1_bss_570, 3, -23.0f, -23.0f);
    HuSprAttrSet(lbl_1_bss_570, 2, 4);
    HuSprBankSet(lbl_1_bss_570, 0, 0);
    HuSprBankSet(lbl_1_bss_570, 1, 0);
    HuSprGrpAttrSet(lbl_1_bss_570, 4);
}

extern "C++" void fn_1_CD18(s32 mode) {
    f32 scale;
    f32 angle;
    s32 frame;

    scale = 0.0f;
    angle = 0.0f;
    HuSprGrpAttrReset(lbl_1_bss_570, 4);
    HuSprAttrSet(lbl_1_bss_570, 2, 4);
    for (frame = 0; frame < 4; frame++) {
        HuSprAttrSet(lbl_1_bss_570, (s16)(frame + 4), 4);
    }
    HuSprAttrSet(lbl_1_bss_570, 3, 4);
    HuSprGrpScaleSet(lbl_1_bss_570, 0.0f, 0.0f);
    fn_1_19CD0(mode);
    do {
        HuPrcVSleep();
        scale = (f32)(1.2f *
                      sin((M_PI * (f64)angle) / 180.0));
        HuSprGrpScaleSet(lbl_1_bss_570, scale, scale);
        angle += 4.5f;
    } while (!(angle >= 90.0f));
    HuSprGrpScaleSet(lbl_1_bss_570, 1.2f, 1.2f);

    scale = 1.2f;
    angle = 0.0f;
    do {
        HuPrcVSleep();
        scale = (f32)(1.2f -
                      (0.4f *
                       sin((M_PI * (f64)angle) / 180.0)));
        HuSprGrpScaleSet(lbl_1_bss_570, scale, scale);
        angle += 18.0f;
    } while (!(angle >= 90.0f));
    HuSprGrpScaleSet(lbl_1_bss_570, 0.8f, 0.8f);

    scale = 1.2f;
    angle = 0.0f;
    do {
        HuPrcVSleep();
        scale = (f32)(0.8f +
                      (0.2f *
                       sin((M_PI * (f64)angle) / 180.0)));
        HuSprGrpScaleSet(lbl_1_bss_570, scale, scale);
        angle += 18.0f;
    } while (!(angle >= 90.0f));
    HuSprGrpScaleSet(lbl_1_bss_570, 1.0f, 1.0f);
}

extern "C++" void fn_1_D058(void) {
    f32 angle;
    f32 scale;

    scale = 0.0f;
    angle = 0.0f;
    HuSprGrpScaleSet(lbl_1_bss_570, 1.0f, 1.0f);
    do {
        HuPrcVSleep();
        scale = (f32) (1.0 - sin((M_PI * (f64) angle) / 180.0));
        HuSprGrpScaleSet(lbl_1_bss_570, scale, scale);
        angle += 3.0f;
    } while (!(angle >= 90.0f));
    HuSprGrpAttrSet(lbl_1_bss_570, 4);
    HuSprGrpScaleSet(lbl_1_bss_570, 0.0f, 0.0f);
}

extern s16 lbl_1_bss_378[4][3];

extern ANIMDATA *lbl_1_bss_390[4][3];

extern "C++" void fn_1_D18C(void) {
    s32 playerIndex;
    s32 digitIndex;
    s32 score;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_102B0(playerIndex) != 0) {
            for (digitIndex = 0; digitIndex < 3; digitIndex++) {
                lbl_1_bss_390[playerIndex][digitIndex] =
                    HuSprAnimRead(fn_80008DD0(0xA20056, 0x10000000, 2));
                lbl_1_bss_378[playerIndex][digitIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(lbl_1_bss_378[playerIndex][digitIndex], 0,
                                  HuSprCreate(lbl_1_bss_390[playerIndex][digitIndex], 0x5DC, 0));
                HuSprPosSet(lbl_1_bss_378[playerIndex][digitIndex], 0,
                            (f32)(0x40 + (playerIndex << 7) + (digitIndex << 5)),
                            275.0f);
                HuSprAttrSet(lbl_1_bss_378[playerIndex][digitIndex], 0, 4);
            }

            score = lbl_1_bss_47C->playerScore[playerIndex];
            hundreds = score / 100;
            tens = (score - (hundreds * 100)) / 10;
            ones = (score - (hundreds * 100)) - (tens * 10);
            HuSprBankSet(lbl_1_bss_378[playerIndex][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_378[playerIndex][1], 0, tens);
            HuSprBankSet(lbl_1_bss_378[playerIndex][2], 0, ones);
        }
    }
}

extern "C++" void fn_1_D3E4(void) {
    s32 i;
    s32 value;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (i = 0; i < 4; i++) {
        if (fn_1_102B0(i) != 0) {
            value = lbl_1_bss_47C->playerScore[i];
            hundreds = value / 100;
            tens = (value - hundreds * 100) / 10;
            ones = value - hundreds * 100 - tens * 10;
            HuSprBankSet(lbl_1_bss_378[i][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_378[i][1], 0, tens);
            HuSprBankSet(lbl_1_bss_378[i][2], 0, ones);
        }
    }
}

extern MGSCORE *lbl_1_bss_350[2];

extern s16 lbl_1_bss_358[4];

extern ANIMDATA *lbl_1_bss_360[4];

extern s16 lbl_1_bss_370;

extern ANIMDATA *lbl_1_bss_374;

extern s16 lbl_1_bss_3C0[4];

extern ANIMDATA *lbl_1_bss_3C8[4];

extern s16 lbl_1_bss_3D8;

extern ANIMDATA *lbl_1_bss_3DC;

static inline void CreatePlayerScoreDigits(void) {
    s32 playerIndex;
    s32 digitIndex;
    s32 score;
    s32 hundreds;
    s32 tens;
    s32 ones;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_102B0(playerIndex) != 0) {
            for (digitIndex = 0; digitIndex < 3; digitIndex++) {
                lbl_1_bss_390[playerIndex][digitIndex] =
                    HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 86), 0x10000000,
                                                       2));
                lbl_1_bss_378[playerIndex][digitIndex] = HuSprGrpCreate(1);
                HuSprGrpMemberSet(
                    lbl_1_bss_378[playerIndex][digitIndex], 0,
                    HuSprCreate(lbl_1_bss_390[playerIndex][digitIndex],
                                0x5DC, 0));
                HuSprPosSet(lbl_1_bss_378[playerIndex][digitIndex], 0,
                            (f32)(0x40 + (playerIndex << 7) + (digitIndex << 5)),
                            275.0f);
                HuSprAttrSet(lbl_1_bss_378[playerIndex][digitIndex], 0, 4);
            }

            score = lbl_1_bss_47C->playerScore[playerIndex];
            hundreds = score / 100;
            tens = (score - hundreds * 100) / 10;
            ones = score - hundreds * 100 - tens * 10;
            HuSprBankSet(lbl_1_bss_378[playerIndex][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_378[playerIndex][1], 0, tens);
            HuSprBankSet(lbl_1_bss_378[playerIndex][2], 0, ones);
        }
    }
}

extern "C++" void fn_1_D4F4(void) {
    s32 playerIndex;
    s32 characterIndex;
    s32 hundreds;
    s32 score;
    s32 tens;
    s32 ones;

    lbl_1_bss_14 = HuSprAnimRead(
        fn_80008DD0(DATANUM(0xa20000, 101), 0x10000000, 2));
    lbl_1_bss_3DC = HuSprAnimRead(
        fn_80008DD0(DATANUM(0xa20000, 102), 0x10000000, 2));
    lbl_1_bss_3D8 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_3D8, 0,
                      HuSprCreate(lbl_1_bss_3DC, 0x5DC, 0));
    HuSprPosSet(lbl_1_bss_3D8, 0, 288.0f, 64.0f);
    HuSprAttrSet(lbl_1_bss_3D8, 0, 4);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_3C8[playerIndex] = HuSprAnimRead(
            fn_80008DD0(DATANUM(0xa20000, 104) + playerIndex, 0x10000000,
                                 2));
        lbl_1_bss_3C0[playerIndex] = HuSprGrpCreate(1);
        HuSprGrpMemberSet(lbl_1_bss_3C0[playerIndex], 0,
                          HuSprCreate(lbl_1_bss_3C8[playerIndex], 0x5DC, 0));
        HuSprPosSet(lbl_1_bss_3C0[playerIndex], 0,
                    (f32)((playerIndex << 7) + 0x60), 248.0f);
        HuSprAttrSet(lbl_1_bss_3C0[playerIndex], 0, 4);
    }

    CreatePlayerScoreDigits();

    lbl_1_bss_350[0] = MgScoreInit(DATANUM(0xa20000, 120));
    HuPrcSetStat(lbl_1_bss_350[0]->proc, 4);
    MgScoreMaxDigitSet(lbl_1_bss_350[0], 2);
    MgScoreValueSet(lbl_1_bss_350[0], lbl_1_bss_47C->questionCount);
    MgScorePosSet(lbl_1_bss_350[0], 186.0f, 135.0f);
    MgScoreDispOff(lbl_1_bss_350[0]);
    MgScorePriSet(lbl_1_bss_350[0], 0x32);

    lbl_1_bss_374 = HuSprAnimRead(
        fn_80008DD0(DATANUM(0xa20000, 108), 0x10000000, 2));
    lbl_1_bss_370 = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_370, 0,
                      HuSprCreate(lbl_1_bss_374, 0x5DC, 0));
    HuSprPosSet(lbl_1_bss_370, 0, 288.0f, 140.0f);
    HuSprAttrSet(lbl_1_bss_370, 0, 4);

    lbl_1_bss_350[1] = MgScoreInit(DATANUM(0xa20000, 120));
    HuPrcSetStat(lbl_1_bss_350[1]->proc, 4);
    MgScoreMaxDigitSet(lbl_1_bss_350[1], 2);
    MgScoreValueSet(lbl_1_bss_350[1], lbl_1_bss_47C->playerCount * 5);
    MgScorePosSet(lbl_1_bss_350[1], 338.0f, 145.0f);
    MgScoreDispOff(lbl_1_bss_350[1]);
    MgScorePriSet(lbl_1_bss_350[1], 0x32);

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (fn_1_102B0(playerIndex) != 0) {
            characterIndex = lbl_1_bss_47C->playerCharacter[playerIndex];
            lbl_1_bss_360[playerIndex] = HuSprAnimRead(
                fn_80008DD0(characterIndex + DATANUM(0xa20000, 109),
                                     0x10000000, 2));
            lbl_1_bss_358[playerIndex] = HuSprGrpCreate(1);
            HuSprGrpMemberSet(
                lbl_1_bss_358[playerIndex], 0,
                HuSprCreate(lbl_1_bss_360[playerIndex], 0x5DC, 0));
            HuSprPosSet(lbl_1_bss_358[playerIndex], 0,
                        (f32)((playerIndex << 7) + 0x5F), 214.0f);
            HuSprAttrSet(lbl_1_bss_358[playerIndex], 0, 4);
        }
    }

    lbl_1_bss_34E = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 0x17, 5);
    lbl_1_bss_34C = HuWinExCreateFrame(16.0f, 344.0f, 0x220, 0x60, 0xD, 0);
    HuWinDispOff(lbl_1_bss_34E);
    HuWinPriSet(lbl_1_bss_34E, 0x32);
    HuWinDispOff(lbl_1_bss_34C);
    HuWinPriSet(lbl_1_bss_34C, 0x32);
    lbl_1_bss_320 = HuWinCreate(16.0f, 352.0f, 0x220, 0x80, 0);
    HuWinPosSet(lbl_1_bss_320, 16.0f, 352.0f);
    HuWinBGTPLvlSet(lbl_1_bss_320, 0.0f);
    HuWinMesSpeedSet(lbl_1_bss_320, 0);
    HuWinPriSet(lbl_1_bss_320, 0x32);
    HuWinAttrSet(lbl_1_bss_320, 0x800);
}

extern "C++" void fn_1_DDF8(void) {
    s32 i;
    s32 hundreds;
    s32 value;
    s32 tens;
    s32 ones;

    MgScoreValueSet(lbl_1_bss_350[0], lbl_1_bss_47C->questionCount);
    MgScoreValueSet(lbl_1_bss_350[1], lbl_1_bss_47C->playerCount * 5);
    for (i = 0; i < 4; i++) {
        if (fn_1_102B0(i) != 0) {
            value = lbl_1_bss_47C->playerScore[i];
            hundreds = value / 100;
            tens = (value - hundreds * 100) / 10;
            ones = value - hundreds * 100 - tens * 10;
            HuSprBankSet(lbl_1_bss_378[i][0], 0, hundreds);
            HuSprBankSet(lbl_1_bss_378[i][1], 0, tens);
            HuSprBankSet(lbl_1_bss_378[i][2], 0, ones);
        }
    }
}

extern s16 lbl_1_bss_40E;

extern s16 lbl_1_bss_40C;

extern s16 lbl_1_bss_40A;

extern u8 lbl_1_bss_406[0x4];

extern HUSPR_GROUPID lbl_1_bss_404[1];

extern ANIMDATA *lbl_1_bss_3F4[4];

extern ANIMDATA *lbl_1_bss_3F0;

extern ANIMDATA *lbl_1_bss_3EC;

extern ANIMDATA *lbl_1_bss_3E8;

extern ANIMDATA *lbl_1_bss_3E4;

extern s32 lbl_1_bss_3E0;

extern ANIMDATA *lbl_1_bss_3DC;

extern s16 lbl_1_bss_3D8;

extern ANIMDATA *lbl_1_bss_3C8[4];

extern s16 lbl_1_bss_3C0[4];

extern ANIMDATA *lbl_1_bss_390[4][3];

extern s16 lbl_1_bss_378[4][3];

extern ANIMDATA *lbl_1_bss_374;

extern s16 lbl_1_bss_370;

extern ANIMDATA *lbl_1_bss_360[4];

extern s16 lbl_1_bss_358[4];

extern MGSCORE *lbl_1_bss_350[2];

extern "C++" void fn_1_DF4C(void) {
    s32 i;
    s32 j;

    HuSprAttrReset(lbl_1_bss_3D8, 0, 4);
    for (i = 0; i < 4; i++) {
        HuSprAttrReset(lbl_1_bss_3C0[i], 0, 4);
    }
    for (i = 0; i < 4; i++) {
        if (fn_1_102B0(i) != 0) {
            for (j = 0; j < 3; j++) {
                HuSprAttrReset(lbl_1_bss_378[i][j], 0, 4);
            }
        }
    }
    HuSprAttrReset(lbl_1_bss_370, 0, 4);
    for (i = 0; i < 4; i++) {
        if (fn_1_102B0(i) != 0) {
            HuSprAttrReset(lbl_1_bss_358[i], 0, 4);
        }
    }
    MgScoreDispOn(lbl_1_bss_350[0]);
    MgScoreDispOn(lbl_1_bss_350[1]);
}

extern "C++" void fn_1_E0A0(void) {
    s32 i;
    s32 j;

    HuSprAttrSet(lbl_1_bss_3D8, 0, 4);
    for (i = 0; i < 4; i++) {
        HuSprAttrSet(lbl_1_bss_3C0[i], 0, 4);
    }
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 3; j++) {
            HuSprAttrSet(lbl_1_bss_378[i][j], 0, 4);
        }
    }
    HuSprAttrSet(lbl_1_bss_370, 0, 4);
    for (i = 0; i < 4; i++) {
        if (fn_1_102B0(i) != 0) {
            HuSprAttrSet(lbl_1_bss_358[i], 0, 4);
        }
    }
    MgScoreDispOff(lbl_1_bss_350[0]);
    MgScoreDispOff(lbl_1_bss_350[1]);
}

extern s16 lbl_1_data_208;

extern "C++" void fn_1_E1E4(void) {
    s32 i;

    HuSprGrpDrawNoSet(lbl_1_bss_570, 0);
    HuSprDrawNoSet(lbl_1_bss_40E, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_40C, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_40A, 0, 0);
    HuSprGrpDrawNoSet(lbl_1_bss_404[0], 0);
    if (fn_1_10498() != 8) {
        HuSprDrawNoSet(lbl_1_bss_422, 0, 0);
    }
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 0);
    }
    HuWinDrawNoSet(lbl_1_bss_32A, 0);
    HuWinDrawNoSet(lbl_1_bss_328, 0);
    HuWinDrawNoSet(lbl_1_bss_326, 0);
    HuWinDrawNoSet(lbl_1_bss_324, 0);
    HuSprDrawNoSet(lbl_1_bss_418, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_416, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_414, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_412, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_334, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_332, 0, 0);
    HuSprDrawNoSet(lbl_1_bss_410, 0, 0);
    if (lbl_1_data_20A != -1) {
        HuSprDrawNoSet(lbl_1_data_20A, 0, 0);
    }
    if (lbl_1_data_20C != -1) {
        HuSprDrawNoSet(lbl_1_data_20C, 0, 0);
    }
    if (lbl_1_data_20E != -1) {
        HuSprDrawNoSet(lbl_1_data_20E, 0, 0);
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_data_210[i] != -1) {
            HuSprDrawNoSet(lbl_1_data_210[i], 0, 0);
        }
    }
    if (lbl_1_data_218 != -1) {
        HuSprDrawNoSet(lbl_1_data_218, 0, 0);
    }
    if (lbl_1_data_21A != -1) {
        HuSprDrawNoSet(lbl_1_data_21A, 0, 0);
    }
}

extern "C++" void fn_1_E4E8(void) {
    s32 i;

    HuSprGrpDrawNoSet(lbl_1_bss_570, 0x40);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 0x40);
    }
    HuWinDrawNoSet(lbl_1_bss_32A, 0x40);
    HuWinDrawNoSet(lbl_1_bss_328, 0x40);
    HuWinDrawNoSet(lbl_1_bss_326, 0x40);
    HuWinDrawNoSet(lbl_1_bss_324, 0x40);
    HuSprDrawNoSet(lbl_1_bss_40E, 0, 0x40);
    HuSprDrawNoSet(lbl_1_bss_40C, 0, 0x40);
    HuSprDrawNoSet(lbl_1_bss_40A, 0, 0x40);
    HuSprGrpDrawNoSet(lbl_1_bss_404[0], 0x40);
    if (fn_1_10498() != 8) {
        HuSprDrawNoSet(lbl_1_bss_422, 0, 0x40);
    }
    HuSprDrawNoSet(lbl_1_bss_418, 0, 0x40);
    HuSprDrawNoSet(lbl_1_bss_416, 0, 0x40);
    HuSprDrawNoSet(lbl_1_bss_414, 0, 0x40);
    HuSprDrawNoSet(lbl_1_bss_412, 0, 0x40);
    HuSprDrawNoSet(lbl_1_bss_334, 0, 0x40);
    HuSprDrawNoSet(lbl_1_bss_332, 0, 0x40);
    HuSprDrawNoSet(lbl_1_bss_410, 0, 0x40);
    if (lbl_1_data_20A != -1) {
        HuSprDrawNoSet(lbl_1_data_20A, 0, 0x40);
    }
    if (lbl_1_data_20C != -1) {
        HuSprDrawNoSet(lbl_1_data_20C, 0, 0x40);
    }
    if (lbl_1_data_20E != -1) {
        HuSprDrawNoSet(lbl_1_data_20E, 0, 0x40);
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_data_210[i] != -1) {
            HuSprDrawNoSet(lbl_1_data_210[i], 0, 0x40);
        }
    }
    if (lbl_1_data_218 != -1) {
        HuSprDrawNoSet(lbl_1_data_218, 0, 0x40);
    }
    if (lbl_1_data_21A != -1) {
        HuSprDrawNoSet(lbl_1_data_21A, 0, 0x40);
    }
}

extern MicquizModeWork *lbl_1_bss_47C;

extern ANIMDATA *lbl_1_bss_478;

extern ANIMDATA *lbl_1_bss_470[2];

extern ANIMDATA *lbl_1_bss_46C;

extern ANIMDATA *lbl_1_bss_468;

extern ANIMDATA *lbl_1_bss_464;

extern ANIMDATA *lbl_1_bss_460;

extern ANIMDATA *lbl_1_bss_45C;

extern ANIMDATA *lbl_1_bss_458;

extern ANIMDATA *lbl_1_bss_454;

extern ANIMDATA *lbl_1_bss_450;

extern s16 lbl_1_bss_44C;

extern ANIMDATA *lbl_1_bss_448;

extern ANIMDATA *lbl_1_bss_444;

extern ANIMDATA *lbl_1_bss_440;

extern ANIMDATA *lbl_1_bss_43C;

extern ANIMDATA *lbl_1_bss_438;

extern ANIMDATA *lbl_1_bss_434;

extern ANIMDATA *lbl_1_bss_430;

extern ANIMDATA *lbl_1_bss_42C;

extern ANIMDATA *lbl_1_bss_424[2];

extern s16 lbl_1_bss_422;

extern u8 lbl_1_bss_41A[0x8];

extern s16 lbl_1_bss_418;

extern s16 lbl_1_bss_416;

extern s16 lbl_1_bss_414;

extern s16 lbl_1_bss_412;

extern s16 lbl_1_bss_410;

extern s16 lbl_1_bss_490;

extern HUPROCESS *lbl_1_bss_C;

void fn_1_154F0(void);

extern "C++" void fn_1_E7EC(void) {
    HuSprGrpDrawNoSet(lbl_1_bss_490, 0);
    HuSprGrpDrawNoSet(lbl_1_bss_570, 0);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 0);
    }
    HuWinDrawNoSet(lbl_1_bss_32A, 0);
    HuWinDrawNoSet(lbl_1_bss_328, 0);
    HuWinDrawNoSet(lbl_1_bss_326, 0);
    HuWinDrawNoSet(lbl_1_bss_324, 0);
}

extern "C++" void fn_1_E8A8(void) {
    HuSprGrpDrawNoSet(lbl_1_bss_490, 0x40);
    HuSprGrpDrawNoSet(lbl_1_bss_570, 0x40);
    if (lbl_1_data_208 != -1) {
        HuWinDrawNoSet(lbl_1_data_208, 0x40);
    }
    HuWinDrawNoSet(lbl_1_bss_32A, 0x40);
    HuWinDrawNoSet(lbl_1_bss_328, 0x40);
    HuWinDrawNoSet(lbl_1_bss_326, 0x40);
    HuWinDrawNoSet(lbl_1_bss_324, 0x40);
}

extern "C++" void fn_1_E964(s32 action) {
    if (action == 0) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = 0;
        }
        HuSprAttrSet(lbl_1_bss_418, 0, 4);
        HuSprAttrSet(lbl_1_bss_416, 0, 4);
    } else if (action == 1) {
        HuSprAttrReset(lbl_1_bss_418, 0, 4);
        HuSprAttrReset(lbl_1_bss_416, 0, 4);
        if ((u32)lbl_1_bss_C == 0) {
            lbl_1_bss_C = fn_8000DAC0(fn_1_154F0, 0x64, 0x1800, 0, lbl_1_bss_4EC);
        }
    } else if (action == 3) {
        fn_1_EC84();
        HuPrcSleep(0x14);
        HuSprAttrSet(lbl_1_bss_418, 0, 4);
        HuSprScaleSet(lbl_1_bss_418, 0, 0.0f, 0.0f);
        HuSprGrpAttrSet(lbl_1_bss_416, 4);
        HuSprGrpAttrReset(lbl_1_bss_414, 4);
    } else if (action == 2) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = 0;
        }
        fn_1_ED38();
        HuPrcSleep(0x1E);
        HuSprAttrSet(lbl_1_bss_416, 0, 4);
        HuSprAttrSet(lbl_1_bss_414, 0, 4);
        HuSprScaleSet(lbl_1_bss_416, 0, 0.0f, 0.0f);
    } else if (action == 4) {
        if ((u32)lbl_1_bss_C != 0) {
            HuPrcKill(lbl_1_bss_C);
            lbl_1_bss_C = 0;
        }
        HuAudFXPlay(MSM_SE_BRD00_PRIZE_SPARKLE);
        fn_1_EC84();
        fn_1_ECE0();
        HuPrcSleep(0x1E);
        HuSprAttrSet(lbl_1_bss_418, 0, 4);
        HuSprScaleSet(lbl_1_bss_418, 0, 0.0f, 0.0f);
        HuSprAttrSet(lbl_1_bss_416, 0, 4);
        HuSprScaleSet(lbl_1_bss_416, 0, 0.0f, 0.0f);
    }
}

extern s16 lbl_1_bss_490;

extern "C++" void fn_1_EC84(void) {
    s32 i;

    i = 0;
    while (i < 0x14) {
        fn_8000DAC0(fn_1_ED90, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
        i += 1;
    }
}

extern "C++" void fn_1_ECE0(void) {
    s32 i;

    i = 0;
    while (i < 0xA) {
        fn_8000DAC0(fn_1_F09C, 0x64U, 0x1800U, 0, fn_8000DD9C());
        i += 1;
    }
}

extern "C++" void fn_1_ED38(void) {
    s32 i;

    i = 0;
    while (i < 0xA) {
        fn_8000DAC0(fn_1_F334, 0x64U, 0x1800U, 0, fn_8000DD9C());
        i += 1;
    }
}

extern "C++" void fn_1_ED90(void) {
    s16 sleep;
    ANIMDATA *animation;
    HUSPR_GROUPID group;
    f32 transparency;
    f32 step;
    f32 rotation;
    f32 scale;
    f32 position[2];

    animation = HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 34), 0x10000000, 2));
    group = HuSprGrpCreate(1);
    HuSprGrpMemberSet(group, 0, HuSprCreate(animation, 0x32, 0));
    step = 3.0f;
    position[0] = (f32)(sin((u8)frand() * 1.4117647f * M_PI / 180.0) * 25.0 + 506.0);
    position[1] = (f32)(cos((u8)frand() * 1.4117647f * M_PI / 180.0) * 25.0 + 65.0);
    HuSprGrpPosSet(group, position[0], position[1]);
    HuSprScaleSet(group, 0, 0.0f, 0.0f);
    rotation = 0.0f;
    scale = 0.0f;
    transparency = 1.0f;
    sleep = frandmod(0x14);
    HuPrcSleep(sleep);
    for (;;) {
        if (lbl_1_bss_47C->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(group, 0x40);
        } else {
            HuSprGrpDrawNoSet(group, 0);
        }
        HuPrcVSleep();
        scale = (f32)(sin(M_PI * rotation / 180.0) * 1.5);
        transparency = (f32)(1.0 - sin(M_PI * rotation / 180.0));
        HuSprZRotSet(group, 0, scale);
        HuSprScaleSet(group, 0, scale, scale);
        HuSprTPLvlSet(group, 0, transparency);
        rotation += step;
        if (rotation >= 90.0f) {
            break;
        }
    }
    HuSprGrpKill(group);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_F09C(void) {
    s16 sleep;
    ANIMDATA *animation;
    HUSPR_GROUPID group;
    f32 transparency;
    f32 step;
    f32 rotation;
    f32 scale;
    f32 position[2];

    animation = HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 35), 0x10000000, 2));
    group = HuSprGrpCreate(1);
    HuSprGrpMemberSet(group, 0, HuSprCreate(animation, 0x32, 0));
    step = 3.0f;
    position[0] = (f32)(sin((u8)frand() * 1.4117647f * M_PI / 180.0) * 45.0 + 504.0);
    position[1] = 99.0f;
    HuSprGrpPosSet(group, position[0], position[1]);
    HuSprScaleSet(group, 0, 0.0f, 0.0f);
    rotation = 0.0f;
    scale = 0.0f;
    transparency = 1.0f;
    sleep = frandmod(0x14);
    HuPrcSleep(sleep);
    for (;;) {
        if (lbl_1_bss_47C->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(group, 0x40);
        } else {
            HuSprGrpDrawNoSet(group, 0);
        }
        HuPrcVSleep();
        scale = (f32)(sin(M_PI * rotation / 180.0) * 1.5);
        transparency = (f32)(1.0 - sin(M_PI * rotation / 180.0));
        HuSprZRotSet(group, 0, scale);
        HuSprScaleSet(group, 0, scale, scale);
        HuSprTPLvlSet(group, 0, transparency);
        rotation += step;
        if (rotation >= 90.0f) {
            break;
        }
    }
    HuSprGrpKill(group);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_F334(void) {
    s16 sleep;
    ANIMDATA *animation;
    HUSPR_GROUPID group;
    f32 transparency;
    f32 step;
    f32 rotation;
    f32 scale;
    f32 position[2];

    animation = HuSprAnimRead(fn_80008DD0(DATANUM(0xa20000, 35), 0x10000000, 2));
    group = HuSprGrpCreate(1);
    HuSprGrpMemberSet(group, 0, HuSprCreate(animation, 0x32, 0));
    step = 3.0f;
    position[0] = (f32)(sin((u8)frand() * 1.4117647f * M_PI / 180.0) * 25.0 + 504.0);
    position[1] = 99.0f;
    HuSprGrpPosSet(group, position[0], position[1]);
    HuSprScaleSet(group, 0, 0.0f, 0.0f);
    rotation = 0.0f;
    scale = 0.0f;
    transparency = 1.0f;
    sleep = frandmod(0x14);
    HuPrcSleep(sleep);
    for (;;) {
        if (lbl_1_bss_47C->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(group, 0x40);
        } else {
            HuSprGrpDrawNoSet(group, 0);
        }
        HuPrcVSleep();
        scale = (f32)(sin(M_PI * rotation / 180.0) * 1.5);
        transparency = (f32)(1.0 - sin(M_PI * rotation / 180.0));
        HuSprZRotSet(group, 0, scale);
        HuSprScaleSet(group, 0, scale, scale);
        HuSprTPLvlSet(group, 0, transparency);
        rotation += step;
        if (rotation >= 90.0f) {
            break;
        }
    }
    HuSprGrpKill(group);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

void fn_1_F608(void);

extern "C++" void fn_1_F5CC(void) {
    for (;;) {
        fn_8000DAC0(fn_1_F608, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
        HuPrcSleep(0x14);
    }
}

extern "C++" void fn_1_F608(void) {
    ANIMDATA *animation;
    s16 spriteGroup;
    f32 scalePhase;
    f32 transparencyPhase;
    f32 scale;
    f32 transparency;

    animation = HuSprAnimRead(fn_80008DD0(0xA2001F, 0x10000000, 2));
    spriteGroup = HuSprGrpCreate(1);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(animation, 0x2F, 0));
    HuSprPosSet(spriteGroup, 0, 516.0f, 362.0f);
    HuSprTPLvlSet(spriteGroup, 0, 0.0f);

    scalePhase = 90.0f;
    transparencyPhase = 0.0f;
    do {
        if (lbl_1_bss_47C->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(spriteGroup, 0x40);
        } else {
            HuSprGrpDrawNoSet(spriteGroup, 0);
        }
        HuPrcVSleep();
        scale = (f32)sin((M_PI * (f64)scalePhase) / 180.0);
        transparency = (f32)sin((M_PI * (f64)transparencyPhase) / 180.0);
        HuSprScaleSet(spriteGroup, 0, scale, scale);
        HuSprTPLvlSet(spriteGroup, 0, transparency);
        scalePhase += 2.0f;
        transparencyPhase += 2.0f;
        if (transparency >= 1.0f) {
            transparency = 1.0f;
        }
    } while (!(scalePhase >= 180.0f));
    HuSprGrpKill(spriteGroup);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

void fn_1_F890(void);

extern s16 lbl_1_bss_31C;

extern "C++" void fn_1_F7F4(s32 accepted) {

    if (accepted)
    {
        HuAudFXPlay(MSM_SE_CMN_SELECTION_ACCEPT);
        lbl_1_bss_31C = lbl_1_bss_334;
        fn_1_16988(3);
    }
    else
    {
        HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
        lbl_1_bss_31C = lbl_1_bss_332;
    }
    fn_8000DAC0(fn_1_F890, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
}

extern "C++" void fn_1_F890(void) {
    HuSprAttrReset(lbl_1_bss_31C, 0, 4);
    HuPrcSleep(0x1E);
    HuSprAttrSet(lbl_1_bss_31C, 0, 4);
    HuPrcSleep(0x1E);
    HuSprAttrReset(lbl_1_bss_31C, 0, 4);
    HuPrcSleep(0x1E);
    HuSprAttrSet(lbl_1_bss_31C, 0, 4);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_F920(void) {
    s32 frame = 0;
    f32 angle = 0.0f;

    for (; ; frame++) {
        f32 scale;

        HuPrcVSleep();
        scale = (f32)(1.0 + (0.3f *
                 sin((M_PI * (f64)angle) / 180.0)));
        HuSprScaleSet(lbl_1_bss_570, 1, scale, scale);
        angle += 5.0f;
        if (angle >= 180.0f) {
            angle = 0.0f;
        }
    }
}

extern "C++" void fn_1_F9F4(s32 flag, s32 mode) {
    s32 index;

    if (mode == 0 || mode == 1) {
        if (flag != 0) {
            for (index = 0; index < 15; index++) {
                fn_8000DAC0(fn_1_C558, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
            }
            fn_1_FF84();
            return;
        }
        fn_1_100DC();
        return;
    }
    if (mode == 3) {
        if (flag != 0) {
            fn_1_100DC();
            return;
        }
        fn_1_FB04();
        return;
    }
    if (mode == 6) {
        if (flag != 0) {
            for (index = 0; index < 15; index++) {
                fn_8000DAC0(fn_1_C558, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
            }
            fn_1_FF84();
            return;
        }
        fn_1_100DC();
    }
}

extern void fn_1_FD14(void);

extern "C++" void fn_1_FB04(void) {
    f32 angle;
    f32 transparency;
    s32 childCount;

    angle = 0.0f;
    HuAudFXPlay(MSM_SE_MICQUIZ_EVENT_FRAGMENT);
    for (childCount = 0; childCount < 0xF; childCount++) {
        fn_8000DAC0(fn_1_FD14, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
    }
    HuPrcSleep(0x2D);
    HuSprTPLvlSet(lbl_1_bss_44C, 0, 0.0f);
    HuSprAttrReset(lbl_1_bss_44C, 0, 4);
    do {
        if (lbl_1_bss_47C->pauseDisplayActive != 0) {
            HuSprGrpDrawNoSet(lbl_1_bss_44C, 0x40);
        } else {
            HuSprGrpDrawNoSet(lbl_1_bss_44C, 0);
        }
        HuPrcVSleep();
        transparency = (f32)sin((M_PI * (f64)angle) /
            180.0);
        HuSprTPLvlSet(lbl_1_bss_44C, 0, transparency);
        angle += 18.0f;
    } while (!(angle >= 90.0f));
    fn_1_19BD4(0);
    WipeCreate(2, 6, 5);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    HuSprAttrSet(lbl_1_bss_422, 0, 4);
    HuSprAttrSet(lbl_1_bss_44C, 0, 4);
    WipeCreate(1, 6, 5);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_19BD4(1);
}

extern "C++" void fn_1_FD14(void) {
    f32 stepX;
    f32 stepY;
    f32 centerX;
    f32 centerY;
    f32 angle;
    f32 radius;
    f32 positionX;
    f32 positionY;
    s16 groupId;
    s32 delay;
    s32 frame;
    angle = 2.0f * frandf();
    radius = frandf();
    delay = frandmod(0xA);
    groupId = HuSprGrpCreate(1);
    HuSprGrpMemberSet(groupId, 0, HuSprCreate(lbl_1_bss_470[0], 0x31, 0));
    HuSprAttrSet(groupId, 0, 4);
    HuSprAttrSet(groupId, 0, 1);
    HuPrcSleep(delay);
    centerX = 401.0f;
    centerY = 62.0f;
    positionX = (f32)((f64) centerX + (35.0 * ((f64) radius * sin(M_PI * (f64) angle))));
    positionY = (f32)((f64) centerY + (35.0 * ((f64) radius * cos(M_PI * (f64) angle))));
    stepX = (centerX - positionX) / 45.0f;
    stepY = (centerY - positionY) / 45.0f;
    HuSprAttrReset(groupId, 0, 4);
    HuSprAttrReset(groupId, 0, 1);
    frame = 0;
    while (frame < 0x2D)
    {
        if (lbl_1_bss_47C->pauseDisplayActive != 0)
        {
            HuSprGrpDrawNoSet(groupId, 0x40);
        }
        else
        {
            HuSprGrpDrawNoSet(groupId, 0);
        }
        HuPrcVSleep();
        positionX += stepX;
        positionY += stepY;
        HuSprPosSet(groupId, 0, positionX, positionY);
        if (frame == 0xA)
        {
            HuSprGrpKill(groupId);
            groupId = HuSprGrpCreate(1);
            HuSprGrpMemberSet(groupId, 0, HuSprCreate(lbl_1_bss_470[1], 0x31, 0));
            HuSprPosSet(groupId, 0, positionX, positionY);
        }
        frame += 1;
    }
    HuSprGrpKill(groupId);
    HuPrcEnd();
    while (1) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_FF84(void) {
    f32 transparency;
    f32 scale;
    f32 angle;

    scale = 1.0f;
    transparency = 1.0f;
    angle = 0.0f;
    do {
        HuPrcVSleep();
        HuSprScaleSet(lbl_1_bss_422, 0, scale, scale);
        HuSprTPLvlSet(lbl_1_bss_422, 0, transparency);
        scale = (f32) (1.0 + sin((M_PI * (f64) angle) / 180.0));
        transparency = (f32) (1.0 - sin((M_PI * (f64) angle) / 180.0));
        angle += 1.5f;
    } while (!(angle >= 90.0f));
    HuSprAttrSet(lbl_1_bss_422, 0, 4);
}

extern "C++" void fn_1_100DC(void) {
    f32 transparency;
    f32 scale;
    f32 angle;

    scale = 1.0f;
    transparency = 1.0f;
    angle = 0.0f;
    do {
        HuPrcVSleep();
        HuSprScaleSet(lbl_1_bss_422, 0, scale, scale);
        HuSprTPLvlSet(lbl_1_bss_422, 0, transparency);
        scale = (f32) (1.0 - sin((M_PI * (f64) angle) / 180.0));
        transparency = (f32) (1.0 - sin((M_PI * (f64) angle) / 180.0));
        angle += 3.0f;
    } while (!(angle >= 90.0f));
    HuSprAttrSet(lbl_1_bss_422, 0, 4);
}

extern "C++" void fn_1_10234(void)
{
}

extern "C++" void fn_1_10238(void)
{
}

extern "C++" u8 fn_1_1023C(void) {
    return lbl_1_bss_47C->questionLevel;
}

extern "C++" u8 fn_1_10250(void) {
    return lbl_1_bss_47C->questionColumn;
}

extern "C++" void fn_1_10264(s8 player, u16 points, s32 result) {
    lbl_1_bss_47C->answeringPlayer = player;
    lbl_1_bss_47C->roundPoints = points;
    lbl_1_bss_47C->roundResult = result;
}

extern "C++" u8 fn_1_1029C(void) {
    return lbl_1_bss_47C->currentPlayer;
}

extern "C++" s32 fn_1_102B0(s32 player) {
    return lbl_1_bss_47C->playerActive[player];
}

extern s16 lbl_1_bss_18A;

extern "C++" f32 fn_1_102C8(void) {
    u8 nextIndex;
    u8 previousIndex;
    u8 currentIndex;
    s32 selectedIndex;

    if (fn_1_10498() == 2) {
        nextIndex = lbl_1_bss_47C->questionLevel;
        selectedIndex = nextIndex + 1;
    } else if (fn_1_10498() == 5) {
        previousIndex = lbl_1_bss_47C->questionLevel;
        selectedIndex = previousIndex - 1;
    } else {
        currentIndex = lbl_1_bss_47C->questionLevel;
        selectedIndex = currentIndex;
    }
    return ((f32)lbl_1_bss_4C4 - ((f32)lbl_1_bss_4BC / (f32)lbl_1_bss_4C0)) /
           (f32)lbl_1_data_48[selectedIndex];
}

extern "C++" void fn_1_1043C(void) {
    HuPrcSleep(0x78);
}

extern "C++" void fn_1_10460(void) {
    HuPrcSleep(0x3C);
}

extern "C++" s32 fn_1_10484(void) {
    return lbl_1_bss_47C->presenterLayout;
}

extern "C++" s32 fn_1_10498(void) {
    return lbl_1_bss_47C->questionEvent;
}

extern "C++" void fn_1_104AC(void) {
    lbl_1_bss_47C->presentationBusy = 1;
}

extern "C++" void fn_1_104C4(void) {
    lbl_1_bss_47C->presentationBusy = 0;
}

extern u32 lbl_1_data_A0[];

extern "C++" void fn_1_104DC(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38(0);
        fn_1_8FCC(0, lbl_1_data_A0[lbl_1_bss_47C->playerCharacter[lbl_1_bss_47C->currentPlayer]], 0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 13), 1, 0);
    } else {
        fn_1_8E38(3);
        fn_1_8FCC(3, lbl_1_data_A0[lbl_1_bss_47C->playerCharacter[lbl_1_bss_47C->currentPlayer]], 0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 76), 1, 3);
    }
    fn_1_8F58();
}

extern "C++" void fn_1_10788(u32 message) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38(1);
        fn_1_8FCC(1, message, 0);
        fn_1_8D14(0x1C0010, 1, 1);
    } else {
        fn_1_8E38(3);
        fn_1_8FCC(3, message, 0);
        fn_1_8D14(0x1C004F, 1, 3);
    }
    fn_1_8F58();
}

static inline s16 fn_1_E00_inlined_8BC8_inlined(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0: window = lbl_1_bss_D2[0]; break;
    case 1: window = lbl_1_bss_328; break;
    case 2: window = lbl_1_bss_CE; break;
    case 3: window = lbl_1_bss_324; break;
    }
    return window;
}

static inline void fn_1_1154_inlined_8BC8_inlined(u32 messageId, s16 messageSpeed, u8 windowNo)
{
    s16 window = fn_1_E00_inlined_8BC8_inlined(windowNo);

    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

static inline void fn_1_1228_inlined_8BC8_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_1278_inlined_8BC8_inlined(s32 windowNo)
{
    s16 window = fn_1_E00_inlined_8BC8_inlined(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_1228_inlined_8BC8_inlined();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

static inline void fn_1_1398_inlined_8BC8_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

static inline void fn_1_140C_inlined_8BC8_inlined(s32 windowNo, u32 messageId, s16 insertNo)
{
    HuWinInsertMesSet(fn_1_E00_inlined_8BC8_inlined(windowNo), messageId, insertNo);
}

extern "C++" void fn_1_109C8(u32 message) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_1278_inlined_8BC8_inlined(1);
        fn_1_140C_inlined_8BC8_inlined(1, message, 0);
        fn_1_1154_inlined_8BC8_inlined(0x1C001B, 1, 1);
    } else {
        fn_1_1278_inlined_8BC8_inlined(3);
        fn_1_140C_inlined_8BC8_inlined(3, message, 0);
        fn_1_1154_inlined_8BC8_inlined(0x1C0053, 1, 3);
    }
    fn_1_1398_inlined_8BC8_inlined();
}

extern "C++" void fn_1_10C08(void) {
    OSReport("SSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSSScoreMode:%d\n", lbl_1_bss_4C8);
    if (lbl_1_bss_4C8 != 2) {
        if (lbl_1_bss_47C->presenterLayout == 0) {
            fn_1_1278_inlined_8BC8_inlined(1);
            fn_1_1154_inlined_8BC8_inlined(0x1C0030, 1, 1);
        } else {
            fn_1_1278_inlined_8BC8_inlined(3);
            fn_1_1154_inlined_8BC8_inlined(0x1C0066, 1, 3);
        }
        fn_1_1398_inlined_8BC8_inlined();
    }
}

char lbl_1_data_2E0[0x15] = "/mic/ctx/micquiz_num";
char lbl_1_data_2F5[0x16] = "/mic/ctx/micquiz_num2";
char lbl_1_data_30B[0x17] = "/mic/ctx/micquiz_chara";
char lbl_1_data_322[0x15] = "/mic/ctx/micquiz_npc";
char lbl_1_data_337[0x19] = "/mic/ctx/micquiz_kurabe1";
char lbl_1_data_350[0x18] = "/mic/ctx/micquiz_fruit";

extern "C++" void fn_1_10E34(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38(1);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 17), 1, 1);
    } else {
        fn_1_8E38(3);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 80), 1, 3);
    }
    fn_1_8F58();
}

extern "C++" void fn_1_11030(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38(0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 25), 1, 0);
    } else {
        fn_1_8E38(3);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 81), 1, 3);
    }
    fn_1_8F58();
}

static void fn_1_20B74_inlined_966C_inlined(s32 windowNo);

static void fn_1_20D08_inlined_966C_inlined(s32 windowNo, u32 messageId, s16 insertNo);

static void fn_1_20A50_inlined_966C_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);

static void fn_1_20C94_inlined_966C_inlined(void);

static s16 fn_1_206FC_inlined_966C_inlined(s32 windowNo);

static void fn_1_20B24_inlined_966C_inlined(void);

static inline s16 fn_1_206FC_inlined_966C_inlined(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0:
        window = lbl_1_bss_32A;
        break;
    case 1:
        window = lbl_1_bss_328;
        break;
    case 2:
        window = lbl_1_bss_32A;
        break;
    case 3:
        window = lbl_1_bss_324;
        break;
    }
    return window;
}

static inline void fn_1_20A50_inlined_966C_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex)
{
    s16 window = fn_1_206FC_inlined_966C_inlined(windowIndex);

    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

static inline void fn_1_20B24_inlined_966C_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_20B74_inlined_966C_inlined(s32 windowNo)
{
    s16 window = fn_1_206FC_inlined_966C_inlined(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_20B24_inlined_966C_inlined();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

static inline void fn_1_20C94_inlined_966C_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

static inline void fn_1_20D08_inlined_966C_inlined(s32 windowNo, u32 messageId, s16 insertNo)
{
    HuWinInsertMesSet(fn_1_206FC_inlined_966C_inlined(windowNo), messageId, insertNo);
}

extern "C++" void fn_1_1122C(s32 playerIndex)
{
    if (lbl_1_bss_4C8 != 2) {
        if (lbl_1_bss_47C->presenterLayout == 0) {
            fn_1_20B74_inlined_966C_inlined(2);
            fn_1_20D08_inlined_966C_inlined(2, lbl_1_data_A0[lbl_1_bss_47C->playerCharacter[playerIndex]], 0);
            fn_1_20A50_inlined_966C_inlined(0x1C001A, 1, 2);
        } else {
            fn_1_20B74_inlined_966C_inlined(3);
            fn_1_20D08_inlined_966C_inlined(3, lbl_1_data_A0[lbl_1_bss_47C->playerCharacter[playerIndex]], 0);
            fn_1_20A50_inlined_966C_inlined(0x1C0052, 1, 3);
        }
        fn_1_20C94_inlined_966C_inlined();
    }
}

extern s32 lbl_1_bss_524;

static inline s16 fn_1_206FC_inlined(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0:
        window = lbl_1_bss_32A;
        break;
    case 1:
        window = lbl_1_bss_328;
        break;
    case 2:
        window = lbl_1_bss_382;
        break;
    case 3:
        window = lbl_1_bss_324;
        break;
    }
    return window;
}

static inline void fn_1_20A50_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex)
{
    s16 window = fn_1_206FC_inlined(windowIndex);

    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

static inline void fn_1_20B24_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_20B74_inlined(s32 windowNo)
{
    s16 window = fn_1_206FC_inlined(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_20B24_inlined();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

static inline void fn_1_20C94_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

extern "C++" void fn_1_114D0(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_20B74_inlined(1);
        fn_1_20A50_inlined(0x1C001C, 1, 1);
    } else {
        fn_1_20B74_inlined(3);
        fn_1_20A50_inlined(0x1C0054, 1, 3);
    }
    fn_1_20C94_inlined();
}

static void fn_1_8E38_inlined_inlined_inlined_inlined(s32 windowNo);

static void fn_1_8FCC_inlined_inlined_inlined_inlined(s32 windowNo, u32 messageId, s16 insertMessageNo);

static void fn_1_8D14_inlined_inlined_inlined_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);

static void fn_1_8F58_inlined_inlined_inlined_inlined(void);

static void fn_1_8DE8_inlined_inlined_inlined_inlined(void);

static s16 fn_1_89C0_inlined_inlined_inlined_inlined(s32 windowNo);

static inline s16 fn_1_89C0_inlined_inlined_inlined_inlined(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0:
        window = lbl_1_bss_32A;
        break;
    case 1:
        window = lbl_1_bss_328;
        break;
    case 2:
        window = lbl_1_bss_32A;
        break;
    case 3:
        window = lbl_1_bss_324;
        break;
    }
    return window;
}

static inline void fn_1_8D14_inlined_inlined_inlined_inlined(u32 messageId, s16 messageSpeed,
                                                     u8 windowIndex)
{
    s16 window = fn_1_89C0_inlined_inlined_inlined_inlined(windowIndex);

    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

static inline void fn_1_8DE8_inlined_inlined_inlined_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_8E38_inlined_inlined_inlined_inlined(s32 windowNo)
{
    s16 window = fn_1_89C0_inlined_inlined_inlined_inlined(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_8DE8_inlined_inlined_inlined_inlined();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

static inline void fn_1_8F58_inlined_inlined_inlined_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

static inline void fn_1_8FCC_inlined_inlined_inlined_inlined(s32 windowNo, u32 messageId,
                                                     s16 insertMessageNo)
{
    HuWinInsertMesSet(fn_1_89C0_inlined_inlined_inlined_inlined(windowNo), messageId,
                      insertMessageNo);
}

extern "C++" void fn_1_116CC(u32 message) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38_inlined_inlined_inlined_inlined(2);
        fn_1_8FCC_inlined_inlined_inlined_inlined(2, message, 0);
        fn_1_8D14_inlined_inlined_inlined_inlined(0x1C001D, 1, 2);
    } else {
        fn_1_8E38_inlined_inlined_inlined_inlined(3);
        fn_1_8FCC_inlined_inlined_inlined_inlined(3, message, 0);
        fn_1_8D14_inlined_inlined_inlined_inlined(0x1C0055, 1, 3);
    }
    fn_1_8F58_inlined_inlined_inlined_inlined();
    fn_1_8DE8_inlined_inlined_inlined_inlined();
}

extern "C++" void fn_1_11940(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38(2);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 40), 1, 2);
    } else {
        fn_1_8E38(3);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 96), 1, 3);
    }
    fn_1_8F58();
}

extern "C++" void fn_1_11B3C(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_20B74_inlined(1);
        fn_1_20A50_inlined(0x1C0031, 1, 1);
    } else {
        fn_1_20B74_inlined(3);
        fn_1_20A50_inlined(0x1C0067, 1, 3);
    }
    fn_1_20C94_inlined();
}

extern "C++" void fn_1_11D38(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_20B74_inlined(1);
        fn_1_20A50_inlined(0x1C0032, 1, 1);
    } else {
        fn_1_20B74_inlined(3);
        fn_1_20A50_inlined(0x1C0068, 1, 3);
    }
    fn_1_20C94_inlined();
}

extern "C++" void fn_1_11F34(void) {
    if (lbl_1_bss_4C8 != 2) {
        if (lbl_1_bss_47C->presenterLayout == 0) {
            fn_1_20B74_inlined(0);
            fn_1_20A50_inlined(0x1C0037, 1, 0);
        } else {
            fn_1_20B74_inlined(3);
            fn_1_20A50_inlined(0x1C0072, 1, 3);
        }
        fn_1_20C94_inlined();
    }
}

extern "C++" void fn_1_12144(void) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_20B74_inlined(0);
        fn_1_20A50_inlined(0x1E0002, 1, 0);
    } else {
        fn_1_20B74_inlined(3);
        fn_1_20A50_inlined(0x1E002A, 1, 3);
    }
    fn_1_20C94_inlined();
}

extern s16 lbl_1_bss_8C;

extern s16 lbl_1_bss_8E;

static void fn_1_1CD0_inlined(s32 mode);

static void fn_1_1BAC_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);

static void fn_1_1DF0_inlined(void);

static void insertWindowMessage_inlined(s32 windowNo, u32 messageId, s16 insertNo);

static s16 fn_1_1858_inlined(s32 windowNo);

static void fn_1_1C80_inlined(void);

static inline s16 fn_1_1858_inlined(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0:
        window = lbl_1_bss_32A;
        break;
    case 1:
        window = lbl_1_bss_328;
        break;
    case 2:
        window = lbl_1_bss_8E;
        break;
    case 3:
        window = lbl_1_bss_8C;
        break;
    }
    return window;
}

static inline void fn_1_1BAC_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex)
{
    s16 window;

    window = fn_1_1858_inlined(windowIndex);
    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

static inline void fn_1_1C80_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_1CD0_inlined(s32 mode)
{
    s16 window;

    window = fn_1_1858_inlined(mode);
    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_1C80_inlined();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

static inline void fn_1_1DF0_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

static inline void insertWindowMessage_inlined(s32 windowNo, u32 messageId, s16 insertNo)
{
    HuWinInsertMesSet(fn_1_1858_inlined(windowNo), messageId, insertNo);
}

extern "C++" void fn_1_12340(s32 messageIndex) {
    fn_1_1CD0_inlined(0);
    fn_1_1BAC_inlined(0x1D0014, 1, 0);
    fn_1_1DF0_inlined();
    HuPrcSleep(0x78);
    fn_1_1CD0_inlined(1);
    insertWindowMessage_inlined(1, lbl_1_data_A0[lbl_1_bss_47C->playerCharacter[messageIndex]], 0);
    fn_1_1BAC_inlined(0x1D0004, 1, 1);
    fn_1_1DF0_inlined();
}

extern s32 lbl_1_bss_49C;

extern s16 lbl_1_bss_4B0;

extern "C++" void fn_1_1259C(char *path, s32 mode) {
    lbl_1_bss_49C = mode;
    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, 88.0f);
    lbl_1_bss_4B0 = HuMCContextCreate(path);
}

extern char lbl_1_data_2E0[0x15];

extern char lbl_1_data_2F5[0x16];

extern char lbl_1_data_30B[0x17];

extern char lbl_1_data_322[0x15];

extern char lbl_1_data_337[0x19];

extern char lbl_1_data_350[0x18];

extern "C++" void fn_1_12608(s32 argument) {
    lbl_1_bss_49C = argument;
    HuMCInit(0);
    HuMCSelWinCreate(-10000.0f, 88.0f);
    if (argument == 0) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_2E0);
    } else if (argument == 1) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_2F5);
    } else if (argument == 2) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_30B);
    } else if (argument == 3) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_30B);
    } else if (argument == 4) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_322);
    } else if (argument == 5) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_337);
    } else if (argument == 6) {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_350);
    } else {
        lbl_1_bss_4B0 = HuMCContextCreate(lbl_1_data_30B);
    }
}

extern s32 lbl_1_bss_4;

extern s32 lbl_1_bss_4CC;

extern s32 lbl_1_bss_4E8;

extern s32 lbl_1_bss_4DC;

extern s32 lbl_1_bss_4E0;

extern s32 lbl_1_bss_4D0;

extern u16 lbl_802C0008[4];

void fn_1_1988C(void);

extern s32 lbl_1_bss_4B4;

extern "C++" void fn_1_12774(void) {
    lbl_1_bss_4EC = fn_8000DD9C();
}

extern "C++" void fn_1_127A0(void) {
    lbl_1_bss_4EC = fn_8000DD9C();
}

/* fn_1_127CC: see copies/ (exact copy of an analog) */

extern s32 lbl_1_bss_4D4;

extern s32 lbl_1_bss_4D8;

extern u32 lbl_1_data_1CC[2];

extern u32 lbl_1_data_1D4[8];

extern u32 lbl_1_data_5C[0x11];

extern u32 lbl_1_data_E8[0x11];

static void fn_1_935C_inlined(void);

static s32 fn_1_9134_for_B664_inlined(void);

static void SetQuizResponseMessageForB664_inlined(u32 messageType);

static void fn_1_1A40_inlined(void);

static s32 CurrentQuizAnswerForB664_inlined(void);

static void fn_1_18D4_inlined(void);

static void fn_1_90FC_inlined(s8 field13, u16 field1C, s32 field18);

static inline s32 CurrentQuizAnswerForB664_inlined(void)
{
    return lbl_1_bss_47C->questionEvent;
}

static inline void SetQuizResponseMessageForB664_inlined(u32 messageType)
{
    u32 messageId;

    if (lbl_1_data_1F4 != (s32) messageType) {
        switch (messageType) {
        case 0:
            messageId = 0x10017;
            break;
        case 1:
            messageId = 0x10018;
            break;
        case 2:
            messageId = 0x10019;
            break;
        case 3:
            messageId = 0x1001A;
            break;
        case 4:
            messageId = 0x1001B;
            break;
        case 7:
            messageId = 0x1001E;
            break;
        case 8:
            messageId = 0x1001F;
            break;
        case 9:
            messageId = 0x1001E;
            break;
        case 10:
            messageId = 0x1001F;
            break;
        }
        HuWinMesSet(lbl_1_bss_322, messageId);
        lbl_1_data_1F4 = messageType;
    }
}

static inline void fn_1_18D4_inlined(void)
{
    f32 amplitude;
    f32 angle;
    f32 sineOffset;

    amplitude = 362.0f;
    HuSprAttrReset(lbl_1_bss_410, 0, 4);
    angle = 0.0f;
    if (lbl_1_bss_480 == 0) {
        do {
            HuPrcVSleep();
            sineOffset = (f32)((f64)amplitude * sin((M_PI * (f64)angle) / 180.0));
            HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484 + sineOffset);
            angle += 3.0f;
        } while (!(angle >= 90.0f));
        lbl_1_bss_484 = 210.0f;
        HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484);
        lbl_1_bss_480 = 1;
    }
}

static inline void fn_1_1A40_inlined(void)
{
    f32 amplitude = -362.0f;
    f32 angle = 0.0f;
    f32 sineOffset;

    if (lbl_1_bss_480 != 0) {
        do {
            HuPrcVSleep();
            sineOffset = (f32)((f64)amplitude * sin((M_PI * (f64)angle) / 180.0));
            HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484 + sineOffset);
            angle += 3.0f;
        } while (!(angle >= 90.0f));
        lbl_1_bss_484 = -152.0f;
        HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484);
        HuSprAttrSet(lbl_1_bss_410, 0, 4);
        lbl_1_bss_480 = 0;
    }
}

static inline void fn_1_90FC_inlined(s8 field13, u16 field1C, s32 field18)
{
    lbl_1_bss_47C->answeringPlayer = field13;
    lbl_1_bss_47C->roundPoints = field1C;
    lbl_1_bss_47C->roundResult = field18;
}

static inline s32 fn_1_9134_for_B664_inlined(void)
{
    return lbl_1_bss_47C->currentPlayer;
}

static inline void fn_1_935C_inlined(void)
{
    lbl_1_bss_47C->presentationBusy = 0;
}

extern "C++" void fn_1_127CC(s32 quizMode) {
    f32 firstVerticalOffset;
    f32 outgoingAmplitude;
    f32 secondVerticalOffset;
    f32 incomingAmplitude;
    HUPROCESS *motionProcess;
    HUPROCESS *inputProcess;
    f32 verticalOffset;
    f32 alternateVerticalOffset;
    f64 motionAngle;
    f64 returnAngle;
    f64 messageMotionAngle;
    f64 initialMotionAngle;
    f64 closingAngle;
    f64 openingAngle;
    s32 hasSelection;
    s32 selectedAnswer;
    s32 currentAnswer;
    s32 currentPlayer;
    s32 playerIndex;
    u32 *messageEntry;
    u32 *alternateMessageEntry;
    s32 initialPlayer;

    hasSelection = 0;
    if (quizMode == 3 || quizMode == 2) {
        if (lbl_1_bss_4B8 >= 9) {
            lbl_1_bss_4B8 -= 1;
        }
    } else if (quizMode == 4) {
        if (lbl_1_bss_4B8 >= 0xF) {
            lbl_1_bss_4B8 -= 1;
        }
    }
    lbl_1_bss_4A0 = 0;
    lbl_1_bss_4CC = 0;
    lbl_1_bss_4E8 = quizMode;
    lbl_1_bss_4E4 = 0;
    lbl_1_bss_4DC = 0;
    lbl_1_bss_4D8 = 1;
    lbl_1_bss_4 = 0;
    fn_1_935C_inlined();
    lbl_1_bss_47C->responseRecord[2] = (s8)(lbl_1_bss_4E0 = fn_1_9134_for_B664_inlined());
    fn_1_15D80(lbl_1_bss_49C, lbl_1_bss_4E0);
    fn_1_19EE0();
    fn_1_15A0C(0);
    fn_1_19BD4(0);
    lbl_1_bss_4C8 = 1;
    do {
        HuPrcVSleep();
    } while (lbl_1_bss_4A0 == 0);
    fn_1_16988(0);
    lbl_1_bss_4AC = 0;
    HuPrcVSleep();
    lbl_1_bss_4AC = 1;
    HuPrcSleep(0x1E);
    lbl_1_bss_4C8 = 0;
    fn_1_E964(1);
    fn_1_19EE0();
    fn_1_15A0C(1);
    motionProcess = fn_8000DAC0(fn_1_15FB0, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
    inputProcess = fn_8000DAC0(fn_1_15610, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
    fn_1_19BD4(1);
    while (1) {
    HuPrcVSleep();
    if (HuMCSelWinCheck()) {
        SetQuizResponseMessageForB664_inlined(1);
    }
    lbl_1_bss_4CC = 1;
    lbl_1_bss_4D4 = fn_1_15090(lbl_1_bss_4E0, quizMode);
    fn_1_19BD4(1);
    lbl_1_bss_4CC = 0;
    fn_1_19EE0();
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_322);
    }
    if (lbl_1_bss_4C8 == 2) {
        while (1) {
            HuPrcVSleep();
        }
    }
    fn_1_15A0C(0);
    if (lbl_1_bss_4D4 == 0) {
        if (lbl_1_bss_4DC == 0) {
            fn_1_1A40_inlined();
            lbl_1_bss_4AC = 2;
            fn_1_E964(3);
            fn_1_12144();
            HuPrcVSleep();
            while (1) {
            HuPrcVSleep();
            if (lbl_1_bss_4E4 != 0) {
            fn_1_E964(2);
            lbl_1_bss_4DC = 1;
            lbl_1_bss_4D8 = 0;
            lbl_1_bss_4C8 = 0;
            fn_1_19EE0();
            fn_1_15A0C(1);
            fn_1_16988(2);
            break;
} else {
                fn_1_1A40_inlined();
                continue;
            }
            }
            fn_1_1122C(lbl_1_bss_4E0);
            fn_1_15A0C(1);
            } else {
            fn_1_11D38();
            HuPrcSleep(0x78);
    fn_1_1122C(lbl_1_bss_4E0);
            fn_1_15A0C(1);
        }
        continue;
    } else if (lbl_1_bss_4D4 == 1) {
        lbl_1_bss_4C8 = 1;
        lbl_1_bss_4D8 = 0;
        if (lbl_1_bss_4E4 == 0) {
    fn_1_E964(4);
        }
        HuAudFXPlay(MSM_SE_CMN_SELECTION_ACCEPT);
        lbl_1_bss_31C = lbl_1_bss_334;
        fn_1_16988(3);
        fn_8000DAC0(fn_1_F890, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
        lbl_1_bss_4D0 = 1;
        fn_1_F9F4(1, CurrentQuizAnswerForB664_inlined());
        lbl_1_bss_47C->responseRecord[0] = 0;
        break;
    } else if (lbl_1_bss_4D4 == 2) {
        if ((CurrentQuizAnswerForB664_inlined() == 3) && (hasSelection == 0)) {
    fn_1_F9F4(0, 3U);
            lbl_1_bss_4C8 = 1;
            hasSelection = 1;
            fn_1_12340(lbl_1_bss_4E0);
            HuSprAttrSet(lbl_1_bss_422, 0, 4);
            fn_1_15A0C(1);
            continue;
        }
        if (hasSelection == 0) {
            lbl_1_bss_4C8 = 1;
            lbl_1_bss_4D8 = 0;
            HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
            lbl_1_bss_31C = lbl_1_bss_332;
            fn_8000DAC0(fn_1_F890, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
            fn_1_17248(quizMode, 0);
            HuPrcSleep(0x78);
            lbl_1_bss_4D0 = 0;
            lbl_1_bss_47C->responseRecord[0] = 1;
            if (lbl_1_bss_4E4 == 0) {
    fn_1_E964(4);
            }
        } else {
            lbl_1_bss_4C8 = 1;
            lbl_1_bss_4D8 = 0;
            HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
            lbl_1_bss_31C = lbl_1_bss_332;
            fn_8000DAC0(fn_1_F890, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
            fn_1_17248(quizMode, 1);
            HuPrcSleep(0x3C);
            lbl_1_bss_4D0 = 0;
            lbl_1_bss_47C->responseRecord[0] = 1;
            if (lbl_1_bss_4E4 == 0) {
    fn_1_E964(4);
            }
            break;
        }
    } else if (lbl_1_bss_4D4 == -2) {
        fn_1_10C08();
        HuPrcSleep(0x78);
        fn_1_1122C(lbl_1_bss_4E0);
        fn_1_15A0C(1);
        continue;
    } else if (lbl_1_bss_4D4 == -3) {
        fn_1_11F34();
        HuPrcSleep(0x78);
        fn_1_1122C(lbl_1_bss_4E0);
        fn_1_15A0C(1);
        continue;
    } else if (lbl_1_bss_4D4 == -4) {
    fn_1_1122C(lbl_1_bss_4E0);
        fn_1_15A0C(1);
        continue;
    } else {
        continue;
    }
result2_common:
        lbl_1_bss_4D8 = 0;
        if (HuMCSelWinCheck() != 0) {
            SetQuizResponseMessageForB664_inlined(3);
        }
        HuPrcVSleep();
        fn_1_19DE4();
        fn_1_15A0C(2);
        fn_1_19BE8(lbl_1_bss_4E0);
        fn_1_1A40_inlined();
        fn_1_16988(2);
        lbl_1_bss_4C8 = 0;
        HuPrcKill(motionProcess);
        fn_1_11030();
        while (1) {
            HuPrcVSleep();
            if (lbl_1_bss_4C8 == 2) {
                fn_1_10E34();
                HuPrcSleep(0x3C);
                lbl_1_bss_4D0 = 0;
                lbl_1_bss_47C->responseRecord[1] = 2;
                break;
            } else {
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    if (lbl_1_bss_47C->playerActive[playerIndex] != 0 &&
                        playerIndex != fn_1_9134_for_B664_inlined() &&
                        (lbl_802C0008[playerIndex] & 0x100) != 0) {
                    HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_BUZZ_IN);
                    lbl_1_bss_47C->responseRecord[3] = (s8)(lbl_1_bss_4E0 = playerIndex);
                    lbl_1_bss_4C8 = 1;
                    fn_1_19EE0();
                    fn_1_15A0C(1);
                    fn_1_19BE8(fn_1_9134_for_B664_inlined());
    fn_1_19CD0(lbl_1_bss_4E0);
                    fn_1_18D4_inlined();
    fn_1_1122C(lbl_1_bss_4E0);
                    fn_1_15D80(lbl_1_bss_49C, lbl_1_bss_4E0);
                    goto buzzed_in;
                    }
                }
                continue;
buzzed_in:
                    if (HuMCSelWinCheck() != 0) {
                        SetQuizResponseMessageForB664_inlined(4);
                    }
retry_answer:
                    lbl_1_bss_4CC = 1;
                    lbl_1_bss_4D4 = fn_1_15090(playerIndex, quizMode);
                    fn_1_19BD4(1);
                    lbl_1_bss_4CC = 0;
                    if (HuMCSelWinCheck() != 0) {
                        HuWinHomeClear(lbl_1_bss_322);
                    }
                    fn_1_15A0C(0);
                    if (lbl_1_bss_4D4 == 0) {
                        fn_1_11D38();
                        HuPrcSleep(0x78);
                        if (HuMCSelWinCheck() != 0) {
                            SetQuizResponseMessageForB664_inlined(4);
                        }
    fn_1_1122C(lbl_1_bss_4E0);
                        fn_1_15A0C(1);
                        goto retry_answer;
                    }
                    else if (lbl_1_bss_4D4 == 1) {
                        lbl_1_bss_4C8 = 1;
                        lbl_1_bss_4D0 = 1;
                        HuAudFXPlay(MSM_SE_CMN_SELECTION_ACCEPT);
                        lbl_1_bss_31C = lbl_1_bss_334;
        fn_1_16988(3);
                        fn_8000DAC0(fn_1_F890, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
                        lbl_1_bss_47C->responseRecord[1] = 0;
                        fn_1_F9F4(1, CurrentQuizAnswerForB664_inlined());
                        break;
                    } else if (lbl_1_bss_4D4 == 2) {
                        lbl_1_bss_4C8 = 1;
                        HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
                        lbl_1_bss_31C = lbl_1_bss_332;
                        fn_8000DAC0(fn_1_F890, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
                        fn_1_17248(quizMode, 1);
                        HuPrcSleep(0x3C);
                        lbl_1_bss_4D0 = 0;
                        lbl_1_bss_47C->responseRecord[1] = 1;
                        fn_1_F9F4(1, CurrentQuizAnswerForB664_inlined());
                        break;
                    } else {
                    if (lbl_1_bss_4D4 == -2) {
                            fn_1_10C08();
                            HuPrcSleep(0x78);
                            fn_1_1122C(lbl_1_bss_4E0);
                            fn_1_15A0C(1);
                    } else if (lbl_1_bss_4D4 == -3) {
                            fn_1_11F34();
                            HuPrcSleep(0x78);
    fn_1_1122C(lbl_1_bss_4E0);
                            fn_1_15A0C(1);
                    } else if (lbl_1_bss_4D4 == -4) {
    fn_1_1122C(lbl_1_bss_4E0);
                            fn_1_15A0C(1);
                        }
                        goto retry_answer;
                    }
                }
                continue;        }
        break;
    }
        if (HuMCSelWinCheck() != 0) {
            HuWinHomeClear(lbl_1_bss_322);
        }
        fn_1_90FC_inlined((s8) lbl_1_bss_4E0, (u16) lbl_1_bss_4C4, lbl_1_bss_4D0);
    fn_1_19BD4(0);
        fn_1_15A0C(0);
        if (lbl_1_bss_4D0 != 0) {
            fn_1_11940();
        } else if (lbl_1_bss_4 == 0) {
            fn_1_1A40_inlined();
            fn_1_15A0C(0);
            lbl_1_bss_4 = 1;
        if (quizMode == 0) {
                fn_1_17A30(lbl_1_bss_4B8);
        } else if (quizMode == 1) {
                fn_1_17CA8(lbl_1_bss_4B8);
        } else if (quizMode == 2) {
                fn_1_177F0(lbl_1_data_E8[lbl_1_bss_4B8]);
        } else if (quizMode == 3) {
            fn_1_177F0(lbl_1_data_E8[lbl_1_bss_4B8]);
        } else if (quizMode == 4) {
                fn_1_177F0(lbl_1_data_5C[lbl_1_bss_4B8]);
        } else if (quizMode == 5) {
                fn_1_177F0(lbl_1_data_1CC[lbl_1_bss_4B8]);
        } else if (quizMode == 6) {
                fn_1_177F0(lbl_1_data_1D4[lbl_1_bss_4B8]);
            }
        }
        HuPrcSleep(0x64);
        fn_1_1A40_inlined();
        HuPrcSleep(0x1E);
        if (lbl_1_bss_4D0 != 0) {
            lbl_1_bss_4AC = 3;
            do {
                HuPrcVSleep();
            } while ((s32) lbl_1_bss_47C->presentationBusy != 0);
        } else {
            lbl_1_bss_4AC = 4;
            do {
                HuPrcVSleep();
            } while ((s32) lbl_1_bss_47C->presentationBusy != 0);
        }
        while (1) {
            HuPrcVSleep();
        }
}

static void SetQuizResponseMessage_inlined(u32 messageType);

static inline void SetQuizResponseMessage_inlined(u32 messageType)
{
    u32 messageId;

    if (lbl_1_data_1F4 != (s32) messageType) {
        switch (messageType) {
        case 0:
            messageId = 0x10017;
            break;
        case 1:
            messageId = 0x10018;
            break;
        case 2:
            messageId = 0x10019;
            break;
        case 3:
            messageId = 0x1001A;
            break;
        case 4:
            messageId = 0x1001B;
            break;
        case 7:
            messageId = 0x1001E;
            break;
        case 8:
            messageId = 0x1001F;
            break;
        case 9:
            messageId = 0x1001E;
            break;
        case 10:
            messageId = 0x1001F;
            break;
        }
        HuWinMesSet(lbl_1_bss_322, messageId);
        lbl_1_data_1F4 = messageType;
    }
}

extern "C++" void fn_1_13EB8(s32 mode) {
    HUPROCESS *buttonProcess;
    HUPROCESS *selectionProcess;
    f64 angleFinal;
    f64 angleFourth;
    f64 angleThird;
    f64 angleSecond;
    f64 angleFirst;
    s32 playerIndex;

    lbl_1_bss_4A0 = 0;
    lbl_1_bss_4CC = 0;
    lbl_1_bss_4E8 = mode;
    lbl_1_bss_4E4 = 0;
    lbl_1_bss_4DC = 0;
    lbl_1_bss_4D8 = 1;
    fn_1_935C_inlined();
    lbl_1_bss_47C->responseRecord[2] = (s8)(lbl_1_bss_4E0 = fn_1_9134_for_B664_inlined());
    fn_1_15D80(lbl_1_bss_49C, lbl_1_bss_4E0);
    fn_1_19EE0();
    fn_1_15A0C(0);
    fn_1_19BD4(0);
    lbl_1_bss_4EC = fn_8000DD9C();
    do {
        HuPrcVSleep();
    } while (lbl_1_bss_4A0 == 0);
    fn_1_16988(0);
    lbl_1_bss_4AC = 0;
    HuPrcVSleep();
    lbl_1_bss_4AC = 1;
    HuPrcSleep(0x1E);
    lbl_1_bss_4C8 = 0;
    fn_1_E964(1);
    fn_1_19EE0();
    fn_1_15A0C(1);
    fn_8000DAC0(fn_1_1988C, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
    buttonProcess = fn_8000DAC0(fn_1_16574, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
    selectionProcess = fn_8000DAC0(fn_1_15610, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
    fn_1_19BD4(1);
    while (1) {
    HuPrcVSleep();
    if (HuMCSelWinCheck() != 0) {
        SetQuizResponseMessage_inlined(1);
    }
    lbl_1_bss_4CC = 1;
    lbl_1_bss_4D4 = fn_1_15090(lbl_1_bss_4E0, mode);
    fn_1_19BD4(1);
    lbl_1_bss_4CC = 0;
    fn_1_19EE0();
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_322);
    }
    if (lbl_1_bss_4C8 == 2) {
        while (1) {
            HuPrcVSleep();
        }
    }
    fn_1_15A0C(0);
    if (lbl_1_bss_4D4 == 0) {
        if (lbl_1_bss_4DC == 0) {
            fn_1_19EE0();
            fn_1_15A0C(0);
            fn_1_1A40_inlined();
            lbl_1_bss_4AC = 2;
            fn_1_E964(3);
            fn_1_12144();
            HuPrcVSleep();
            while (1) {
            HuPrcVSleep();
            if (lbl_1_bss_4E4 != 0) {
                fn_1_E964(2);
                lbl_1_bss_4DC = 1;
                lbl_1_bss_4D8 = 0;
                lbl_1_bss_4C8 = 0;
                fn_1_19EE0();
                fn_1_15A0C(1);
                fn_1_16988(2);
                break;
} else {
                fn_1_1A40_inlined();
                continue;
            }
            }
            fn_1_1122C(lbl_1_bss_4E0);
            fn_1_15A0C(1);
        } else {
            fn_1_11D38();
            HuPrcSleep(0x78);
            fn_1_1122C(lbl_1_bss_4E0);
            fn_1_15A0C(1);
        }
        continue;
    }
    if (lbl_1_bss_4D4 == 1) {
        lbl_1_bss_4C8 = 1;
        lbl_1_bss_4D8 = 0;
        if (lbl_1_bss_4E4 == 0) {
    fn_1_E964(4);
        }
        lbl_1_bss_4D0 = 1;
        fn_1_F9F4(1, CurrentQuizAnswerForB664_inlined());
        lbl_1_bss_47C->responseRecord[0] = 0;
        fn_1_193B4(lbl_1_bss_4D0, lbl_1_bss_47C->responseRecord[2]);
        break;
    }
    if (lbl_1_bss_4D4 == 2) {
        fn_1_F9F4(0, CurrentQuizAnswerForB664_inlined());
        lbl_1_bss_4C8 = 1;
        lbl_1_bss_4D8 = 0;
        lbl_1_bss_4D0 = 0;
        lbl_1_bss_47C->responseRecord[0] = 1;
        if (lbl_1_bss_4E4 == 0) {
    fn_1_E964(4);
        }
    fn_1_193B4(lbl_1_bss_4D0, lbl_1_bss_47C->responseRecord[2]);
        HuPrcSleep(0x78);
        goto result_2_continue;
    }
    if (lbl_1_bss_4D4 == -2) {
        fn_1_10C08();
        HuPrcSleep(0x78);
    fn_1_1122C(lbl_1_bss_4E0);
        fn_1_15A0C(1);
    } else if (lbl_1_bss_4D4 == -3) {
        fn_1_11F34();
        HuPrcSleep(0x78);
    fn_1_1122C(lbl_1_bss_4E0);
        fn_1_15A0C(1);
    } else if (lbl_1_bss_4D4 == -4) {
    fn_1_1122C(lbl_1_bss_4E0);
        fn_1_15A0C(1);
    }
    continue;
result_2_continue:
    fn_1_19B30(0, 8.0f, 0x40000001);
    lbl_1_bss_4D8 = 0;
    if (HuMCSelWinCheck() != 0) {
        SetQuizResponseMessage_inlined(3);
    }
    HuPrcVSleep();
    fn_1_19DE4();
    fn_1_15A0C(2);
    fn_1_19BE8(lbl_1_bss_4E0);
    fn_1_1A40_inlined();
    fn_1_19B30(1, 16.0f, 0x40000001);
    fn_1_16988(2);
    lbl_1_bss_4C8 = 0;
    HuPrcKill(buttonProcess);
    fn_1_11030();
    while (1) {
    HuPrcVSleep();
    if (lbl_1_bss_4C8 == 2) {
        fn_1_10E34();
        HuPrcSleep(0x3C);
        lbl_1_bss_4D0 = 0;
        lbl_1_bss_47C->responseRecord[1] = 2;
    } else {
        for (playerIndex = 0; playerIndex < 4; playerIndex += 1) {
            if (lbl_1_bss_47C->playerActive[playerIndex] != 0 &&
                playerIndex != fn_1_9134_for_B664_inlined() &&
                (lbl_802C0008[playerIndex] & 0x100) != 0) {
            HuAudFXPlay(MSM_SE_MICQUIZ_PLAYER_BUZZ_IN);
            lbl_1_bss_47C->responseRecord[3] = (s8)(lbl_1_bss_4E0 = playerIndex);
            lbl_1_bss_4C8 = 1;
            fn_1_19EE0();
            fn_1_15A0C(1);
            fn_1_19BE8(fn_1_9134_for_B664_inlined());
            fn_1_19CD0(lbl_1_bss_4E0);
            fn_1_18D4_inlined();
            fn_1_1122C(lbl_1_bss_4E0);
            fn_1_15D80(lbl_1_bss_49C, lbl_1_bss_4E0);
            goto response_44;
            }
        }
        continue;
response_44:
    if (HuMCSelWinCheck() != 0) {
        SetQuizResponseMessage_inlined(4);
    }
    while (1) {
    lbl_1_bss_4CC = 1;
    lbl_1_bss_4D4 = fn_1_15090(playerIndex, mode);
    fn_1_19BD4(1);
    lbl_1_bss_4CC = 0;
    fn_1_19EE0();
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_322);
    }
    fn_1_15A0C(0);
    if (lbl_1_bss_4D4 == 0) {
        fn_1_11D38();
        HuPrcSleep(0x78);
        if (HuMCSelWinCheck()) {
            SetQuizResponseMessage_inlined(4);
        }
        fn_1_1122C(lbl_1_bss_4E0);
        fn_1_15A0C(1);
        continue;
    }
    if (lbl_1_bss_4D4 == 1) {
        lbl_1_bss_4C8 = 1;
        lbl_1_bss_4D0 = 1;
        lbl_1_bss_47C->responseRecord[1] = 0;
        fn_1_193B4(lbl_1_bss_4D0, lbl_1_bss_47C->responseRecord[3]);
    } else if (lbl_1_bss_4D4 == 2) {
        lbl_1_bss_4C8 = 1;
        lbl_1_bss_4D0 = 0;
        lbl_1_bss_47C->responseRecord[1] = 1;
        fn_1_193B4(lbl_1_bss_4D0, lbl_1_bss_47C->responseRecord[3]);
    } else {
        if (lbl_1_bss_4D4 == -2) {
            fn_1_10C08();
            HuPrcSleep(0x78);
            fn_1_1122C(lbl_1_bss_4E0);
            fn_1_15A0C(1);
        } else if (lbl_1_bss_4D4 == -3) {
            fn_1_11F34();
            HuPrcSleep(0x78);
            fn_1_1122C(lbl_1_bss_4E0);
            fn_1_15A0C(1);
        } else if (lbl_1_bss_4D4 == -4) {
            fn_1_1122C(lbl_1_bss_4E0);
            fn_1_15A0C(1);
        }
        continue;
    }
        break;
    }
    }
        break;
    }
        break;
    }
    if (HuMCSelWinCheck() != 0) {
        HuWinHomeClear(lbl_1_bss_322);
    }
    fn_1_90FC_inlined((s8) lbl_1_bss_4E0, (u16) lbl_1_bss_4C4, lbl_1_bss_4D0);
    fn_1_19BD4(0);
    fn_1_1A40_inlined();
    fn_1_15A0C(0);
    if (lbl_1_bss_4D0 != 0) {
        lbl_1_bss_4AC = 3;
    } else {
        HuPrcSleep(0x78);
        lbl_1_bss_4AC = 4;
    }
    while (1) {
        HuPrcVSleep();
    }
}

extern s16 lbl_1_bss_472;

extern s32 lbl_1_bss_4A4;

extern MicquizResponse *lbl_1_bss_8;

void fn_1_17F64(HUPROCESS *process);

s32 fn_1_154D8(s32 responseIndex, s32 count);

extern "C++" s32 fn_1_15090(s32 playerIndex, s32 responseMode) {
    s32 newResponseMessage;
    s32 selectedResponse;
    s32 responseData;

    if (responseMode == 0) {
        newResponseMessage = 0x49002C;
    } else if (responseMode == 1) {
        newResponseMessage = 0x490041;
    } else if (responseMode == 2) {
        newResponseMessage = 0x490005;
    } else if (responseMode == 3) {
        newResponseMessage = 0x490005;
    } else if (responseMode == 4) {
        newResponseMessage = 0x490017;
    } else if (responseMode == 5) {
        newResponseMessage = 0x490029;
    } else if (responseMode == 6) {
        newResponseMessage = 0x210018;
    }

    HuMCSelWinContextSet(lbl_1_bss_4B0, 0, (u8) playerIndex);
    HuMCSelWinChoiceGet();
    if (lbl_1_bss_4C8 == 2) {
        return -1;
    }

    lbl_1_bss_8 = (MicquizResponse *) HuMCCurResponseGet();
    if (lbl_1_bss_8->result == 0) {
        lbl_1_bss_4B4 = lbl_1_bss_8->choicesRemaining;
    } else if (lbl_1_bss_8->result == -1) {
        return -2;
    } else if (lbl_1_bss_8->result == -2) {
        return -2;
    } else if (lbl_1_bss_8->result == -4) {
        return -3;
    } else {
        return -2;
    }

    if (HuMCUnkResponseCheck() != 0) {
        fn_1_19BD4(0);
        fn_1_15A0C(1);
        HuPrcVSleep();
        fn_1_15A0C(1);
        fn_1_17F64(lbl_1_bss_4EC);
        selectedResponse = HuMCNewResponseGet(newResponseMessage);
        if (selectedResponse == -1) {
            fn_1_15A0C(0);
            fn_1_17F24();
            fn_1_19BD4(1);
            return -4;
        }
        lbl_1_bss_4B4 = selectedResponse;
        fn_1_19BD4(1);
        fn_1_15A0C(0);
    }

    if (HuMCSelWinCheck() != 0) {
        lbl_1_bss_4B4 -= 1;
    }
    if (lbl_1_bss_4B4 == 0) {
        return 0;
    }

    if (responseMode == 0) {
        if (HuMCSelWinCheck() != 0) {
            responseData = fn_1_154D8(lbl_1_bss_4B4 + lbl_1_bss_4A4, lbl_1_bss_4B8);
        } else {
            responseData = fn_1_154D8(lbl_1_bss_4B4, lbl_1_bss_4B8);
        }
    } else if (responseMode == 1) {
        if (HuMCSelWinCheck() != 0) {
            responseData = fn_1_154D8(lbl_1_bss_4B4 + lbl_1_bss_4A4, lbl_1_bss_4B8);
        } else {
            responseData = fn_1_154D8(lbl_1_bss_4B4, lbl_1_bss_4B8);
        }
    } else if (responseMode == 2) {
        responseData = fn_1_154D8(lbl_1_bss_4B4 - 1, lbl_1_bss_4B8);
    } else if (responseMode == 3) {
        responseData = fn_1_154D8(lbl_1_bss_4B4 - 1, lbl_1_bss_4B8);
    } else if (responseMode == 4) {
        responseData = fn_1_154D8(lbl_1_bss_4B4 - 1, lbl_1_bss_4B8);
    } else if (responseMode == 5) {
        responseData = fn_1_154D8(lbl_1_bss_4B4 - 1, lbl_1_bss_4B8);
    } else if (responseMode == 6) {
        responseData = fn_1_154D8(lbl_1_bss_4B4 - 1, lbl_1_bss_4B8);
    }
    return responseData;
}

extern HUPROCESS *lbl_1_bss_C;

extern MicquizResponse *lbl_1_bss_8;

extern "C++" s32 fn_1_154D8(s32 left, s32 right) {
    if (left == right) {
        return 1;
    }
    return 2;
}

extern "C++" void fn_1_154F0(void) {
    f32 baseRotation;
    f32 angle;
    f32 sineOffset;
    s16 groupId;
    s32 direction;

    groupId = lbl_1_bss_416;
    angle = 0.0f;
    direction = 0;
    baseRotation = 0.0f;
    while (1) {
        HuPrcVSleep();
        sineOffset = (f32) (10.0 * sin((M_PI * (f64) angle) / 180.0));
        HuSprZRotSet(groupId, 0, baseRotation + sineOffset);
        if (direction == 0) {
            angle += 3.0f;
            if (angle >= 90.0f) {
                angle = 90.0f;
                direction = 1;
            }
            continue;
        }
        angle -= 3.0f;
        if (!(angle <= -90.0f)) {
            continue;
        }
        angle = -90.0f;
        direction = 0;
    }
}

extern "C" {
void fn_1_15CD0(void);
}

void fn_1_15D30(void);

static inline void fn_1_8A3C_for_15610(void)
{
    f32 amplitude;
    f32 angle;
    f32 sineOffset;

    amplitude = 362.0f;
    HuSprAttrReset(lbl_1_bss_410, 0, 4);
    angle = 0.0f;
    if (lbl_1_bss_480 == 0) {
        do {
            HuPrcVSleep();
            sineOffset = (f32) ((f64) amplitude * sin((M_PI * (f64) angle) / 180.0));
            HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484 + sineOffset);
            angle += 3.0f;
        } while (!(angle >= 90.0f));
        lbl_1_bss_484 = 210.0f;
        HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484);
        lbl_1_bss_480 = 1;
    }
}

static inline void fn_1_9128_for_15610(u32 messageType)
{
    u32 messageId;

    if (lbl_1_data_1F4 != (s32) messageType) {
        switch (messageType) {
        case 0:
            messageId = MESSNUM(MESS_SYS_GUIDE, 23);
            break;
        case 1:
            messageId = MESSNUM(MESS_SYS_GUIDE, 24);
            break;
        case 2:
            messageId = MESSNUM(MESS_SYS_GUIDE, 25);
            break;
        case 3:
            messageId = MESSNUM(MESS_SYS_GUIDE, 26);
            break;
        case 4:
            messageId = MESSNUM(MESS_SYS_GUIDE, 27);
            break;
        case 7:
            messageId = MESSNUM(MESS_SYS_GUIDE, 30);
            break;
        case 8:
            messageId = MESSNUM(MESS_SYS_GUIDE, 31);
            break;
        case 9:
            messageId = MESSNUM(MESS_SYS_GUIDE, 30);
            break;
        case 10:
            messageId = MESSNUM(MESS_SYS_GUIDE, 31);
            break;
        }
        HuWinMesSet(lbl_1_bss_322, messageId);
        lbl_1_data_1F4 = messageType;
    }
}

extern "C++" void fn_1_15610(void)
{
    s32 selectionHandled = 0;

    for (;;) {
        HuPrcVSleep();
        if (lbl_1_bss_4C8 == 2) {
            fn_1_15A0C(0);
            continue;
        }
        if (lbl_1_bss_4CC == 0) {
            continue;
        }
        if (HuMCSelWinCheck() != 0) {
            if (HuMCSelModeGet() == 1) {
                if (selectionHandled == 0) {
                    fn_1_19BD4(0);
                    fn_1_15CD0();
                    fn_1_16988(1);
                    fn_1_9128_for_15610(2);
                    fn_1_15A0C(3);
                    if (lbl_1_bss_4C8 != 2) {
                        lbl_1_bss_4C8 = 1;
                    }
                    fn_1_8A3C_for_15610();
                    selectionHandled = 1;
                }
            } else {
                fn_1_15D30();
                selectionHandled = 0;
            }
            continue;
        }
        if (HuMCButtonGet() != 0) {
            fn_1_19BD4(0);
            fn_1_15CD0();
            fn_1_15A0C(3);
            fn_1_8A3C_for_15610();
            fn_1_16988(1);
            if (lbl_1_bss_4C8 != 2) {
                lbl_1_bss_4C8 = 1;
            }
        } else {
            fn_1_15D30();
        }
        if (HuMCButtonGet() != 0) {
            if (selectionHandled == 0) {
                fn_1_19BD4(0);
                fn_1_15CD0();
                selectionHandled = 1;
            }
            continue;
        }
        selectionHandled = 0;
    }
}

extern s32 lbl_1_bss_4CC;

extern s32 lbl_1_bss_4C8;

extern "C" {
void fn_1_15C80(void);
}

extern "C" {
void fn_1_15C20(void);
}

extern "C++" void fn_1_15A0C(s32 mode) {
    if (mode == 0) {
        if (lbl_1_data_1F8 != mode) {
            lbl_1_data_1F8 = mode;
            HuSprAttrReset(lbl_1_bss_570, 2, 4);
            HuSprBankSet(lbl_1_bss_570, 0, 1);
            HuSprBankSet(lbl_1_bss_570, 1, 1);
            fn_1_15C80();
            HuSprScaleSet(lbl_1_bss_570, 1, 1.0f, 1.0f);
            fn_1_15D30();
        }
    } else if (mode == 1) {
        if (lbl_1_data_1F8 != mode) {
            lbl_1_data_1F8 = mode;
            HuSprAttrSet(lbl_1_bss_570, 2, 4);
            HuSprBankSet(lbl_1_bss_570, 0, 0);
            HuSprBankSet(lbl_1_bss_570, 1, 0);
            fn_1_15C20();
        }
    } else if (mode == 2) {
        if (lbl_1_data_1F8 != mode) {
            lbl_1_data_1F8 = mode;
            HuSprAttrReset(lbl_1_bss_570, 2, 4);
            HuSprBankSet(lbl_1_bss_570, 0, 1);
            HuSprBankSet(lbl_1_bss_570, 1, 1);
        }
    } else if (mode == 3 && lbl_1_data_1F8 != mode) {
        lbl_1_data_1F8 = mode;
        fn_1_15C80();
        HuSprScaleSet(lbl_1_bss_570, 1, 1.0f, 1.0f);
    }
}

extern "C" { extern HUPROCESS *lbl_1_bss_10; }

void fn_1_F920(void);

extern "C" {
extern "C" void fn_1_15C20(void) {
    if(lbl_1_bss_10==0) {
        lbl_1_bss_10=fn_8000DAC0(fn_1_F920,100,0x2000,0,lbl_1_bss_4EC);
    }
}
}

extern "C" {
extern "C" void fn_1_15C80(void) {
    if (lbl_1_bss_10) {
        HuPrcKill(lbl_1_bss_10);
        lbl_1_bss_10 = 0;
    }
}
}

extern "C" { extern HUPROCESS *lbl_1_bss_0; }

void fn_1_F5CC(void);

extern "C" {
extern "C" void fn_1_15CD0(void) {
    if(lbl_1_bss_0==0) {
        lbl_1_bss_0=fn_8000DAC0(fn_1_F5CC,100,0x2000,0,lbl_1_bss_4EC);
    }
}
}

extern "C" { extern HUPROCESS *lbl_1_bss_10; }

extern "C" { extern ANIMDATA *lbl_1_bss_14; }

extern "C++" void fn_1_15D30(void) {
    if (lbl_1_bss_0) {
        HuPrcKill(lbl_1_bss_0);
        lbl_1_bss_0 = 0;
    }
}

extern s16 lbl_1_bss_4A8;

void fn_1_15EC0(s32 padNo, s32 direction);

extern "C++" void fn_1_15D80(s32 select, s32 padNo) {
    if (select == 0 || select == 1) {
        fn_1_15EC0(padNo, select);
    } else if (select == 2) {
        lbl_1_bss_4A8 = HuMCSelWinItemSet(0x210000, 11, (s16) padNo);
    } else if (select == 3) {
        lbl_1_bss_4A8 = HuMCSelWinItemSet(0x210000, 11, (s16) padNo);
    } else if (select == 4) {
        lbl_1_bss_4A8 = HuMCSelWinItemSet(0x21000B, 7, (s16) padNo);
    } else if (select == 5) {
        lbl_1_bss_4A8 = HuMCSelWinItemSet(0x210036, 3, (s16) padNo);
    } else if (select == 6) {
        lbl_1_bss_4A8 = HuMCSelWinItemSet(0x490076, 9, (s16) padNo);
    }
    HuMCSessionSet((s16) padNo);
}

extern s32 lbl_1_bss_4;

extern HUPROCESS *lbl_1_bss_0;

extern "C++" void fn_1_15EC0(s32 padNo, s32 direction) {
    s32 messageBase;

    if (direction == 0) {
        if (lbl_1_bss_4B8 <= 10) {
            messageBase = 0x210051;
            lbl_1_bss_4A4 = 0;
        } else {
            messageBase = 0x21005C;
            lbl_1_bss_4A4 = 10;
        }
    } else if (lbl_1_bss_4B8 <= 10) {
        messageBase = 0x210067;
        lbl_1_bss_4A4 = 0;
    } else {
        messageBase = 0x210072;
        lbl_1_bss_4A4 = 10;
    }
    lbl_1_bss_4A8 = HuMCSelWinItemSet(messageBase, 11, (s16) padNo);
}

static void fn_1_20B74_inlined_inlined(s32 windowNo);

static void fn_1_20A50_inlined_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex);

static void fn_1_20C94_inlined_inlined(void);

static s16 fn_1_206FC_inlined_inlined(s32 windowNo);

static void fn_1_20B24_inlined_inlined(void);

static inline s16 fn_1_206FC_inlined_inlined(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0:
        window = lbl_1_bss_32A;
        break;
    case 1:
        window = lbl_1_bss_328;
        break;
    case 2:
        window = lbl_1_bss_382;
        break;
    case 3:
        window = lbl_1_bss_324;
        break;
    }
    return window;
}

static inline void fn_1_20A50_inlined_inlined(u32 messageId, s16 messageSpeed, u8 windowIndex)
{
    s16 window = fn_1_206FC_inlined_inlined(windowIndex);

    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

static inline void fn_1_20B24_inlined_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_20B74_inlined_inlined(s32 windowNo)
{
    s16 window = fn_1_206FC_inlined_inlined(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_20B24_inlined_inlined();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

static inline void fn_1_20C94_inlined_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

extern "C++" void fn_1_15FB0(void)
{
    s8 playerIndex;
    u16 selectedValue;
    s32 currentState;
    f32 verticalOffset;
    f32 angle;
    f32 amplitude;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_4C8 != 2);

    fn_1_16988(1);
    fn_1_E964(0);
    HuMCSelWinContextKill();

    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_20B74_inlined_inlined(1);
        fn_1_20A50_inlined_inlined(0x1C0011, 1, 1);
    } else {
        fn_1_20B74_inlined_inlined(3);
        fn_1_20A50_inlined_inlined(0x1C0050, 1, 3);
    }
    fn_1_20C94_inlined_inlined();
    HuPrcSleep(0x78);

    lbl_1_bss_4D0 = 0;
    lbl_1_bss_47C->responseRecord[0] = 2;
    currentState = lbl_1_bss_4D0;
    selectedValue = lbl_1_bss_4C4;
    playerIndex = (s8)lbl_1_bss_4E0;
    lbl_1_bss_47C->answeringPlayer = playerIndex;
    lbl_1_bss_47C->roundPoints = (s16)selectedValue;
    lbl_1_bss_47C->roundResult = currentState;
    lbl_1_bss_4AC = 4;

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_47C->presentationBusy != 0);

    if (lbl_1_bss_4 == 0) {
        lbl_1_bss_4 = 1;
        amplitude = -362.0f;
        angle = 0.0f;
        if (lbl_1_bss_480 != 0) {
            do {
                HuPrcVSleep();
                verticalOffset = amplitude * sin((M_PI * (f64)angle) / 180.0);
                HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484 + verticalOffset);
                angle += 3.0f;
            } while (!(angle >= 90.0f));
            lbl_1_bss_484 = -152.0f;
            HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484);
            HuSprAttrSet(lbl_1_bss_410, 0, 4);
            lbl_1_bss_480 = 0;
        }

        if (lbl_1_bss_4E8 == 0) {
            fn_1_17A30(lbl_1_bss_4B8);
        } else if (lbl_1_bss_4E8 == 1) {
            fn_1_17CA8(lbl_1_bss_4B8);
        } else if (lbl_1_bss_4E8 == 2) {
            fn_1_177F0(lbl_1_data_E8[lbl_1_bss_4B8]);
        } else if (lbl_1_bss_4E8 == 3) {
            fn_1_177F0(lbl_1_data_E8[lbl_1_bss_4B8]);
        } else if (lbl_1_bss_4E8 == 4) {
            fn_1_177F0(lbl_1_data_5C[lbl_1_bss_4B8]);
        } else if (lbl_1_bss_4E8 == 5) {
            fn_1_177F0(lbl_1_data_1CC[lbl_1_bss_4B8]);
        } else if (lbl_1_bss_4E8 == 6) {
            fn_1_177F0(lbl_1_data_1D4[lbl_1_bss_4B8]);
        }
    }

    do {
        HuPrcVSleep();
    } while (lbl_1_bss_47C->presentationBusy != 0);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

static void fn_1_2E2B0_AnimateOut_inlined_inlined(void);

static inline void fn_1_2E2B0_AnimateOut_inlined_inlined(void) {
    f32 amplitude;
    f32 angle;
    f32 sineOffset;

    amplitude = -362.0f;
    angle = 0.0f;
    if (lbl_1_bss_480 != 0) {
        do {
            HuPrcVSleep();
            sineOffset = (f32) ((f64) amplitude * sin((M_PI * (f64) angle) / 180.0));
            HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484 + sineOffset);
            angle += 3.0f;
        } while (!(angle >= 90.0f));
        lbl_1_bss_484 = -152.0f;
        HuSprPosSet(lbl_1_bss_410, 0, lbl_1_bss_488, lbl_1_bss_484);
        HuSprAttrSet(lbl_1_bss_410, 0, 4);
        lbl_1_bss_480 = 0;
    }
}

extern "C++" void fn_1_16574(void) {
    s8 playerIndex;
    u16 selectedValue;
    s32 currentState;
    do
    {
        HuPrcVSleep();
    }
    while (lbl_1_bss_4C8 != 2);
    fn_1_16988(1);
    fn_1_E964(0);
    HuMCSelWinContextKill();
    fn_1_2E2B0_AnimateOut_inlined_inlined();
    fn_1_19B30(1, 16.0f, 0x40000001);
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_20B74_inlined_inlined(1);
        fn_1_20A50_inlined_inlined(0x1C0011, 1, 1);
    } else {
        fn_1_20B74_inlined_inlined(3);
        fn_1_20A50_inlined_inlined(0x1C0050, 1, 3);
    }
    fn_1_20C94_inlined_inlined();
    HuPrcSleep(0x78);
    lbl_1_bss_4D0 = 0;
    lbl_1_bss_47C->responseRecord[0] = 2;
    currentState = lbl_1_bss_4D0;
    selectedValue = lbl_1_bss_4C4;
    playerIndex = (s8) lbl_1_bss_4E0;
    lbl_1_bss_47C->answeringPlayer = playerIndex;
    lbl_1_bss_47C->roundPoints = (s16) selectedValue;
    lbl_1_bss_47C->roundResult = currentState;
    HuPrcSleep(0x78);
    lbl_1_bss_4AC = 4;
    HuPrcEnd();
    for (;;)
    {
        HuPrcVSleep();
    }
}

void fn_1_16AEC(void);

void fn_1_16D64(void);

void fn_1_16E74(void);

void fn_1_16F6C(void);

extern "C++" void fn_1_16988(s32 mode) {
    if (mode == 0) {
        if (lbl_1_data_1FC != mode) {
            lbl_1_data_1FC = mode;
            fn_8000DAC0(fn_1_16AEC, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
        }
    } else if (mode == 1) {
        if (lbl_1_data_1FC != mode) {
            lbl_1_data_1FC = mode;
            fn_8000DAC0(fn_1_16D64, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
        }
    } else if (mode == 2) {
        if (lbl_1_data_1FC != mode) {
            lbl_1_data_1FC = mode;
            fn_8000DAC0(fn_1_16E74, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
        }
    } else if (mode == 3 && lbl_1_data_1FC != mode) {
        lbl_1_data_1FC = mode;
        fn_8000DAC0(fn_1_16F6C, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
    }
}

extern "C++" void fn_1_16AEC(void) {
    f32 angle;
    f32 transparency;
    f32 scale;
    s32 frame;

    HuSprGrpAttrReset(lbl_1_bss_404[0], 4);
    scale = 0.0f;
    angle = 0.0f;
    for (frame = 0; frame < 0x14; frame++) {
        HuPrcVSleep();
        scale = (f32)(2.0 - sin((M_PI * (f64)angle) / 180.0));
        HuSprGrpScaleSet(lbl_1_bss_404[0], scale, scale);
        angle += 4.5f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprGrpScaleSet(lbl_1_bss_404[0], 1.0f, 1.0f);
    HuSprBankSet(lbl_1_bss_40C, 0, 1);
    angle = 0.0f;
    for (frame = 0; frame < 0xA; frame++) {
        HuPrcVSleep();
        transparency = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(lbl_1_bss_40A, 0, transparency);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_40E, 0, 4);
    HuSprAttrSet(lbl_1_bss_40C, 0, 4);
    angle = 90.0f;
    for (frame = 0; frame < 0xA; frame++) {
        HuPrcVSleep();
        transparency = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(lbl_1_bss_40A, 0, transparency);
        angle -= 9.0f;
        if (angle <= 45.0f) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_16D64(void) {
    f32 angle = 90.0f;
    f32 tpLevel;
    s32 count;

    for (count = 0; count < 10; count++) {
        HuPrcVSleep();
        tpLevel = (f32)(0.5 * sin((M_PI * (f64)angle) / 180.0));
        HuSprTPLvlSet(lbl_1_bss_40A, 0, tpLevel);
        angle += 9.0f;
        if (angle <= 0.0f) {
            break;
        }
    }

    HuSprAttrSet(lbl_1_bss_40E, 0, 4);
    HuSprAttrReset(lbl_1_bss_40C, 0, 4);
    HuSprBankSet(lbl_1_bss_40C, 0, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_16E74(void) {
    f32 angle = 0.0f;
    s32 frame;

    for (frame = 0; frame < 10; frame++) {
        f32 height;

        HuPrcVSleep();
        height = (f32) (0.5 * sin((M_PI * (f64) angle) / 180.0));
        HuSprTPLvlSet(lbl_1_bss_40A, 0, height);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_40E, 0, 4);
    HuSprAttrSet(lbl_1_bss_40C, 0, 4);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_16F6C(void) {
    f32 angle;
    f32 transparencyLevel;
    s32 frame;

    angle = 0.0f;
    for (frame = 0; frame < 0xA; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(lbl_1_bss_40A, 0, transparencyLevel);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprBankSet(lbl_1_bss_40C, 0, 1);
    angle = 90.0f;
    for (frame = 0; frame < 0xA; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(lbl_1_bss_40A, 0, transparencyLevel);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuSprBankSet(lbl_1_bss_40C, 0, 0);
    angle = 90.0f;
    for (frame = 0; frame < 3; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(lbl_1_bss_40A, 0, transparencyLevel);
        angle -= 30.0f;
        if (angle <= 0.0f) {
            break;
        }
    }
    HuSprAttrReset(lbl_1_bss_40E, 0, 4);
    HuSprAttrSet(lbl_1_bss_40C, 0, 4);
    HuSprSpeedSet(lbl_1_bss_40E, 0, 3.0f);
    angle = 0.0f;
    for (frame = 0; frame < 0xA; frame++) {
        HuPrcVSleep();
        transparencyLevel = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprTPLvlSet(lbl_1_bss_40A, 0, transparencyLevel);
        angle += 9.0f;
        if (angle >= 90.0f) {
            break;
        }
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

static inline s16 fn_1_E00_inlined(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0:
        window = lbl_1_bss_D2[0];
        break;
    case 1:
        window = lbl_1_bss_328;
        break;
    case 2:
        window = lbl_1_bss_CE;
        break;
    case 3:
        window = lbl_1_bss_324;
        break;
    }
    return window;
}

static inline void fn_1_1154_inlined(u32 messageId, s16 messageSpeed, u8 windowNo)
{
    s16 window = fn_1_E00_inlined(windowNo);

    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

static inline void fn_1_1228_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_1278_inlined(s32 windowNo)
{
    s16 window = fn_1_E00_inlined(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_1228_inlined();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

static inline void fn_1_1398_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

static inline void fn_1_140C_inlined(s32 windowNo, u32 messageId, s16 insertNo)
{
    HuWinInsertMesSet(fn_1_E00_inlined(windowNo), messageId, insertNo);
}

static inline void fn_1_8BC8_inlined(u32 message)
{
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_1278_inlined(1);
        fn_1_140C_inlined(1, message, 0);
        fn_1_1154_inlined(0x1C0010, 1, 1);
    } else {
        fn_1_1278_inlined(3);
        fn_1_140C_inlined(3, message, 0);
        fn_1_1154_inlined(0x1C004F, 1, 3);
    }
    fn_1_1398_inlined();
}

static inline void fn_1_8E08_inlined(u32 message)
{
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_1278_inlined(1);
        fn_1_140C_inlined(1, message, 0);
        fn_1_1154_inlined(0x1C001B, 1, 1);
    } else {
        fn_1_1278_inlined(3);
        fn_1_140C_inlined(3, message, 0);
        fn_1_1154_inlined(0x1C0053, 1, 3);
    }
    fn_1_1398_inlined();
}

extern "C++" void fn_1_17248(s32 mode, s32 wait)
{
    s32 messageIndex;
    u32 message;

    if (HuMCSelWinCheck() != 0) {
        if (mode == 0 || mode == 1) {
            messageIndex = lbl_1_bss_4B4 + lbl_1_bss_4A4 - 1;
        } else {
            messageIndex = lbl_1_bss_4B4 - 1;
        }
    } else {
        messageIndex = lbl_1_bss_4B4 - 1;
    }

    switch (mode) {
    case 0:
        message = messageIndex + 0x210022;
        break;
    case 1:
        message = messageIndex + 0x21003A;
        break;
    case 2:
        message = lbl_1_data_E8[messageIndex];
        break;
    case 3:
        message = lbl_1_data_E8[messageIndex];
        break;
    case 4:
        message = lbl_1_data_5C[messageIndex];
        break;
    case 5:
        message = lbl_1_data_1CC[messageIndex];
        break;
    case 6:
        message = lbl_1_data_1D4[messageIndex];
        break;
    }

    if (wait == 0) {
        fn_1_8BC8_inlined(message);
    } else {
        fn_1_8E08_inlined(message);
    }
}/* Unknown initialized bytes at .data 0x384..0x388. */
u8 lbl_1_data_384[4] = {0xFF,0xFF,0xFF,0xFF};
s16 lbl_1_data_388[6] = { 0x0, 0x0, 0x0, 0x0, 0x50, 0x0 };

u32 lbl_1_data_394[5][2] = {
    { MESSNUM(MESS_MICQUIZ_Q, 19), MESSNUM(MESS_MICQUIZ_Q, 51) },
    { MESSNUM(MESS_MICQUIZ_Q, 20), MESSNUM(MESS_MICQUIZ_Q, 52) },
    { MESSNUM(MESS_MICQUIZ_Q, 21), MESSNUM(MESS_MICQUIZ_Q, 53) },
    { MESSNUM(MESS_MICQUIZ_Q, 22), MESSNUM(MESS_MICQUIZ_Q, 54) },
    { MESSNUM(MESS_MICQUIZ, 29), MESSNUM(MESS_MICQUIZ, 85) }
};
u32 lbl_1_data_3BC[5] = {
    MESSNUM(MESS_MICQUIZ_Q, 23), MESSNUM(MESS_MICQUIZ_Q, 24), MESSNUM(MESS_MICQUIZ_Q, 25),
    MESSNUM(MESS_MICQUIZ_Q, 26), MESSNUM(MESS_MICQUIZ_Q, 27)
};
u32 lbl_1_data_3D0[5] = {
    MESSNUM(MESS_MICQUIZ_Q, 28), MESSNUM(MESS_MICQUIZ_Q, 29), MESSNUM(MESS_MICQUIZ_Q, 31),
    MESSNUM(MESS_MICQUIZ_Q, 32), MESSNUM(MESS_MICQUIZ_Q, 33)
};
u32 lbl_1_data_3E4[6] = {
    MESSNUM(MESS_QUIZ_CHARA, 12), MESSNUM(MESS_QUIZ_CHARA, 13), MESSNUM(MESS_QUIZ_CHARA, 14),
    MESSNUM(MESS_QUIZ_CHARA, 17), MESSNUM(MESS_QUIZ_CHARA, 15), MESSNUM(MESS_QUIZ_CHARA, 16)
};
s16 lbl_1_data_3FC[6] = { 1, 2, 3, 4, 5, 0 };
Point3d lbl_1_data_408 = { -2000.0f, 10000.0f, -400.0f };
Point3d lbl_1_data_414 = { -0.4f, -0.8f, -0.8f };
GXColor lbl_1_data_420 = { 0xC0, 0xC0, 0xC0, 0xC0 };
s32 lbl_1_data_424[6] = {
    DATANUM(0xa50000, 0), DATANUM(0xa50000, 5), DATANUM(0xa50000, 10),
    DATANUM(0xa50000, 15), DATANUM(0xa50000, 20), DATANUM(0xa50000, 25)
};
s32 lbl_1_data_43C[6] = {
    960, 981, 986,
    999, 0, 939
};
s32 lbl_1_data_454[5] = {
    DATANUM(0xa50000, 50), DATANUM(0xa50000, 51), DATANUM(0xa50000, 52),
    DATANUM(0xa50000, 54), DATANUM(0xa50000, 56)
};
s32 lbl_1_data_468[5] = {
    DATANUM(0xa50000, 57), DATANUM(0xa50000, 58), DATANUM(0xa50000, 59),
    DATANUM(0xa50000, 60), DATANUM(0xa50000, 61)
};
s32 lbl_1_data_47C[5] = {
    DATANUM(0xa50000, 30), DATANUM(0xa50000, 31), DATANUM(0xa50000, 32),
    DATANUM(0xa50000, 33), DATANUM(0xa50000, 34)
};
s32 lbl_1_data_490[5] = {
    DATANUM(0xa50000, 35), DATANUM(0xa50000, 36), DATANUM(0xa50000, 37),
    DATANUM(0xa50000, 38), DATANUM(0xa50000, 39)
};
s32 lbl_1_data_4A4[5] = {
    DATANUM(0xa50000, 40), DATANUM(0xa50000, 41), DATANUM(0xa50000, 42),
    DATANUM(0xa50000, 43), DATANUM(0xa50000, 44)
};
u32 lbl_1_data_4B8[9] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1), DATANUM(0x930000, 39),
    DATANUM(0x930000, 38), DATANUM(0x930000, 40), DATANUM(0x930000, 3),
    DATANUM(0x930000, 4), DATANUM(0x930000, 11), DATANUM(0x930000, 34)
};
s32 lbl_1_data_4DC[11] = {
    0x910000, 0x390000, 0xc30000, 0xf60000, 0xef0000,
    0x170000, 0xe90000, 0x330000, 0xd20000, 0xba0000,
    0x2d0000
};
s32 lbl_1_data_508[11] = {
    0x930000, 0x3b0000, 0xc50000, 0xf80000, 0xf10000,
    0x190000, 0xeb0000, 0x350000, 0xd40000, 0xbc0000,
    0x2f0000
};
s32 lbl_1_data_534[11] = {
    0x8e0000, 0x360000, 0xc00000, 0xf30000, 0xec0000,
    0x140000, 0xe60000, 0x300000, 0xcf0000, 0xa80000, 0x2a0000
};

f32 lbl_1_data_560[11] = {
    85.0f, 80.0f, 60.0f, 85.0f, 100.0f, 60.0f,
    50.0f, 140.0f, 105.0f, 140.0f, 140.0f
};

extern "C++" void fn_1_177F0(u32 message) {
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38(2);
        fn_1_8FCC(2, message, 0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_8E38(3);
        fn_1_8FCC(3, message, 0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_8F58();
}

extern u32 lbl_1_data_12C[0x14];

extern "C++" void fn_1_17A30(s32 choice) {
    choice == 0;
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38(2);
        fn_1_8FCC(2, lbl_1_data_12C[choice - 1], 0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_8E38(3);
        fn_1_8FCC(3, lbl_1_data_12C[choice - 1], 0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_8F58();
}

extern u32 lbl_1_data_17C[0x14];

extern "C++" void fn_1_17CA8(u32 choice) {
    choice == 0;
    if (lbl_1_bss_47C->presenterLayout == 0) {
        fn_1_8E38(2);
        fn_1_8FCC(2, lbl_1_data_17C[choice - 1], 0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 29), 1, 2);
    } else {
        fn_1_8E38(3);
        fn_1_8FCC(3, lbl_1_data_17C[choice - 1], 0);
        fn_1_8D14(MESSNUM(MESS_MICQUIZ, 85), 1, 3);
    }
    fn_1_8F58();
}

extern "C++" void fn_1_17F20(void)
{
}

void fn_1_17FEC(void);

extern "C++" void fn_1_17F24(void) {
    fn_8000DAC0(fn_1_17FEC, 0x64U, 0x1800U, 0, lbl_1_bss_4EC);
}

void fn_1_17FB4(void);

extern "C++" void fn_1_17F64(HUPROCESS* parent) {
    if(lbl_1_data_208!=-1)
        fn_8000DAC0(fn_1_17FB4,100,0x1800,0,parent);
}

extern "C++" void fn_1_17FB4(void) {
    HuWinKill(lbl_1_data_208);
    lbl_1_data_208 = -1;
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

extern "C++" void fn_1_17FEC(void) {
    HuVec2f messageSize;

    HuWinMesMaxSizeGet(1, &messageSize, 0x1C003A);
    if (lbl_1_data_208 != -1) {
        HuPrcEnd();
        for (;;) {
            HuPrcVSleep();
        }
    }
    lbl_1_data_208 = HuWinExCreateFrame(-10000.0f, 200.0f,
        messageSize.x, messageSize.y, -1, 0);
    HuWinExOpen(lbl_1_data_208);
    HuWinMesSpeedSet(lbl_1_data_208, 0);
    HuWinAttrSet(lbl_1_data_208, 0x800);
    HuWinMesSet(lbl_1_data_208, 0x1C003A);
    HuWinMesWait(lbl_1_data_208);
    HuPrcSleep(0xB4);
    if (lbl_1_data_208 != -1) {
        HuWinExClose(lbl_1_data_208);
        HuWinKill(lbl_1_data_208);
        lbl_1_data_208 = -1;
    }
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

void fn_1_183E0(s32 value);

extern "C++" void fn_1_18138(void) {
    u8 currentIndex;
    s32 firstMode;
    u8 nextIndex;
    s32 nextMode;
    u8 previousIndex;
    u8 fallbackIndex;
    s32 timingMode;

    lbl_1_bss_4C8 = 1;
    currentIndex = lbl_1_bss_47C->questionLevel;
    lbl_1_bss_4C4 = lbl_1_data_48[currentIndex];
    firstMode = lbl_1_bss_47C->questionEvent;

    if (firstMode == 2) {
        nextIndex = lbl_1_bss_47C->questionLevel;
        lbl_1_bss_4C4 = lbl_1_data_48[nextIndex + 1];
    } else {
        nextMode = lbl_1_bss_47C->questionEvent;
        if (nextMode == 5) {
            previousIndex = lbl_1_bss_47C->questionLevel;
            lbl_1_bss_4C4 = lbl_1_data_48[previousIndex - 1];
        } else {
            fallbackIndex = lbl_1_bss_47C->questionLevel;
            lbl_1_bss_4C4 = lbl_1_data_48[fallbackIndex];
        }
    }

    timingMode = lbl_1_bss_47C->questionEvent;
    if (timingMode == 4) {
        lbl_1_bss_4C0 = 900 / lbl_1_bss_4C4;
    } else {
        lbl_1_bss_4C0 = 1800 / lbl_1_bss_4C4;
    }
    lbl_1_bss_4BC = 0;
    fn_1_183E0(lbl_1_bss_4C4);
}

extern "C++" s32 fn_1_18300(void) {
    if (lbl_1_bss_4C8 == 0) {
        lbl_1_bss_4BC += 1;
        if (lbl_1_bss_4BC >= lbl_1_bss_4C0) {
            HuAudFXPlay(MSM_SE_MICQUIZ_COUNTDOWN_TICK);
            lbl_1_bss_4C4 -= 1;
            lbl_1_bss_4BC = 0;
            fn_1_183E0(lbl_1_bss_4C4);
            if (lbl_1_bss_4C4 <= 0) {
                lbl_1_bss_4C8 = 2;
            }
        }
    }
    return lbl_1_bss_4C8;
}

extern "C++" void fn_1_183E0(s32 value) {
    if (value >= 0) {
        s32 tens = value / 10;
        s32 ones = value - tens * 10;
        HuSprBankSet(lbl_1_bss_404[0], 0, tens);
        HuSprBankSet(lbl_1_bss_404[0], 1, ones);
    }
}

extern "C++" s32 fn_1_18470(s32 minimum, s32 limit) {

    return minimum + frandmod(limit - minimum);
}

extern "C++" s32 fn_1_184AC(s32 chance) {
    s32 roll;

    roll = frandmod(0x64);
    if (chance == 0) {
        return 0;
    }
    if (roll < chance) {
        return 1;
    }
    return 0;
}

void fn_1_19288(void);

static inline void fn_1_20D08_inlined(s32 windowNo, u32 messageId, s16 insertNo)
{
    HuWinInsertMesSet(fn_1_206FC_inlined(windowNo), messageId, insertNo);
}

extern "C++" void fn_1_1850C(s32 primaryMessageIndex, s32 secondaryMessageIndex) {
    extern s32 lbl_1_bss_4AC;
    extern u32 lbl_1_data_1D4[8];
    extern u32 lbl_1_data_A0[0x12];
    s32 randomChoice;
    s32 previousSelection;

    randomChoice = frandmod(3);
    previousSelection = lbl_1_bss_4AC;
    fn_1_19288();
    lbl_1_bss_4AC = 5;
    lbl_1_bss_47C->presentationBusy = 1;
    do {
        HuPrcVSleep();
    } while (lbl_1_bss_47C->presentationBusy != 0);
    lbl_1_bss_4AC = previousSelection;
    HuPrcSleep(0x3C);

    fn_1_20B74_inlined(3);
    fn_1_19B30(1, 16.0f, 0x40000001);

    if (randomChoice == 0) {
        fn_1_20D08_inlined(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_20D08_inlined(3, lbl_1_data_A0[primaryMessageIndex], 1);
        fn_1_20A50_inlined(0x1E003F, 1, 3);
        fn_1_20C94_inlined();
    } else if (randomChoice == 1) {
        fn_1_20D08_inlined(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_20A50_inlined(0x1E0040, 1, 3);
        fn_1_20C94_inlined();
        HuPrcSleep(0x78);
        fn_1_20A50_inlined(0x1E0041, 1, 3);
        fn_1_20C94_inlined();
    } else {
        fn_1_20A50_inlined(0x1E0042, 1, 3);
        fn_1_20C94_inlined();
        HuPrcSleep(0x78);
        fn_1_20D08_inlined(3, lbl_1_data_A0[primaryMessageIndex], 0);
        fn_1_20A50_inlined(0x1E0043, 1, 3);
        fn_1_20C94_inlined();
    }
}

extern "C++" void fn_1_18964(s32 primaryMessageIndex, s32 secondaryMessageIndex)
{
    s32 randomChoice = frandmod(4);

    if (randomChoice != 0) {
        s32 previousSelection = lbl_1_bss_4AC;

        fn_1_19288();
        lbl_1_bss_4AC = 5;
        lbl_1_bss_47C->presentationBusy = 1;
        do {
            HuPrcVSleep();
        } while (lbl_1_bss_47C->presentationBusy != 0);
        lbl_1_bss_4AC = previousSelection;
        HuPrcSleep(0x3C);
    }

    fn_1_1278_inlined(3);
    fn_1_19B30(1, 16.0f, 0x40000001);

    if (randomChoice == 0) {
        fn_1_140C_inlined(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_1154_inlined(0x1E0044, 1, 3);
    } else if (randomChoice == 1) {
        fn_1_140C_inlined(3, lbl_1_data_A0[primaryMessageIndex], 0);
        fn_1_1154_inlined(0x1E0045, 1, 3);
    } else if (randomChoice == 2) {
        fn_1_1154_inlined(0x1E0046, 1, 3);
        fn_1_1398_inlined();
        HuPrcSleep(0x78);
        fn_1_1154_inlined(0x1E0047, 1, 3);
        fn_1_1398_inlined();
    } else {
        fn_1_140C_inlined(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_1154_inlined(0x1E0048, 1, 3);
        fn_1_1398_inlined();
        HuPrcSleep(0x78);
        fn_1_1154_inlined(0x1E0049, 1, 3);
        fn_1_1398_inlined();
    }
    fn_1_1398_inlined();
}

extern "C++" void fn_1_18DD0(s32 primaryMessageIndex, s32 secondaryMessageIndex)
{
    s32 randomChoice = frandmod(2);

    if (randomChoice != 0) {
        s32 previousSelection = lbl_1_bss_4AC;

        fn_1_19288();
        lbl_1_bss_4AC = 5;
        lbl_1_bss_47C->presentationBusy = 1;
        do {
            HuPrcVSleep();
        } while (lbl_1_bss_47C->presentationBusy != 0);
        lbl_1_bss_4AC = previousSelection;
        HuPrcSleep(0x3C);
    }

    fn_1_1278_inlined(3);
    fn_1_19B30(1, 16.0f, 0x40000001);

    if (randomChoice == 0) {
        fn_1_140C_inlined(3, lbl_1_data_1D4[secondaryMessageIndex], 0);
        fn_1_140C_inlined(3, lbl_1_data_A0[primaryMessageIndex], 1);
        fn_1_1154_inlined(0x1E004A, 1, 3);
        fn_1_1398_inlined();
        HuPrcSleep(0x78);
        fn_1_1154_inlined(0x1E004B, 1, 3);
        fn_1_1398_inlined();
    } else {
        fn_1_1154_inlined(0x1E004C, 1, 3);
        fn_1_1398_inlined();
        HuPrcSleep(0x78);
        fn_1_1154_inlined(0x1E004D, 1, 3);
        HuPrcSleep(0x78);
    }
}

static inline s16 fn_1_E00_inlined_11558_inlined(s32 windowNo)
{
    s16 window;

    switch (windowNo) {
    case 0:
        window = lbl_1_bss_D2[0];
        break;
    case 1:
        window = lbl_1_bss_328;
        break;
    case 2:
        window = lbl_1_bss_CE;
        break;
    case 3:
        window = lbl_1_bss_324;
        break;
    }
    return window;
}

static inline void fn_1_1154_inlined_11558_inlined(u32 messageId, s16 messageSpeed, u8 windowNo)
{
    s16 window = fn_1_E00_inlined_11558_inlined(windowNo);

    HuWinAttrSet(window, 0x10);
    HuWinMesSet(window, messageId);
    HuWinMesSpeedSet(window, messageSpeed);
}

static inline void fn_1_1228_inlined_11558_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinExClose(lbl_1_bss_31E);
        lbl_1_bss_31E = -1;
    }
}

static inline void fn_1_1278_inlined_11558_inlined(s32 windowNo)
{
    s16 window = fn_1_E00_inlined_11558_inlined(windowNo);

    if (lbl_1_bss_31E != window) {
        if (lbl_1_bss_31E != -1) {
            fn_1_1228_inlined_11558_inlined();
        }
        HuWinExOpen(window);
        lbl_1_bss_31E = window;
    }
}

static inline void fn_1_1398_inlined_11558_inlined(void)
{
    if (lbl_1_bss_31E != -1) {
        HuWinMesWait(lbl_1_bss_31E);
    }
}

static inline void fn_1_140C_inlined_11558_inlined(s32 windowNo, u32 messageId, s16 insertNo)
{
    HuWinInsertMesSet(fn_1_E00_inlined_11558_inlined(windowNo), messageId, insertNo);
}

extern "C++" void fn_1_19118(s32 messageIndex) {

    fn_1_1278_inlined_11558_inlined(3);
    fn_1_19B30(1, 16.0f, 0x40000001);
    fn_1_140C_inlined_11558_inlined(3, lbl_1_data_1D4[messageIndex], 0);
    fn_1_1154_inlined_11558_inlined(0x1E004F, 1, 3);
    fn_1_1398_inlined_11558_inlined();
}

extern "C++" void fn_1_19288(void) {
    fn_1_8E38(3);
    fn_1_8D14(0x1E003E, 1, 3);
    fn_1_8F58();
}

extern "C++" s32 fn_1_193A0(void) {
    return lbl_1_bss_4B4 - 1;
}

extern "C" {
extern int fn_1_19968(int selection);
}

extern "C" {
extern int fn_1_199B8(int selection);
}

extern "C++" void fn_1_193B4(s32 useCurrentPlayer, s32 playerFieldOffset)
{
    s32 menuSelection;
    s32 messageIndex;

    fn_1_8BA8();
    HuPrcSleep(0x1E);

    if (useCurrentPlayer != 0) {
        fn_1_1850C(lbl_1_bss_47C->playerCharacter[playerFieldOffset], lbl_1_bss_4B4 - 1);
        HuAudFXPlay(MSM_SE_CMN_SELECTION_ACCEPT);
        lbl_1_bss_31C = lbl_1_bss_334;
        if (lbl_1_data_1FC != 3) {
            lbl_1_data_1FC = 3;
    fn_8000DAC0(fn_1_16F6C, 0x64, 0x1800, 0, lbl_1_bss_4EC);
        }
        fn_8000DAC0(fn_1_F890, 0x64, 0x1800, 0, lbl_1_bss_4EC);
        HuAudFXPlay(MSM_SE_GUIDE_KOOPA_TALK);
        fn_1_19B30(3, 8.0f, ((1 << 30) | (1 << 0)));
        HuPrcSleep(0x78);
        return;
    }

    if (fn_1_199B8(lbl_1_bss_4B4 - 1) != 0) {
        menuSelection = fn_1_19968(lbl_1_bss_4B4 - 1);
        if (lbl_1_bss_4F0[menuSelection].flag != 0) {
            fn_1_18DD0(lbl_1_bss_47C->playerCharacter[playerFieldOffset], lbl_1_bss_4B4 - 1);
        } else {
            fn_1_18964(lbl_1_bss_47C->playerCharacter[playerFieldOffset], lbl_1_bss_4B4 - 1);
        }
    } else {
        messageIndex = lbl_1_bss_4B4 - 1;
        fn_1_19118(messageIndex);
    }

    HuAudFXPlay(MSM_SE_CMN_SELECTION_REJECT);
    lbl_1_bss_31C = lbl_1_bss_332;
    fn_8000DAC0(fn_1_F890, 0x64, 0x1800, 0, lbl_1_bss_4EC);
    fn_1_19B30(6, 8.0f, ((1 << 30) | (1 << 0)));
    HuPrcSleep(0x78);
}

extern HU3D_MOTIONID lbl_1_bss_30C[7];

extern HU3D_MODELID lbl_1_bss_31A;

extern "C++" void fn_1_1988C(void) {
    f32 motionTime;
    s16 motionId;

    for (;;) {
        HuPrcVSleep();
        motionTime = fn_8002D4E0(lbl_1_bss_31A);
        motionId = fn_8002D3C4(lbl_1_bss_31A);
        if (motionId == lbl_1_bss_30C[6]) {
            if ((20.0f == motionTime) || (55.0f == motionTime)) {
                HuAudFXPlay(MSM_SE_GUIDE_KOOPA_STEP);
            }
            continue;
        }
        if (motionId != lbl_1_bss_30C[2]) {
            continue;
        }
        if ((10.0f != motionTime) && (60.0f != motionTime)) {
            continue;
        }
        HuAudFXPlay(MSM_SE_GUIDE_KOOPA_STEP);
    }
}

extern "C" { void fn_1_19B30(s32 index, f32 duration, u32 attr); }

extern "C" {  }

extern "C" { void fn_1_19A24(void); }

extern "C" { void fn_1_19BE8(int base); }

extern "C" { void fn_1_19DE4(void); }

extern "C" { void fn_1_19EE0(void); }

extern "C" { extern s16 lbl_1_bss_2D8; }

extern "C" {
extern "C" int fn_1_19968(int key) {
    for(int index=0;index<8;index++) {
        if(key==lbl_1_bss_4F0[index].key) return index;
    }
    return -1;
}
}

extern "C" {
extern "C" int fn_1_199B8(int key)
{
    for (int index = 0; index < 8; index++) {
        if (key == lbl_1_bss_4F0[index].key &&
            lbl_1_bss_4F0[index].state == 0) {
            return 1;
        }
    }
    return 0;
}
}

extern "C" {  }

extern "C" {
extern "C" void fn_1_19A24(void) {
    s32 index;

    lbl_1_bss_31A = fn_80025A04(fn_80008DD0(0xA40079, 0x10000000, 2));
    for (index = 0; index < 7; index++) {
        lbl_1_bss_30C[index] = fn_80030458(
            lbl_1_bss_31A,
            fn_80008DD0(lbl_1_data_0[index], 0x10000000, 2)
        );
    }
    fn_800269E8(lbl_1_bss_31A, 0x40000001U);
    fn_1_19B30(0, 0.0f, 0x40000001U);
    fn_80026760(lbl_1_bss_31A, 0.0f, 0.0f, -410.0f);
}
}

extern "C" {
extern "C" void fn_1_19B30(s32 motionNo, f32 startTime, u32 attr) {
    HU3D_MOTIONID currentMotion;

    currentMotion = fn_8002D3C4(lbl_1_bss_31A);
    if (currentMotion != lbl_1_bss_30C[motionNo]) {
        fn_8002CCC0(lbl_1_bss_31A, lbl_1_bss_30C[motionNo],
                           0.0f, startTime, attr);
    }
}
}

extern "C" {
extern "C" void fn_1_19BD4(s32 allowed) {
    lbl_1_bss_47C->pauseAllowed = allowed;
}
}

extern "C" {
extern "C" void fn_1_19BE8(int base) {
    s16 sprite = (s16)(base + 4);
    f32 scale;
    f32 angle;

    angle = 90.0f;
    do {
        HuPrcVSleep();
        scale = (f32) sin((M_PI * (f64) angle) / 180.0);
        HuSprScaleSet(lbl_1_bss_570, sprite, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 180.0f));
    HuSprAttrSet(lbl_1_bss_570, sprite, 4);
}
}

extern "C++" void fn_1_19CD0(s32 baseIndex) {
    f32 scale;
    f32 angle;
    s16 spriteIndex;
    spriteIndex = baseIndex + 4;
    angle = 0.0f;
    HuSprScaleSet(lbl_1_bss_570, spriteIndex, 0.0f, 0.0f);
    HuSprAttrReset(lbl_1_bss_570, spriteIndex, 4);
    do {
        HuPrcVSleep();
        scale = (f32)sin((M_PI * (f64)angle) / 180.0);
        HuSprScaleSet(lbl_1_bss_570, spriteIndex, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 90.0f));
}

extern "C" {
extern "C" void fn_1_19DE4(void) {
    f32 scale;
    f32 angle;

    angle = 0.0f;
    HuSprScaleSet(lbl_1_bss_570, 3, 0.0f, 0.0f);
    HuSprAttrReset(lbl_1_bss_570, 3, 4);
    do {
        HuPrcVSleep();
        scale = (f32) sin((M_PI * (f64) angle) / 180.0);
        HuSprScaleSet(lbl_1_bss_570, 3, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 90.0f));
}
}

extern "C" {
extern "C" void fn_1_19EE0(void) {
    f32 scale;
    f32 angle;

    angle = 90.0f;
    do {
        HuPrcVSleep();
        scale = (f32) sin((M_PI * (f64) angle) / 180.0);
        HuSprScaleSet(lbl_1_bss_570, 3, scale, scale);
        angle += 9.0f;
    } while (!(angle >= 180.0f));
    HuSprAttrSet(lbl_1_bss_570, 3, 4);
}
}

extern "C" {
const QuizAnswerIds lbl_1_rodata_224 = {
    {
        0x001C0000, 0x001C0006, 0x001C000C, 0x001C001E, 0x001C0022,
        0x001C0023, 0x001C0024, 0x001C0025, 0x001C0027, 0x001C0028,
        0x001C0036, 0x001C0039, 0x001C003A, 0x001F0028, 0x001F0029,
        0x001F002A, 0x001F002C, 0x001F002D, 0x00220000, -1
    }
};
}

extern "C" {
const QuizSoundIds lbl_1_rodata_274 = {
    {
        0x000003B6, 0x000003B7, 0x000003B8, 0x000003AE, 0x000003AF,
        0x000003B0, 0x000003CB, 0x000003CC, 0x000003CD, 0x000003CE,
        0x000003CF, 0x000003D0, 0x000003D1, 0x000003D2, 0x000003BA,
        0x000003B2, -1
    }
};
}

extern "C" {
extern "C" void fn_1_19FB0(HUWINID window, u32 answerId, s16 soundIndex) {
    QuizAnswerIds answerIds;
    QuizSoundIds sounds;
    s16 answerIndex;

    answerIds = lbl_1_rodata_224;
    sounds = lbl_1_rodata_274;
    soundIndex--;

    for (answerIndex = 0; ; answerIndex++) {
        if (answerIds.values[answerIndex] == -1) {
            HuAudFXPlay(sounds.values[soundIndex]);
            return;
        }
        if (answerId == (u32)answerIds.values[answerIndex]) {
            if (soundIndex == 0 || soundIndex == 1 ||
                soundIndex == 2 || soundIndex == 14) {
                HuAudFXPlayPan(sounds.values[soundIndex], 0x30);
                return;
            }
            if (soundIndex == 3 || soundIndex == 4 ||
                soundIndex == 5 || soundIndex == 15) {
                HuAudFXPlayPan(sounds.values[soundIndex], 0x50);
            }
            break;
        }
    }
}
}
