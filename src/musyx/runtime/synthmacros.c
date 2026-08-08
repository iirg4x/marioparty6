#define _MATH_H
#include "dolphin/math.h"

#include "musyx/assert.h"
#include "musyx/hardware.h"
#include "musyx/macros.h"
#include "musyx/seq.h"
#include "musyx/snd.h"
#include "musyx/synth.h"
#include "musyx/synth_dbtab.h"
#include "musyx/synthdata.h"

#include <float.h>
#include <string.h>

/* Macro command opcodes. */
#define MAC_CMD_END_OF_MACRO 0
#define MAC_CMD_STOP 1
#define MAC_CMD_IF_KEY 2
#define MAC_CMD_IF_VELOCITY 3
#define MAC_CMD_WAIT 4
#define MAC_CMD_LOOP 5
#define MAC_CMD_GOTO 6
#define MAC_CMD_WAIT_MS 7
#define MAC_CMD_PLAY_MACRO 8
#define MAC_CMD_SEND_KEY_OFF 9
#define MAC_CMD_IF_MODULATION 10
#define MAC_CMD_SET_PIANO_PANNING 11
#define MAC_CMD_SET_ADSR 12
#define MAC_CMD_SCALE_VOLUME 13
#define MAC_CMD_SET_PANNING 14
#define MAC_CMD_ENVELOPE 15
#define MAC_CMD_START_SAMPLE 16
#define MAC_CMD_STOP_SAMPLE 17
#define MAC_CMD_KEY_OFF 18
#define MAC_CMD_IF_RANDOM 19
#define MAC_CMD_FADE_IN 20
#define MAC_CMD_SET_SURROUND_PANNING 21
#define MAC_CMD_SET_ADSR_FROM_CTRL 22
#define MAC_CMD_RANDOM_KEY 23
#define MAC_CMD_ADD_KEY 24
#define MAC_CMD_SET_KEY 25
#define MAC_CMD_LAST_KEY 26
#define MAC_CMD_PORTAMENTO 27
#define MAC_CMD_VIBRATO 28
#define MAC_CMD_PITCH_SWEEP_UP 29
#define MAC_CMD_PITCH_SWEEP_DOWN 30
#define MAC_CMD_SET_PITCH 31
#define MAC_CMD_SET_PITCH_ADSR 32
#define MAC_CMD_SCALE_VOLUME_DLS 33
#define MAC_CMD_SET_MOD_VIBRATO 34
#define MAC_CMD_SETUP_TREMOLO 35
#define MAC_CMD_RETURN 36
#define MAC_CMD_GOSUB 37
#define MAC_CMD_TRAP_EVENT 40
#define MAC_CMD_UNTRAP_EVENT 41
#define MAC_CMD_SEND_MESSAGE 42
#define MAC_CMD_GET_MESSAGE 43
#define MAC_CMD_GET_VID 44
#define MAC_CMD_ADD_AGE_COUNTER 48
#define MAC_CMD_SET_AGE_COUNTER 49
#define MAC_CMD_SEND_FLAG 50
#define MAC_CMD_SET_PITCH_WHEEL_RANGE 51
#define MAC_CMD_SCALE_REVERB 52
#define MAC_CMD_SET_PITCHBEND_AFTER_KEYOFF 53
#define MAC_CMD_SET_PRIORITY 54
#define MAC_CMD_ADD_PRIORITY 55
#define MAC_CMD_SET_AGE_COUNTER_SPEED 56
#define MAC_CMD_SET_AGE_COUNTER_BY_VOLUME 57
#define MAC_CMD_VOLUME_SELECT 64
#define MAC_CMD_PANNING_SELECT 65
#define MAC_CMD_PITCH_WHEEL_SELECT 66
#define MAC_CMD_MOD_WHEEL_SELECT 67
#define MAC_CMD_PEDAL_SELECT 68
#define MAC_CMD_PORTAMENTO_SELECT 69
#define MAC_CMD_REVERB_SELECT 70
#define MAC_CMD_SURROUND_PANNING_SELECT 71
#define MAC_CMD_DOPPLER_SELECT 72
#define MAC_CMD_TREMOLO_SELECT 73
#define MAC_CMD_PRE_AUX_A_SELECT 74
#define MAC_CMD_PRE_AUX_B_SELECT 75
#define MAC_CMD_POST_AUX_B_SELECT 76
#define MAC_CMD_AUX_AFX_SELECT 77
#define MAC_CMD_AUX_BFX_SELECT 78
#define MAC_CMD_SETUP_LFO 80
#define MAC_CMD_MODE_SELECT 88
#define MAC_CMD_SET_KEY_GROUP 89
#define MAC_CMD_SRC_MODE_SELECT 90
#define MAC_CMD_FILTER_SWITCH 94
#define MAC_CMD_FILTER_PARAMETER 95
#define MAC_CMD_VAR_ADD 96
#define MAC_CMD_VAR_SUB 97
#define MAC_CMD_VAR_MUL 98
#define MAC_CMD_VAR_DIV 99
#define MAC_CMD_VAR_ADD_IMMEDIATE 100
#define MAC_CMD_SET_VAR_IMMEDIATE 101
#define MAC_CMD_IF_VAR_EQUAL 112
#define MAC_CMD_IF_VAR_LESS 113

/* Packed macro fields. */
#define MAC_PACK_SHIFT_BYTE 8
#define MAC_PACK_SHIFT_HALFWORD 16
#define MAC_PACK_SHIFT_HIGH_BYTE 24
#define MAC_PACK_SHIFT_AGE 15
#define MAC_PACK_MASK_7BIT 127
#define MAC_PACK_MASK_BYTE 255
#define MAC_PACK_MASK_HALFWORD 65535
#define MAC_PACK_MASK_FREQUENCY 16777215
#define MAC_PACK_MASK_ADSR_SCALE 1023
#define MAC_VAR_INDEX_MASK 31
#define MAC_PACK_SCALE_BYTE 256

/* Voice and control sentinels. */
#define MAC_VOICE_ID_INVALID ((u32)4294967295U)
#define MAC_VOICE_KEY_FX 128
#define MAC_MIDI_NONE 255
#define MAC_U8_MAX 255
#define MAC_WAIT_SENTINEL 65535
#define MAC_LOOP_SENTINEL 65535
#define MAC_CURVE_NONE 65535
#define MAC_MESSAGE_MACRO_NONE 65535
#define MAC_PRIORITY_NONE 65535
#define MAC_SAMPLE_ID_NONE 65535

/* Fixed-point scales and limits. */
#define MAC_FIXED_POINT_UNIT 65536
#define MAC_AGE_SCALE 32768
#define MAC_VOLUME_MAX 8323072
#define MAC_PANNING_MAX 8323072
#define MAC_AGE_INITIAL 1966080000
#define MAC_AGE_MAX_FIXED 2147450880
#define MAC_AGE_COUNTER_MAX 65535
#define MAC_AGE_SPEED_INITIAL 1024
#define MAC_PITCHBEND_CENTER 8192
#define MAC_ADSR_SCALE_MIN ((s32)2147483648U)
#define MAC_ADSR_SCALE_UNSET ((u32)2147483648U)
#define MAC_SIGNED_16BIT_MIN (-32768)
#define MAC_SIGNED_16BIT_MAX 32767

/* Voice state and macro control flags. */
#define MAC_CFLAG_VOICE_ALLOCATED 2
#define MAC_CFLAG_WAITING 4
#define MAC_CFLAG_KEYOFF 8
#define MAC_CFLAG_KEYOFF_REQUEST 128
#define MAC_CFLAG_PERSISTENT 16
#define MAC_CFLAG_SAMPLE_ACTIVE 32
#define MAC_CFLAG_HW_EVENT_WAIT 262144
#define MAC_CFLAG_PORTAMENTO 1024
#define MAC_CFLAG_PITCHBEND_AFTER_KEYOFF 65536
#define MAC_CFLAG_VIBRATO 8192
#define MAC_CFLAG_VIBRATO_MODE 16384
#define MAC_CFLAG_ADSR 256
#define MAC_CFLAG_ENVELOPE 32768
#define MAC_CFLAG_UNYIELD_MASK 262148
#define MAC_CFLAG_SRC_MODE ((u64)8796093022208ULL)
#define MAC_CFLAG_PITCH_ADSR ((u64)2199023255552ULL)
#define MAC_CFLAG_VOLUME ((u64)17592186044416ULL)
#define MAC_CFLAG_PANNING ((u64)35184372088832ULL)
#define MAC_CFLAG_PEDAL ((u64)1099511627776ULL)
#define MAC_CFLAG_TRAP_PENDING ((u64)4398046511104ULL)
#define MAC_CFLAG_KEYOFF_PEDAL ((u64)1099511627784ULL)
#define MAC_CFLAG_INITIALIZED ((u64)52776558133248ULL)

/* Control source and dirty masks. */
#define MAC_SOURCE_VOLUME 524288
#define MAC_SOURCE_PANNING 1048576
#define MAC_SOURCE_PITCH_BEND 2097152
#define MAC_SOURCE_MODULATION 4194304
#define MAC_SOURCE_PEDAL 33554432
#define MAC_SOURCE_PORTAMENTO 16777216
#define MAC_SOURCE_REVERB 8388608
#define MAC_SOURCE_PRE_AUX_A 536870912
#define MAC_SOURCE_PRE_AUX_B 1073741824
#define MAC_SOURCE_POST_AUX_B ((u32)2147483648U)
#define MAC_SOURCE_SURROUND_PANNING 67108864
#define MAC_SOURCE_DOPPLER 134217728
#define MAC_SOURCE_TREMOLO 268435456
#define MAC_FILTER_SWITCH_SOURCE_FLAG 64
#define MAC_FILTER_PARAMETER_SOURCE_FLAG 2048
#define MAC_DIRTY_DOPPLER 16
#define MAC_DIRTY_MODULATION 32
#define MAC_DIRTY_PEDAL 64
#define MAC_DIRTY_PORTAMENTO 128
#define MAC_DIRTY_PRE_AUX_A 256
#define MAC_DIRTY_REVERB 512
#define MAC_DIRTY_PRE_AUX_B 1024
#define MAC_DIRTY_POST_AUX_B 2048
#define MAC_DIRTY_TREMOLO 4096
#define MAC_FILTER_SWITCH_DIRTY_FLAG 8192
#define MAC_FILTER_PARAMETER_DIRTY_FLAG 16384
#define MAC_DIRTY_GLOBAL ((u32)2147483648U)

/* Auxiliary source tables. */
#define MAC_AUX_A_SOURCE_0 ((u64)4294967296ULL)
#define MAC_AUX_A_SOURCE_1 ((u64)8589934592ULL)
#define MAC_AUX_A_SOURCE_2 ((u64)17179869184ULL)
#define MAC_AUX_A_SOURCE_3 ((u64)34359738368ULL)
#define MAC_AUX_A_DIRTY_0 ((u32)2147483649U)
#define MAC_AUX_A_DIRTY_1 ((u32)2147483650U)
#define MAC_AUX_A_DIRTY_2 ((u32)2147483652U)
#define MAC_AUX_A_DIRTY_3 ((u32)2147483656U)
#define MAC_AUX_B_SOURCE_0 ((u64)68719476736ULL)
#define MAC_AUX_B_SOURCE_1 ((u64)137438953472ULL)
#define MAC_AUX_B_SOURCE_2 ((u64)274877906944ULL)
#define MAC_AUX_B_SOURCE_3 ((u64)549755813888ULL)
#define MAC_AUX_B_DIRTY_0 ((u32)2147483664U)
#define MAC_AUX_B_DIRTY_1 ((u32)2147483680U)
#define MAC_AUX_B_DIRTY_2 ((u32)2147483712U)
#define MAC_AUX_B_DIRTY_3 ((u32)2147483776U)

/* MIDI, message, and queue domains. */
#define MAC_MIDI_CTRL_PORTAMENTO 65

static u8 DebugMacroSteps;

static SYNTH_VOICE* macActiveMacroRoot;
static SYNTH_VOICE* macTimeQueueRoot;
static u64 macRealTime;

static void TimeQueueAdd(SYNTH_VOICE* svoice);

void macMakeActive(SYNTH_VOICE* svoice);

void macSetExternalKeyoff(SYNTH_VOICE* svoice);

static void DoSetPitch(SYNTH_VOICE* svoice);

static int SendSingleKeyOff(u32 voiceid) {
  u32 i; // r31

  if (voiceid != MAC_VOICE_ID_INVALID) {

    i = voiceid & MAC_PACK_MASK_BYTE;

    if (voiceid == synthVoice[i].id) {

      macSetExternalKeyoff(&synthVoice[i]);

      return 0;
    }
  }

  return -1;
}

static u32 ExecuteTrap(SYNTH_VOICE* svoice, u8 trapType) {
  if (svoice->trapEventAny != 0 && svoice->trapEventAddr[trapType] != NULL) {
    svoice->curAddr = svoice->trapEventCurAddr[trapType];
    svoice->addr = svoice->trapEventAddr[trapType];
    svoice->trapEventAddr[trapType] = NULL;
    macMakeActive(svoice);
    return 1;
  }

  return 0;
}

static u32 HasHWEventTrap(SYNTH_VOICE* svoice) {
  if (svoice->trapEventAny != '\0') {
    return svoice->trapEventAddr[1] != NULL;
  }
  return 0;
}

static void CheckHWEventTrap(SYNTH_VOICE* svoice) {
  if ((svoice->cFlags & MAC_CFLAG_SAMPLE_ACTIVE) == 0 && !hwIsActive(svoice->id & MAC_PACK_MASK_BYTE)) {
    ExecuteTrap(svoice, 1);
  }
}

static u32 mcmdWait(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 w;  // r1+0x10
  u32 ms; // r29

  if ((ms = (u16)(cstep->para[1] >> MAC_PACK_SHIFT_HALFWORD))) {
    if (((u8)(cstep->para[0] >> 8) & 1)) {
      if (svoice->cFlags & 8) {
        if (!(svoice->cFlags & MAC_CFLAG_PEDAL)) {
          return 0;
        }
        svoice->cFlags |= MAC_CFLAG_TRAP_PENDING;
      }
      svoice->cFlags |= 4;
    } else {
      svoice->cFlags &= ~4;
    }

    if (((u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE) & 1)) {
      if (!(svoice->cFlags & MAC_CFLAG_SAMPLE_ACTIVE) && !hwIsActive(svoice->id & MAC_PACK_MASK_BYTE)) {
        return 0;
      }
      svoice->cFlags |= MAC_CFLAG_HW_EVENT_WAIT;
    } else {
      svoice->cFlags &= ~MAC_CFLAG_HW_EVENT_WAIT;
    }

    if (((u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD)) & 1) {
      ms = sndRand() % ms;
    }

    if (ms != MAC_WAIT_SENTINEL) {
      if ((w = ((u8)(cstep->para[1] >> MAC_PACK_SHIFT_BYTE) & 1) != 0)) {
        sndConvertMs(&ms);
      } else {
        sndConvertTicks(&ms, svoice);
      }

      if (w != 0) {
        if ((u8)cstep->para[1] & 1) {
          svoice->wait = svoice->macStartTime + ms;
        } else {
          svoice->wait = macRealTime + ms;
        }
      } else {
        if ((u8)cstep->para[1] & 1) {
          svoice->wait = ms;
        } else {
          svoice->wait = svoice->waitTime + ms;
        }
      }

      if (!(svoice->wait > macRealTime)) {
        svoice->waitTime = svoice->wait;
        svoice->wait = 0;
      }
    } else {
      svoice->wait = -1;
    }

    if (svoice->wait != 0) {
      if (svoice->wait != -1) {
        TimeQueueAdd(svoice);
      }
      macMakeInactive(svoice, 1);
      return 1;
    }
  }

  return 0;
}

static u32 mcmdWaitMs(SYNTH_VOICE* svoice, MSTEP* cstep) {
  *((u8*)cstep->para + 6) = 1;
  return mcmdWait(svoice, cstep);
}

static u32 mcmdEndOfMacro(SYNTH_VOICE* svoice) {
  vidRemoveVoiceReferences(svoice);
  voiceFree(svoice);
  return 1;
}

static u32 mcmdStop(SYNTH_VOICE* svoice) { return mcmdEndOfMacro(svoice); }

static u32 mcmdReturn(SYNTH_VOICE* svoice) {
  if (svoice->callStackEntryNum != 0) {
    svoice->addr = svoice->callStack[svoice->callStackIndex].addr;
    svoice->curAddr = svoice->callStack[svoice->callStackIndex].curAddr;
    svoice->callStackIndex = (svoice->callStackIndex - 1) & 3;
    --svoice->callStackEntryNum;
  }
  return 0;
}

static void mcmdIfKey(SYNTH_VOICE* svoice, MSTEP* cstep) {
  MSTEP* addr; // r31
  if (svoice->curNote < ((u8)(cstep->para[0] >> 8))) {
    return;
  }

  if ((addr = (MSTEP*)dataGetMacro((cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD))) != NULL) {
    svoice->addr = addr;
    svoice->curAddr = addr + (u16)cstep->para[1];
  }
}

static void mcmdIfVelocity(SYNTH_VOICE* svoice, MSTEP* cstep) {
  MSTEP* addr;
  if (((u8)(svoice->volume >> MAC_PACK_SHIFT_HALFWORD)) < (u8)(cstep->para[0] >> 8)) {
    return;
  }

  if ((addr = (MSTEP*)dataGetMacro(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD))) {
    svoice->addr = addr;
    svoice->curAddr = addr + (u16)cstep->para[1];
  }
}

static void mcmdIfModulation(SYNTH_VOICE* svoice, MSTEP* cstep) {
  MSTEP* addr; // r30
  u8 mod;      // r28

  if (svoice->midi == MAC_MIDI_NONE) {
    return;
  }
  mod = inpGetModulation(svoice) >> 7;
  if (mod < (u8)(cstep->para[0] >> 8)) {
    return;
  }

  if ((addr = (MSTEP*)dataGetMacro(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD))) {
    svoice->addr = addr;
    svoice->curAddr = addr + (u16)(cstep->para[1]);
  }
}

static void mcmdIfRandom(SYNTH_VOICE* svoice, MSTEP* cstep) {
  MSTEP* addr; // r31
  if ((u8)sndRand() < (u8)(cstep->para[0] >> 8)) {
    return;
  }

  if ((addr = (MSTEP*)dataGetMacro(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD))) {
    svoice->addr = addr;
    svoice->curAddr = addr + (u16)cstep->para[1];
  }
}

static u32 mcmdGoto(SYNTH_VOICE* svoice, MSTEP* cstep) {
  MSTEP* addr; // r31
  if ((addr = (MSTEP*)dataGetMacro(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD)) != NULL) {
    svoice->addr = addr;
    svoice->curAddr = addr + (u16)cstep->para[1];
    return 0;
  }

  return mcmdEndOfMacro(svoice);
}

static u32 mcmdGosub(SYNTH_VOICE* svoice, MSTEP* cstep) {
  MSTEP* addr; // r30
  if ((addr = (MSTEP*)dataGetMacro((u16)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD))) != NULL) {
    svoice->callStackIndex = (svoice->callStackIndex + 1) & 3;
    svoice->callStack[svoice->callStackIndex].addr = svoice->addr;
    svoice->callStack[svoice->callStackIndex].curAddr = svoice->curAddr;
    if (++svoice->callStackEntryNum > 4) {
      svoice->callStackEntryNum = 4;
    }

    svoice->addr = addr;
    svoice->curAddr = addr + (u16)cstep->para[1];
    return 0;
  }

  return mcmdEndOfMacro(svoice);
}

static void mcmdTrapEvent(SYNTH_VOICE* svoice, MSTEP* cstep) {
  MSTEP* addr; // r29
  u8 t;        // r30
  if ((addr = (MSTEP*)dataGetMacro(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD)) != NULL) {
    t = (u8)(cstep->para[0] >> 8);
    svoice->trapEventAddr[t] = addr;
    svoice->trapEventCurAddr[t] = addr + (u16)cstep->para[1];
    svoice->trapEventAny = 1;
    if (t == 0 && (svoice->cFlags & MAC_CFLAG_KEYOFF_PEDAL) == MAC_CFLAG_KEYOFF_PEDAL) {
      svoice->cFlags |= MAC_CFLAG_TRAP_PENDING;
    }
  }
}

static void mcmdUntrapEvent(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u8 i; // r31
  svoice->trapEventAddr[(u8)(cstep->para[0] >> 8)] = 0;

  for (i = 0; i < 3; ++i) {
    if (svoice->trapEventAddr[i] != NULL) {
      return;
    }
  }

  svoice->trapEventAny = 0;
}

static void mcmdLoop(SYNTH_VOICE* svoice, MSTEP* cstep) {

  if (svoice->loop == 0) {
    if ((u8)(cstep->para[0] >> 16) & 1) {
      svoice->loop = sndRand() % (u16)(cstep->para[1] >> 16);
    } else {
      svoice->loop = (cstep->para[1] >> 16);
    }

    if (svoice->loop == MAC_LOOP_SENTINEL) {
      goto skip;
    }
    ++svoice->loop;
  } else if (svoice->loop == MAC_LOOP_SENTINEL) {
    goto skip;
  }

  if (--svoice->loop == 0) {
    return;
  }
skip:
  if (((u8)(cstep->para[0] >> 8) & 1) != 0 && (svoice->cFlags & MAC_CFLAG_KEYOFF_PEDAL) == MAC_CFLAG_KEYOFF) {
    svoice->loop = 0;

  } else if (((u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE) & 1) && (svoice->cFlags & MAC_CFLAG_SAMPLE_ACTIVE) == 0 &&
             !hwIsActive(svoice->id & MAC_PACK_MASK_BYTE)) {
    svoice->loop = 0;
  } else {
    svoice->curAddr = svoice->addr + ((u16)cstep->para[1]);
  }
}

static void mcmdPlayMacro(SYNTH_VOICE* svoice, MSTEP* cstep) {
  s32 key;       // r29
  u32 new_child; // r30

  key = ((u32)svoice->orgNote + (s8)(u8)(cstep->para[0] >> 8));
  key = (key < 0) ? 0 : key > MAC_PACK_MASK_7BIT ? MAC_PACK_MASK_7BIT : key;

  if (svoice->fxFlag != 0) {
    key |= MAC_VOICE_KEY_FX;
  }

  svoice->block = 1;
  new_child = macStart((u16)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD), (u8)(cstep->para[1] >> MAC_PACK_SHIFT_HALFWORD),
                       (u8)(cstep->para[1] >> MAC_PACK_SHIFT_HIGH_BYTE), svoice->allocId, key,
                       (u8)(svoice->volume >> MAC_PACK_SHIFT_HALFWORD), (u8)(svoice->panning[0] >> MAC_PACK_SHIFT_HALFWORD), svoice->midi,
                       svoice->midiSet, svoice->section, (u16)cstep->para[1], (u16)svoice->track, 0,
                       svoice->vGroup, svoice->studio, svoice->itdMode == 0);
  svoice->block = 0;
  if (new_child != MAC_VOICE_ID_INVALID) {
    svoice->lastVID = synthVoice[(u8)new_child].vidList->vid;
    synthVoice[(u8)new_child].parent = svoice->id;
    if (svoice->child != -1) {
      synthVoice[(u8)new_child].child = svoice->child;
      synthVoice[(u8)svoice->child].parent = new_child;
    }
    svoice->child = new_child;
    if (svoice->fxFlag != 0) {
      synthFXCloneMidiSetup(&synthVoice[(u8)new_child], svoice);
    }
  } else {
    svoice->lastVID = MAC_VOICE_ID_INVALID;
  }
}

static void mcmdSendKeyOff(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 voiceid; // r30
  u32 i;       // r31

  voiceid = (svoice->orgNote + (u8)(cstep->para[0] >> 8)) << 8;
  voiceid |= ((u16)(cstep->para[0] >> 16)) << 16;
  for (i = 0; i < synthInfo.voiceNum; ++i) {
    if (synthVoice[i].id == (voiceid | i)) {
      SendSingleKeyOff(voiceid | i);
    }
  }
}

static void mcmdAddAgeCounter(SYNTH_VOICE* svoice, MSTEP* cstep) {
  s16 step; // r29
  s32 age;  // r30

  step = (u16)(cstep->para[0] >> 16);
  age = (svoice->age >> 15) + step;

  if (age < 0) {
    svoice->age = 0;
  } else if (age > MAC_AGE_COUNTER_MAX) {
    svoice->age = MAC_AGE_MAX_FIXED;
  } else {
    svoice->age = age * MAC_AGE_SCALE;
  }

  hwSetPriority(svoice->id & MAC_PACK_MASK_BYTE, ((u32)svoice->prio << 24) | ((u32)svoice->age >> 15));
}

static void mcmdSetAgeCounter(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->age = (u16)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD) << MAC_PACK_SHIFT_AGE;
  hwSetPriority(svoice->id & MAC_PACK_MASK_BYTE, (u32)svoice->prio << MAC_PACK_SHIFT_HIGH_BYTE | svoice->age >> MAC_PACK_SHIFT_AGE);
}

static void mcmdSetAgeCounterSpeed(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 time = cstep->para[1];
  if (time != 0) {
    svoice->ageSpeed = (svoice->age >> 8) / time;
  } else {
    svoice->ageSpeed = 0;
  }
}
static void mcmdSetAgeCounterByVolume(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 age; // r30

  age = (((u8)(svoice->volume >> 16) * (u16)cstep->para[1]) >> 7) + (u16)(cstep->para[0] >> 16);
  svoice->age = age > 60000 ? MAC_AGE_INITIAL : age * MAC_AGE_SCALE;
  hwSetPriority(svoice->id & MAC_PACK_MASK_BYTE, (u32)svoice->prio << MAC_PACK_SHIFT_HIGH_BYTE | svoice->age >> MAC_PACK_SHIFT_AGE);
}

static void mcmdAddPriority(SYNTH_VOICE* svoice, MSTEP* cstep) {
  s16 add;  // r30
  s16 prio; // r31
  add = (u16)(cstep->para[0] >> 16);
  prio = svoice->prio + add;
  prio = (prio < 0) ? 0 : (prio > MAC_U8_MAX) ? MAC_U8_MAX : prio;

  voiceSetPriority(svoice, prio);
}

static void mcmdSetPriority(SYNTH_VOICE* svoice, MSTEP* cstep) {
  voiceSetPriority(svoice, cstep->para[0] >> 8);
}

static void mcmdSendFlag(MSTEP* cstep) {
  synthGlobalVariable[(u8)(cstep->para[0] >> 8)] = (u8)(cstep->para[0] >> 16);
}

static void mcmdSetPitchWheelRange(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->pbLowerKeyRange = (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD);
  svoice->pbUpperKeyRange = (u8)(cstep->para[0] >> 8);
}

static u32 mcmdSetKey(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->curNote = (u8)(cstep->para[0] >> 8) & MAC_PACK_MASK_7BIT;
  svoice->curDetune = (s8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD);
  if (voiceIsLastStarted(svoice) != 0) {
    inpSetMidiLastNote(svoice->midi, svoice->midiSet, svoice->curNote & MAC_PACK_MASK_BYTE);
  }
  cstep->para[0] = 4;
  return mcmdWait(svoice, cstep);
}

static u32 mcmdAddKey(SYNTH_VOICE* svoice, MSTEP* cstep) {
  if ((u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE) == 0) {
    svoice->curNote += (s8)(u8)(cstep->para[0] >> 8);
  } else {
    svoice->curNote = (u16)svoice->orgNote + (s16)(s8)(u8)(cstep->para[0] >> 8);
  }

  svoice->curNote = (s16)svoice->curNote < 0 ? 0 : svoice->curNote > MAC_PACK_MASK_7BIT ? MAC_PACK_MASK_7BIT : svoice->curNote;
  svoice->curDetune = (s8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD);

  if (voiceIsLastStarted(svoice) != 0) {
    inpSetMidiLastNote(svoice->midi, svoice->midiSet, svoice->curNote);
  }
  cstep->para[0] = 4;
  return mcmdWait(svoice, cstep);
}

static u32 mcmdLastKey(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->curNote = svoice->lastNote + (s8)(u8)(cstep->para[0] >> 8);
  svoice->curNote = (s16)svoice->curNote < 0 ? 0 : svoice->curNote > MAC_PACK_MASK_7BIT ? MAC_PACK_MASK_7BIT : svoice->curNote;
  svoice->curDetune = (s8)(cstep->para[0] >> 16);
  if (svoice->midi != MAC_MIDI_NONE) {
    inpSetMidiLastNote(svoice->midi, svoice->midiSet, svoice->curNote);
  }
  cstep->para[0] = 4;

  return mcmdWait(svoice, cstep);
}

static void mcmdStartSample(SYNTH_VOICE* svoice, MSTEP* cstep) {
  static SAMPLE_INFO newsmp;
  u16 smp; // r28
  smp = cstep->para[0] >> 8;

  if (dataGetSample(smp, &newsmp) != 0) {
    return;
  }
  switch ((u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE)) {
  case 0:
    newsmp.offset = cstep->para[1];
    break;
  case 1:
    newsmp.offset = ((u8)(MAC_PACK_MASK_7BIT - (svoice->volume >> MAC_PACK_SHIFT_HALFWORD)) * (u32)cstep->para[1]) / MAC_PACK_MASK_7BIT;
    ;
    break;
  case 2:
    newsmp.offset = ((u8)((svoice->volume >> MAC_PACK_SHIFT_HALFWORD)) * (u32)cstep->para[1]) / MAC_PACK_MASK_7BIT;
    break;
  default:
    newsmp.offset = 0;
    break;
  }

  if (newsmp.offset >= newsmp.length) {
    newsmp.offset = newsmp.length - 1;
  }

  hwInitSamplePlayback(svoice->id & MAC_PACK_MASK_BYTE, smp, &newsmp, (svoice->cFlags & MAC_CFLAG_ADSR) == 0,
                       ((u32)svoice->prio << 24) | ((u32)svoice->age >> 15), svoice->id,
                       (svoice->cFlags & MAC_CFLAG_SRC_MODE) == 0, svoice->itdMode);

  svoice->sInfo = newsmp.info;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
  svoice->sampleId = smp;
#endif

  if (svoice->playFrq != -1) {
    DoSetPitch(svoice);
  }
  svoice->cFlags |= MAC_CFLAG_SAMPLE_ACTIVE;
  synthKeyStateUpdate(svoice);
}

static void mcmdStopSample(SYNTH_VOICE* svoice) { hwBreak(svoice->id & MAC_PACK_MASK_BYTE); }
static void mcmdKeyOff(SYNTH_VOICE* svoice) {
  svoice->cFlags |= MAC_CFLAG_KEYOFF_REQUEST;
  synthKeyStateUpdate(svoice);
}

static void mcmdSetMod2Vibrato(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->vibModAddScale = (s8)(cstep->para[0] >> 8) << 8;
  if (svoice->vibModAddScale >= 0) {
    svoice->vibModAddScale += ((s16)(s8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD) << 8) / 100;

  } else {
    svoice->vibModAddScale -= ((s16)(s8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD) << 8) / 100;
  }
}

static void mcmdVibrato(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 time; // r1+0x10
  s8 kr;    // r29
  s8 cr;    // r30

  if ((u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE) & 3) {
    svoice->cFlags |= MAC_CFLAG_VIBRATO_MODE;
  } else {
    svoice->cFlags &= ~MAC_CFLAG_VIBRATO_MODE;
  }

  time = (u16)(cstep->para[1] >> MAC_PACK_SHIFT_HALFWORD);
  if ((u8)(cstep->para[1] >> 8) & 1) {
    sndConvertMs(&time);
  } else {
    sndConvertTicks(&time, svoice);
  }

  if (time) {
    svoice->cFlags |= MAC_CFLAG_VIBRATO;
    svoice->vibPeriod = time;

    kr = (s8)(cstep->para[0] >> 8);
    cr = (s8)(cstep->para[0] >> 16);

    if (kr < 0) {
      if (cr < 0) {
        svoice->vibCentRange = -cr;
      } else {
        svoice->vibCentRange = cr;
      }

      svoice->vibKeyRange = -kr;
      svoice->vibCurTime = svoice->vibPeriod / 2;
    } else {
      if (cr < 0) {
        if (kr == 0) {
          svoice->vibCentRange = -cr;
          svoice->vibCurTime = svoice->vibPeriod / 2;
        } else {
          --kr;
          svoice->vibCentRange = 100 - cr;
          svoice->vibCurTime = 0;
        }
      } else {
        svoice->vibCentRange = cr;
        svoice->vibCurTime = 0;
      }
      svoice->vibKeyRange = kr;
    }
  } else {
    svoice->cFlags &= ~MAC_CFLAG_VIBRATO;
  }
}

static void mcmdSetupLFO(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 time;  // r1+0x14
  u32 phase; // r1+0x10
  u8 n;      // r31

  n = (u8)(cstep->para[0] >> 8);
  time = (u16)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD);
  sndConvertMs(&time);
  if (svoice->lfo[n].period != 0) {
    phase = (u16)cstep->para[1];
    sndConvertMs(&phase);
    svoice->lfo[n].time = phase;
  }
  svoice->lfo[n].period = time;
}

static void DoSetPitch(SYNTH_VOICE* svoice) {
  u32 f;    // r29
  u32 of;   // r26
  u32 i;    // r31
  u32 frq;  // r28
  u32 ofrq; // r27
  u32 no;   // r30
  s32 key;  // r25
  u8 oKey;  // r24
  static u16 kf[13] = {
      4096, 4339, 4597, 4871, 5160, 5467, 5792, 6137, 6502, 6888, 7298, 7732, 8192,
  };

  frq = svoice->playFrq & MAC_PACK_MASK_FREQUENCY;
  ofrq = svoice->sInfo & MAC_PACK_MASK_FREQUENCY;

  if (ofrq == frq) {
    svoice->curNote = svoice->sInfo >> 24;
    svoice->curDetune = 0;
  } else if (ofrq < frq) {
    f = (frq << 12) / ofrq;
    of = f >> 12;

    for (no = 0; no < 11; no++) {
      if (of < (1 << (no + 1))) {
        break;
      }
    }

    f /= (1 << no);

    for (i = 11;; i--) {
      if (f > kf[i]) {
        break;
      }
    }

    svoice->curNote = (svoice->sInfo >> 24) + (no * 12) + i;
    svoice->curDetune = ((f - kf[i]) * 100) / (kf[i + 1] - kf[i]);
  } else {
    f = (ofrq << 12) / frq;
    of = f >> 12;

    for (no = 0; no < 11; no++) {
      if (of < (1 << (no + 1))) {
        break;
      }
    }

    f /= (1 << no);

    for (i = 11;; i--) {
      if (f > kf[i]) {
        break;
      }
    }

    key = i + (no * 12);
    oKey = (svoice->sInfo >> 24);
    if (key > oKey) {
      svoice->curNote = svoice->curDetune = 0;
    } else {
      svoice->curNote = oKey - key;
      svoice->curDetune = ((kf[i] - f) * 100) / (kf[i + 1] - kf[i]);
    }
  }
}

static void mcmdSetPitch(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->playFrq = (u32)(cstep->para[0] >> 8);
  svoice->playFrq |= (u8)cstep->para[1];
  if (svoice->sInfo != -1) {
    DoSetPitch(svoice);
  }
}

static void mcmdSetADSR(SYNTH_VOICE* svoice, MSTEP* cstep) {
  ADSR_INFO adsr;      // r1+0x8
  ADSR_INFO* adsr_ptr; // r31
  s32 ascale;          // r28
  s32 dscale;          // r27
  f32 sScale;          // f31

  if ((adsr_ptr = dataGetCurve(cstep->para[0] >> 8)) != NULL) {
    if (!(u8)(cstep->para[0] >> 24)) {
      adsr.data.linear.atime = adsr_ptr->data.linear.atime >> 8 | adsr_ptr->data.linear.atime << 8;
      adsr.data.linear.dtime = adsr_ptr->data.linear.dtime >> 8 | adsr_ptr->data.linear.dtime << 8;
      adsr.data.linear.slevel = adsr_ptr->data.linear.slevel >> 8 | adsr_ptr->data.linear.slevel
                                                                        << 8;
      adsr.data.linear.rtime = adsr_ptr->data.linear.rtime >> 8 | adsr_ptr->data.linear.rtime << 8;
      hwSetADSR(svoice->id & MAC_PACK_MASK_BYTE, &adsr, FALSE);
    } else {
      sScale =
          dspDLSVolTab[(u16)(adsr_ptr->data.dls.slevel >> 8 | adsr_ptr->data.dls.slevel << 8) >> 5];
      adsr.data.dls.atime =
          ((u8*)&adsr_ptr->data.dls.atime)[0] << 0 | ((u8*)&adsr_ptr->data.dls.atime)[1] << 8 |
          ((u8*)&adsr_ptr->data.dls.atime)[2] << 16 | ((u8*)&adsr_ptr->data.dls.atime)[3] << 24;
      adsr.data.dls.dtime =
          ((u8*)&adsr_ptr->data.dls.dtime)[0] << 0 | ((u8*)&adsr_ptr->data.dls.dtime)[1] << 8 |
          ((u8*)&adsr_ptr->data.dls.dtime)[2] << 16 | ((u8*)&adsr_ptr->data.dls.dtime)[3] << 24;
      adsr.data.dls.slevel = 4096.f * sScale;
      adsr.data.dls.rtime = adsr_ptr->data.dls.rtime >> 8 | adsr_ptr->data.dls.rtime << 8;
      ascale =
          ((u8*)&adsr_ptr->data.dls.ascale)[0] << 0 | ((u8*)&adsr_ptr->data.dls.ascale)[1] << 8 |
          ((u8*)&adsr_ptr->data.dls.ascale)[2] << 16 | ((u8*)&adsr_ptr->data.dls.ascale)[3] << 24;

      dscale =
          ((u8*)&adsr_ptr->data.dls.dscale)[0] << 0 | ((u8*)&adsr_ptr->data.dls.dscale)[1] << 8 |
          ((u8*)&adsr_ptr->data.dls.dscale)[2] << 16 | ((u8*)&adsr_ptr->data.dls.dscale)[3] << 24;

      if (ascale != MAC_ADSR_SCALE_UNSET) {
        adsr.data.dls.atime += (s32)(FLT_EPSILON * svoice->orgVolume * ascale);
      }

      if (dscale != MAC_ADSR_SCALE_UNSET) {
        adsr.data.dls.dtime += (s32)(0.0078125f * svoice->orgNote * dscale);
      }

      hwSetADSR(svoice->id & MAC_PACK_MASK_BYTE, &adsr, TRUE);
    }

    svoice->cFlags |= MAC_CFLAG_ADSR;
  }
}

static s32 midi2TimeTab[128] = {
    0,      10,     20,     30,     40,     50,     60,     70,     80,     90,     100,    110,
    110,    120,    130,    140,    150,    160,    170,    190,    200,    220,    230,    250,
    270,    290,    310,    330,    350,    380,    410,    440,    470,    500,    540,    580,
    620,    660,    710,    760,    820,    880,    940,    1000,   1000,   1100,   1200,   1300,
    1400,   1500,   1600,   1700,   1800,   2000,   2100,   2300,   2400,   2600,   2800,   3000,
    3200,   3500,   3700,   4000,   4300,   4600,   4900,   5300,   5700,   6100,   6500,   7000,
    7500,   8100,   8600,   9300,   9900,   10000,  11000,  12000,  13000,  14000,  15000,  16000,
    17000,  18000,  19000,  21000,  22000,  24000,  26000,  28000,  30000,  32000,  34000,  37000,
    39000,  42000,  45000,  49000,  50000,  55000,  60000,  65000,  70000,  75000,  80000,  85000,
    90000,  95000,  100000, 105000, 110000, 115000, 120000, 125000, 130000, 135000, 140000, 145000,
    150000, 155000, 160000, 165000, 170000, 175000, 180000, 0,
};

static void mcmdSetADSRFromCtrl(SYNTH_VOICE* svoice, MSTEP* cstep) {
  // Local variables
  f32 sScale;     // f31
  ADSR_INFO adsr; // r1+0x8

  sScale = dspDLSVolTab[inpGetMidiCtrl(cstep->para[0] >> 24, svoice->midi, svoice->midiSet) >> 7];
  adsr.data.dls.atime =
      midi2TimeTab[inpGetMidiCtrl(cstep->para[0] >> 8, svoice->midi, svoice->midiSet) >> 7];
  adsr.data.dls.dtime =
      midi2TimeTab[inpGetMidiCtrl(cstep->para[0] >> 16, svoice->midi, svoice->midiSet) >> 7];
  adsr.data.dls.slevel = 193 - dspScale2IndexTab[(u32)(1023.f * sScale)];
  adsr.data.dls.rtime =
      midi2TimeTab[inpGetMidiCtrl(cstep->para[1], svoice->midi, svoice->midiSet) >> 7];
  adsr.data.dls.ascale = MAC_ADSR_SCALE_MIN;
  adsr.data.dls.dscale = MAC_ADSR_SCALE_MIN;
  hwSetADSR((u8)svoice->id, &adsr, 2);
  svoice->cFlags |= MAC_CFLAG_ADSR;
}

static void mcmdSetPitchADSR(SYNTH_VOICE* svoice, MSTEP* cstep) {
  ADSR_INFO adsr;      // r1+0x8
  ADSR_INFO* adsr_ptr; // r30
  u32 sl;              // r28
  s32 ascale;          // r27
  s32 dscale;          // r26

  adsr_ptr = dataGetCurve((cstep->para[0] >> 8));

  if (adsr_ptr == NULL) {
    return;
  }

  svoice->pitchADSRRange = ((s8)cstep->para[1] << 8);

  if (svoice->pitchADSRRange >= 0) {
    svoice->pitchADSRRange += ((s8)(cstep->para[1] >> 8) << 8) / 100;
  } else {
    svoice->pitchADSRRange -= ((s8)(cstep->para[1] >> 8) << 8) / 100;
  }

  adsr.data.dls.atime =
      (((u8*)&adsr_ptr->data.dls.atime)[0] << 0) | (((u8*)&adsr_ptr->data.dls.atime)[1] << 8) |
      (((u8*)&adsr_ptr->data.dls.atime)[2] << 16) | (((u8*)&adsr_ptr->data.dls.atime)[3] << 24);
  adsr.data.dls.dtime =
      (((u8*)&adsr_ptr->data.dls.dtime)[0] << 0) | (((u8*)&adsr_ptr->data.dls.dtime)[1] << 8) |
      (((u8*)&adsr_ptr->data.dls.dtime)[2] << 16) | (((u8*)&adsr_ptr->data.dls.dtime)[3] << 24);

  adsr.data.dls.slevel = (adsr_ptr->data.dls.slevel >> 8) | (adsr_ptr->data.dls.slevel << 8);
  adsr.data.dls.rtime = (adsr_ptr->data.dls.rtime >> 8) | (adsr_ptr->data.dls.rtime << 8);
  ascale =
      (((u8*)&adsr_ptr->data.dls.ascale)[0] << 0) | (((u8*)&adsr_ptr->data.dls.ascale)[1] << 8) |
      (((u8*)&adsr_ptr->data.dls.ascale)[2] << 16) | (((u8*)&adsr_ptr->data.dls.ascale)[3] << 24);
  dscale =
      (((u8*)&adsr_ptr->data.dls.dscale)[0] << 0) | (((u8*)&adsr_ptr->data.dls.dscale)[1] << 8) |
      (((u8*)&adsr_ptr->data.dls.dscale)[2] << 16) | (((u8*)&adsr_ptr->data.dls.dscale)[3] << 24);

  if (ascale != MAC_ADSR_SCALE_UNSET) {
    adsr.data.dls.atime += (s32)((FLT_EPSILON * svoice->orgVolume) * (f32)ascale);
  }
  if (dscale != MAC_ADSR_SCALE_UNSET) {
    adsr.data.dls.dtime += (s32)((0.0078125f * svoice->orgNote) * (f32)dscale);
  }

  svoice->pitchADSR.mode = 1;
  svoice->pitchADSR.data.dls.aMode = 0;
  svoice->pitchADSR.data.dls.aTime = adsrConvertTimeCents(adsr.data.dls.atime);
  svoice->pitchADSR.data.dls.dTime = adsrConvertTimeCents(adsr.data.dls.dtime);
  sl = adsr.data.dls.slevel >> 2;
  if (sl > MAC_PACK_MASK_ADSR_SCALE) {
    sl = MAC_PACK_MASK_ADSR_SCALE;
  }

  svoice->pitchADSR.data.dls.sLevel = 193 - dspScale2IndexTab[sl];
  svoice->pitchADSR.data.dls.rTime = adsr.data.dls.rtime;
  ;
  adsrSetup(&svoice->pitchADSR);
  svoice->cFlags |= MAC_CFLAG_PITCH_ADSR;
}

static u32 mcmdPitchSweep(SYNTH_VOICE* svoice, MSTEP* cstep, int num) {
  s32 delta; // r31
  svoice->sweepOff[num] = 0;
  svoice->sweepNum[num] = (u8)(cstep->para[0] >> 8);
  svoice->sweepCnt[num] = (s32)svoice->sweepNum[num] << MAC_PACK_SHIFT_HALFWORD;
  delta = (int)(short)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD);
  if (delta >= 0) {
    delta = hwFrq2Pitch(delta);
  } else {
    delta = -hwFrq2Pitch(-delta);
  }
  svoice->sweepAdd[num] = delta << MAC_PACK_SHIFT_HALFWORD;
  cstep->para[0] = 0;
  return mcmdWait(svoice, cstep);
}

static void DoPanningSetup(SYNTH_VOICE* svoice, MSTEP* cstep, u8 pi) {
  s32 width;  // r29
  u32 mstime; // r27
  svoice->panTime[pi] = width = (u16)(cstep->para[0] >> 16);
  sndConvertMs(&svoice->panTime[pi]);
  mstime = (s8)(cstep->para[1]);
  svoice->panning[pi] = ((u8)(cstep->para[0] >> 8)) << 16;
  svoice->panTarget[pi] = svoice->panning[pi] + mstime * MAC_FIXED_POINT_UNIT;
  if (svoice->panTime[pi] != 0) {
    svoice->panDelta[pi] = (s32)(mstime << 16) / width;
  } else {
    svoice->panDelta[pi] = (s32)(mstime << 16);
  }

  svoice->cFlags |= MAC_CFLAG_PANNING;
}

static void mcmdSetPanning(SYNTH_VOICE* svoice, MSTEP* cstep) { DoPanningSetup(svoice, cstep, 0); }

static void mcmdSetSurroundPanning(SYNTH_VOICE* svoice, MSTEP* cstep) {
  DoPanningSetup(svoice, cstep, 1);
}

static void mcmdSetPianoPanning(SYNTH_VOICE* svoice, MSTEP* cstep) {
  s32 delta; // r31
  s32 scale; // r30
  delta = (svoice->curNote << 16) - ((u8)(cstep->para[0] >> 16) << 16);
  scale = (s8)((u8)(cstep->para[0] >> 8));
  delta = ((delta * scale) >> 7);
  delta += (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE) << 16;
  delta = delta < 0 ? 0 : delta > MAC_PANNING_MAX ? MAC_PANNING_MAX : delta;
  svoice->panTarget[0] = delta;
  svoice->panning[0] = delta;
}

static u32 TranslateVolume(u32 volume, u16 curve) {
  u8* ptr;   // r30
  u32 vlow;  // r28
  u32 vhigh; // r31
  s32 d;     // r27

  if (curve != MAC_CURVE_NONE) {
    if ((ptr = (u8*)dataGetCurve(curve))) {
      vhigh = (volume >> 16) & MAC_PACK_MASK_HALFWORD;
      vlow = volume & MAC_PACK_MASK_HALFWORD;

      if (vhigh < MAC_PACK_MASK_7BIT) {
        d = vlow * (ptr[vhigh + 1] - ptr[vhigh]);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 0)
        volume = d + ((u16)ptr[vhigh] << 16);
#else
        volume = d + (ptr[vhigh] << 16);
#endif
      } else {
        volume = ptr[vhigh] << 16;
      }
    }
  }

  return volume;
}

static void mcmdScaleVolume(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u16 curve; // r29
  u16 scale; // r28
  scale = (u16)(u8)(cstep->para[0] >> 8);

  if ((u8)(cstep->para[1] >> 8) == 0) {
    svoice->volume = (svoice->volume * scale) / MAC_PACK_MASK_7BIT;
  } else {
    svoice->volume = (svoice->orgVolume * scale) / MAC_PACK_MASK_7BIT;
  }
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 0)
  svoice->volume += (u8)(cstep->para[0] >> 16) << 16;
#else
  svoice->volume += EXTRACT_3RDNYBBLE(cstep->para[0]);
#endif
  if (svoice->volume > MAC_VOLUME_MAX) {
    svoice->volume = MAC_VOLUME_MAX;
  }

  curve = (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE);
  curve |= ((u16)((u8)cstep->para[1]) << 8);

  svoice->volume = TranslateVolume(svoice->volume, curve);
  svoice->cFlags |= MAC_CFLAG_VOLUME;
}

static void mcmdScaleVolumeDLS(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u16 scale; // r31

  scale = (cstep->para[0] >> 8);
  if ((u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE) == 0) {
    svoice->volume = ((svoice->volume >> 5) * scale) >> 7;
  } else {
    svoice->volume = ((svoice->orgVolume >> 5) * scale) >> 7;
  }
  if (svoice->volume > MAC_VOLUME_MAX) {
    svoice->volume = MAC_VOLUME_MAX;
  }

  svoice->cFlags |= MAC_CFLAG_VOLUME;
}

static void DoEnvelopeCalculation(SYNTH_VOICE* svoice, MSTEP* cstep, s32 start_vol) {
  u32 tvol;   // r31
  u32 time;   // r1+0x14
  s32 mstime; // r28
  u16 curve;  // r27

  time = (u16)(cstep->para[1] >> 16);

  if ((u8)(cstep->para[1] >> 8) & 1) {
    sndConvertMs(&time);
  } else {
    sndConvertTicks(&time, svoice);
  }

  mstime = sndConvert2Ms(time);
  if (mstime == 0) {
    mstime = 1;
  }

  tvol = (svoice->volume * (u8)(cstep->para[0] >> 8) >> 7);
  tvol += (u8)(cstep->para[0] >> 16) << 16;

  if (tvol > MAC_VOLUME_MAX) {
    tvol = MAC_VOLUME_MAX;
  }

  curve = (u16)(u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE);
  curve |= (((u16)(u8)cstep->para[1]) << 8);
  tvol = TranslateVolume(tvol, curve);
  svoice->envTarget = tvol;
  svoice->envCurrent = start_vol;
  svoice->envDelta = (s32)(tvol - start_vol) / mstime;
  svoice->volume = start_vol;
  svoice->cFlags |= MAC_CFLAG_ENVELOPE;
}

static void mcmdEnvelope(SYNTH_VOICE* svoice, MSTEP* cstep) {
  DoEnvelopeCalculation(svoice, cstep, svoice->volume);
}
static void mcmdFadeIn(SYNTH_VOICE* svoice, MSTEP* cstep) {
  DoEnvelopeCalculation(svoice, cstep, 0);
}

static void mcmdRandomKey(SYNTH_VOICE* svoice /* r28 */, MSTEP* cstep /* r31 */) {
  u8 k1;     // r30
  u8 k2;     // r29
  u8 t;      // r20
  s32 i1;    // r27
  s32 i2;    // r26
  u8 detune; // r25

  if (!(u8)(cstep->para[1] >> 8)) {
    k1 = (cstep->para[0] >> 8);
    k2 = (cstep->para[0] >> 24);
    if (k1 > k2) {
      t = k1;
      k1 = k2;
      k2 = t;
    }
  } else {
    i1 = svoice->curNote - (u8)(cstep->para[0] >> 8);
    i2 = svoice->curNote + (u8)(cstep->para[0] >> 24);

    k1 = i1 < 0 ? 0 : i1 > 127 ? 127 : i1;
    k2 = i2 < 0 ? 0 : i2 > 127 ? 127 : i2;
  }

  if ((u8)cstep->para[1]) {
    detune = (sndRand() % 201) - 100;
  } else {
    detune = (u8)(cstep->para[0] >> 16);
  }

  cstep->para[0] = ((u8)detune << 16) | MAC_CMD_SET_KEY | ((k1 + (sndRand() % ((k2 - k1) + 1))) * MAC_PACK_SCALE_BYTE);
  cstep->para[1] = 0;
  mcmdSetKey(svoice, cstep);
}

static void mcmdSetPitchbendAfterKeyOff(SYNTH_VOICE* svoice) { svoice->cFlags |= MAC_CFLAG_PITCHBEND_AFTER_KEYOFF; }
static void mcmdScaleReverb(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->revVolScale = (u8)(cstep->para[0] >> 8);
  svoice->revVolOffset = (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD);
}
static void SelectSource(SYNTH_VOICE* svoice, CTRL_DEST* dest, MSTEP* cstep, u64 tstflag,
                         u32 dirtyFlag) {
  u8 comb;   // r28
  s32 scale; // r30

  if (!(svoice->cFlags & tstflag)) {
    comb = 0;
    svoice->cFlags |= tstflag;
  } else {
    comb = (u8)cstep->para[1];
  }

  scale = ((s16)(cstep->para[0] >> 16) << 16) / 100;
  if (scale < 0) {
    scale -= ((s8)(cstep->para[1] >> MAC_PACK_SHIFT_HALFWORD) << 8) / 100;
  } else {
    scale += ((s8)(cstep->para[1] >> MAC_PACK_SHIFT_HALFWORD) << 8) / 100;
  }

  inpAddCtrl(dest, (u8)(cstep->para[0] >> 8), scale, comb, (u8)(cstep->para[1] >> 8) != 0);

  if ((dirtyFlag & MAC_DIRTY_GLOBAL) != 0) {
    inpSetGlobalMIDIDirtyFlag(svoice->midi, svoice->midiSet, dirtyFlag);
  } else {
    svoice->midiDirtyFlags |= dirtyFlag;
  }
}

static void mcmdVolumeSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpVolume, cstep, MAC_SOURCE_VOLUME, 1);
}

static void mcmdPanningSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpPanning, cstep, MAC_SOURCE_PANNING, 2);
}

static void mcmdPitchWheelSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpPitchBend, cstep, MAC_SOURCE_PITCH_BEND, 8);
}

static void mcmdModWheelSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpModulation, cstep, MAC_SOURCE_MODULATION, MAC_DIRTY_MODULATION);
}

static void mcmdPedalSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpPedal, cstep, MAC_SOURCE_PEDAL, MAC_DIRTY_PEDAL);
}

static void mcmdPortamentoSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpPortamento, cstep, MAC_SOURCE_PORTAMENTO, MAC_DIRTY_PORTAMENTO);
}

static void mcmdReverbSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpReverb, cstep, MAC_SOURCE_REVERB, MAC_DIRTY_REVERB);
}

static void mcmdPreAuxASelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpPreAuxA, cstep, MAC_SOURCE_PRE_AUX_A, MAC_DIRTY_PRE_AUX_A);
}

static void mcmdPreAuxBSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpPreAuxB, cstep, MAC_SOURCE_PRE_AUX_B, MAC_DIRTY_PRE_AUX_B);
}

static void mcmdPostAuxBSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpPostAuxB, cstep, MAC_SOURCE_POST_AUX_B, MAC_DIRTY_POST_AUX_B);
}

static void mcmdSurroundPanningSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpSurroundPanning, cstep, MAC_SOURCE_SURROUND_PANNING, 4);
}

static void mcmdDopplerSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpDoppler, cstep, MAC_SOURCE_DOPPLER, MAC_DIRTY_DOPPLER);
}

static void mcmdTremoloSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpTremolo, cstep, MAC_SOURCE_TREMOLO, MAC_DIRTY_TREMOLO);
}

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
static void mcmdFilterSwitchSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpFilterSwitch, cstep, MAC_FILTER_SWITCH_SOURCE_FLAG,
               MAC_FILTER_SWITCH_DIRTY_FLAG);
}

static void mcmdFilterParameterSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  SelectSource(svoice, &svoice->inpFilterParameter, cstep, MAC_FILTER_PARAMETER_SOURCE_FLAG,
               MAC_FILTER_PARAMETER_DIRTY_FLAG);
}
#endif

static void mcmdAuxAFXSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 i;                                                                     // r31
  static u64 mask[4] = {MAC_AUX_A_SOURCE_0, MAC_AUX_A_SOURCE_1, MAC_AUX_A_SOURCE_2, MAC_AUX_A_SOURCE_3}; // size: 0x20
  static u32 dirty[4] = {MAC_AUX_A_DIRTY_0, MAC_AUX_A_DIRTY_1, MAC_AUX_A_DIRTY_2, MAC_AUX_A_DIRTY_3};    // size: 0x10
  i = (u8)(cstep->para[1] >> MAC_PACK_SHIFT_HIGH_BYTE);
  SelectSource(svoice, &inpAuxA[svoice->studio][i], cstep, mask[i], dirty[i]);
}

static void mcmdAuxBFXSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 i;                                                                         // r31
  static u64 mask[4] = {MAC_AUX_B_SOURCE_0, MAC_AUX_B_SOURCE_1, MAC_AUX_B_SOURCE_2, MAC_AUX_B_SOURCE_3}; // size: 0x20
  static u32 dirty[4] = {MAC_AUX_B_DIRTY_0, MAC_AUX_B_DIRTY_1, MAC_AUX_B_DIRTY_2, MAC_AUX_B_DIRTY_3};        // size: 0x10
  i = (u8)(cstep->para[1] >> MAC_PACK_SHIFT_HIGH_BYTE);
  SelectSource(svoice, &inpAuxB[svoice->studio][i], cstep, mask[i], dirty[i]);
}

static void mcmdPortamento(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 time; // r1+0x10
  svoice->portType = cstep->para[0] >> 16;
  time = (u16)(cstep->para[1] >> 16);
  if (((u8)(cstep->para[1] >> 8) & 1)) {
    sndConvertMs(&time);
  } else {
    sndConvertTicks(&time, svoice);
  }

  svoice->portDuration = time;

  switch ((u8)(cstep->para[0] >> 8)) {
  case 0:
    if (svoice->midi != MAC_MIDI_NONE) {
      inpSetMidiCtrl(MAC_MIDI_CTRL_PORTAMENTO, svoice->midi, svoice->midiSet, 0);
    }

    svoice->cFlags &= ~MAC_CFLAG_PORTAMENTO;
    return;
  case 1:
    if (svoice->midi != MAC_MIDI_NONE) {
      inpSetMidiCtrl(MAC_MIDI_CTRL_PORTAMENTO, svoice->midi, svoice->midiSet, MAC_PACK_MASK_7BIT);
    }
  init_port:
    if (!(svoice->cFlags & MAC_CFLAG_PORTAMENTO)) {
      synthInitPortamento(svoice);
    }
    svoice->cFlags |= MAC_CFLAG_PORTAMENTO;
    break;
  case 2:
    if (svoice->midi != MAC_MIDI_NONE && inpGetMidiCtrl(MAC_MIDI_CTRL_PORTAMENTO, svoice->midi, svoice->midiSet) > 8064) {
      goto init_port;
    }
    break;
  }
}

s32 varGet32(SYNTH_VOICE* svoice, u32 ctrl, u8 index) {
  if (ctrl != 0) {
    return inpGetExCtrl(svoice, index);
  }

  index &= MAC_VAR_INDEX_MASK;
  return index < 16 ? svoice->local_vars[index] : synthGlobalVariable[index - 16];
}

s16 varGet(SYNTH_VOICE* svoice, u32 ctrl, u8 index) { return varGet32(svoice, ctrl, index); }

void varSet32(SYNTH_VOICE* svoice, u32 ctrl, u8 index, s32 v) {
  if (ctrl != 0) {
    inpSetExCtrl(svoice, index, v);
    return;
  }
  index &= MAC_VAR_INDEX_MASK;

  if (index < 16) {
    svoice->local_vars[index] = v;
    return;
  }

  synthGlobalVariable[index - 16] = v;
}
void varSet(SYNTH_VOICE* svoice, u32 ctrl, u8 index, s16 v) { varSet32(svoice, ctrl, index, v); }

static void mcmdVarCalculation(SYNTH_VOICE* svoice, MSTEP* cstep, u8 op) {
  s16 s1; // r28
  s16 s2; // r31
  s32 t;  // r30

  s1 = varGet(svoice, (u8)(cstep->para[0] >> 24), cstep->para[1]);
  if (op == 4) {
    s2 = cstep->para[1] >> 8;
  } else {
    s2 = varGet(svoice, (u8)(cstep->para[1] >> 8), cstep->para[1] >> 16);
  }
  switch (op) {
  case 4:
  case 0:
    t = (s1 + s2);
    break;
  case 1:
    t = (s1 - s2);
    break;
  case 2:
    t = (s1 * s2);
    break;
  case 3:
    t = s2 != 0 ? (s1 / s2) : 0;
    break;
  }

  varSet(svoice, (u8)(cstep->para[0] >> 8), (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD),
         (t < MAC_SIGNED_16BIT_MIN  ? MAC_SIGNED_16BIT_MIN
          : t > MAC_SIGNED_16BIT_MAX ? MAC_SIGNED_16BIT_MAX
                       : t));
}

static void mcmdSetVarImmediate(SYNTH_VOICE* svoice, MSTEP* cstep) {
  varSet(svoice, (u8)(cstep->para[0] >> 8), (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD), (s16)cstep->para[1]);
}

static void mcmdIfVarCompare(SYNTH_VOICE* svoice, MSTEP* cstep, u8 cmp) {
  s32 a;     // r28
  s32 b;     // r27
  u8 result; // r30

  a = varGet32(svoice, (u8)(cstep->para[0] >> 8), (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD));
  b = varGet32(svoice, (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HIGH_BYTE), (u8)cstep->para[1]);

  switch (cmp) {
  case 0:
    result = !(b - a);
    break;
  case 1:
    result = (a < b);
    break;
  }

  if ((u8)(cstep->para[1] >> 8) != 0) {
    result = !result;
  }
  if ((u8)result != 0) {
    svoice->curAddr = svoice->addr + (u16)(cstep->para[1] >> MAC_PACK_SHIFT_HALFWORD);
  }
}
bool macPostMessage(u32 vid, s32 mesg) {
  SYNTH_VOICE* sv; // r31
  if ((vid = vidGetInternalId(vid)) != -1 && (sv = &synthVoice[vid & MAC_PACK_MASK_BYTE])->mesgNum < 4) {
    ++sv->mesgNum;
    sv->mesgQueue[sv->mesgWrite] = mesg;
    sv->mesgWrite = (sv->mesgWrite + 1) & 3;
    ExecuteTrap(sv, 2);
    return 1;
  }

  return 0;
}
static void mcmdSendMessage(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u8 i;      // r31
  s32 mesg;  // r30
  u16 macro; // r28

  mesg = varGet32(svoice, 0, (u8)(cstep->para[1] >> 8));

  if (!(u8)(cstep->para[0] >> 8)) {
    macro = (u16)(cstep->para[0] >> 16);
    if (macro != MAC_MESSAGE_MACRO_NONE) {
      for (i = 0; i < synthInfo.voiceNum; ++i) {
        if (synthVoice[i].addr != NULL && macro == synthVoice[i].macroId) {
          macPostMessage(synthVoice[i].vidList->vid, mesg);
        }
      }
    } else if (synthMessageCallback != NULL) {
      synthMessageCallback(svoice->vidList->vid, mesg);
    }
  } else {
    macPostMessage(varGet32(svoice, 0, (u8)cstep->para[1]), mesg);
  }
}

static void mcmdGetMessage(SYNTH_VOICE* svoice, MSTEP* cstep) {
  s32 mesg; // r30
  mesg = 0;
  if (svoice->mesgNum != '\0') {
    mesg = svoice->mesgQueue[svoice->mesgRead];
    svoice->mesgRead = (svoice->mesgRead + 1) & 3;
    --svoice->mesgNum;
  }
  varSet32(svoice, 0, (u8)(cstep->para[0] >> 8), mesg);
}

static void mcmdGetVID(SYNTH_VOICE* svoice, MSTEP* cstep) {
  if ((u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD) == 0) {
    varSet32(svoice, 0, (u8)(cstep->para[0] >> 8), svoice->vidList->vid);
  } else {
    varSet32(svoice, 0, (u8)(cstep->para[0] >> 8), svoice->lastVID);
  }
}
static void mcmdModeSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->volTable = (u8)(cstep->para[0] >> 8) ? TRUE : FALSE;
  svoice->itdMode = (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD) ? FALSE : TRUE;
}
static void mcmdSRCModeSelect(SYNTH_VOICE* svoice, MSTEP* cstep) {
  hwSetSRCType(svoice->id & MAC_PACK_MASK_BYTE, (u8)(cstep->para[0] >> 8));
  hwSetPolyPhaseFilter(svoice->id & MAC_PACK_MASK_BYTE, (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD));
  svoice->cFlags |= MAC_CFLAG_SRC_MODE;
}
static void mcmdSetKeyGroup(SYNTH_VOICE* svoice, MSTEP* cstep) {
  u32 i;    // r31
  u8 kg;    // r30
  u32 kill; // r29

  svoice->keyGroup = 0;
  kg = (u8)(cstep->para[0] >> 8);
  kill = (u8)(cstep->para[0] >> MAC_PACK_SHIFT_HALFWORD) != 0;

  if (kg) {
    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (synthVoice[i].addr != NULL && (synthVoice[i].cFlags & MAC_CFLAG_VOICE_ALLOCATED) == 0 &&
          kg == synthVoice[i].keyGroup) {
        if (!kill) {
          macSetExternalKeyoff(&synthVoice[i]);
        } else {
          voiceKill(i);
        }
      }
    }
    svoice->keyGroup = kg;
  }
}
static void mcmdSetupTremolo(SYNTH_VOICE* svoice, MSTEP* cstep) {
  svoice->treScale = (cstep->para[0] >> 8);
  svoice->treModAddScale = cstep->para[1];
  svoice->treCurScale = 1.f;
}

static void macHandleActive(SYNTH_VOICE* svoice) {
  u8 i;                              // r29
  u8 lastNote;                       // r27
  u32 ex;                            // r30
  CHANNEL_DEFAULTS* channelDefaults; // r28
  static MSTEP cstep;

  if (svoice->cFlags & 3) {
    if (svoice->cFlags & 1) {
      svoice->cFlags &= ~1;
      hwBreak(svoice->id & MAC_PACK_MASK_BYTE);
    }

    svoice->panning[0] = svoice->panTarget[0] = (u32)(svoice->setup.pan) << 16;
    svoice->panning[1] = svoice->panTarget[1] = 0;
    svoice->volume = (u32)(svoice->setup.vol << 16);
    svoice->volTable = 0;
    svoice->orgVolume = svoice->volume;
    svoice->midi = svoice->setup.midi;
    svoice->midiSet = svoice->setup.midiSet;
    svoice->section = svoice->setup.section;
    svoice->track = svoice->setup.track;
    svoice->itdMode = svoice->setup.itdMode;
    svoice->keyGroup = 0;
    svoice->vibModAddScale = 0;
    svoice->treScale = 0;
    inpInit(svoice);
    if ((lastNote = inpGetMidiLastNote(svoice->midi, svoice->midiSet)) != MAC_MIDI_NONE) {
      svoice->lastNote = lastNote;
    } else {
      svoice->lastNote = svoice->orgNote;
    }

    inpSetMidiLastNote(svoice->midi, svoice->midiSet, svoice->orgNote);
    voiceSetLastStarted(svoice);
    svoice->vGroup = svoice->setup.vGroup;
    svoice->studio = svoice->setup.studio;
    svoice->portTime = 0;
    svoice->portDuration = 25600;
    svoice->portType = 0;
    if (svoice->midi != MAC_MIDI_NONE) {
      svoice->portLastCtrlState = inpGetMidiCtrl(65, svoice->midi, svoice->midiSet);
    } else {
      svoice->portLastCtrlState = 0;
    }
    channelDefaults = inpGetChannelDefaults(svoice->midi, svoice->midiSet);
    svoice->pbLowerKeyRange = channelDefaults->pbRange;
    svoice->pbUpperKeyRange = channelDefaults->pbRange;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
    svoice->lpfLowerFrqBoundary = channelDefaults->lpfLowerFrqBoundary;
    svoice->lpfUpperFrqBoundary = channelDefaults->lpfUpperFrqBoundary;
#endif
    svoice->revVolScale = 128;
    svoice->revVolOffset = 0;
    svoice->loop = 0;
    svoice->sweepNum[0] = 0;
    svoice->sweepNum[1] = 0;
    svoice->sweepOff[0] = 0;
    svoice->sweepOff[1] = 0;
    svoice->lfo[0].period = 0;
    svoice->lfo[0].value = 0;
    svoice->lfo[0].lastValue = MAC_SIGNED_16BIT_MAX;
    svoice->lfo[1].period = 0;
    svoice->lfo[1].value = 0;
    svoice->lfo[1].lastValue = MAC_SIGNED_16BIT_MAX;

    for (i = 0; i < 3; ++i) {
      svoice->trapEventAddr[i] = NULL;
    }

    svoice->trapEventAny = 0;
    svoice->sInfo = -1;
    svoice->playFrq = -1;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
    svoice->sampleId = MAC_SAMPLE_ID_NONE;
#endif
    svoice->pbLast = MAC_PITCHBEND_CENTER;
    svoice->curOutputVolume = 0;
    svoice->cFlags &= 8;
    svoice->cFlags |= MAC_CFLAG_INITIALIZED;
    memset(svoice->local_vars, 0, sizeof(svoice->local_vars));
    svoice->waitTime = macRealTime;
    svoice->macStartTime = macRealTime;
    synthStartSynthJobHandling(svoice);
  }

  DebugMacroSteps = 0;

  do {
    if (++DebugMacroSteps > 32) {
      break;
    }

    cstep.para[0] = svoice->curAddr->para[0];
    cstep.para[1] = svoice->curAddr->para[1];
    ++svoice->curAddr;
    ex = 0;
    switch (cstep.para[0] & MAC_PACK_MASK_7BIT) {
    case MAC_CMD_END_OF_MACRO:
      ex = mcmdEndOfMacro(svoice);
      break;
    case MAC_CMD_STOP:
      ex = mcmdStop(svoice);
      break;
    case MAC_CMD_IF_KEY:
      mcmdIfKey(svoice, &cstep);
      break;
    case MAC_CMD_IF_VELOCITY:
      mcmdIfVelocity(svoice, &cstep);
      break;
    case MAC_CMD_WAIT:
      ex = mcmdWait(svoice, &cstep);
      break;
    case MAC_CMD_LOOP:
      mcmdLoop(svoice, &cstep);
      break;
    case MAC_CMD_GOTO:
      ex = mcmdGoto(svoice, &cstep);
      break;
    case MAC_CMD_WAIT_MS:
      ex = mcmdWaitMs(svoice, &cstep);
      break;
    case MAC_CMD_PLAY_MACRO:
      mcmdPlayMacro(svoice, &cstep);
      break;
    case MAC_CMD_SEND_KEY_OFF:
      mcmdSendKeyOff(svoice, &cstep);
      break;
    case MAC_CMD_IF_MODULATION:
      mcmdIfModulation(svoice, &cstep);
      break;
    case MAC_CMD_SET_PIANO_PANNING:
      mcmdSetPianoPanning(svoice, &cstep);
      break;
    case MAC_CMD_SET_ADSR:
      mcmdSetADSR(svoice, &cstep);
      break;
    case MAC_CMD_SCALE_VOLUME:
      mcmdScaleVolume(svoice, &cstep);
      break;
    case MAC_CMD_SET_PANNING:
      mcmdSetPanning(svoice, &cstep);
      break;
    case MAC_CMD_ENVELOPE:
      mcmdEnvelope(svoice, &cstep);
      break;
    case MAC_CMD_START_SAMPLE:
      mcmdStartSample(svoice, &cstep);
      break;
    case MAC_CMD_STOP_SAMPLE:
      mcmdStopSample(svoice);
      break;
    case MAC_CMD_KEY_OFF:
      mcmdKeyOff(svoice);
      break;
    case MAC_CMD_IF_RANDOM:
      mcmdIfRandom(svoice, &cstep);
      break;
    case MAC_CMD_FADE_IN:
      mcmdFadeIn(svoice, &cstep);
      break;
    case MAC_CMD_SET_SURROUND_PANNING:
      mcmdSetSurroundPanning(svoice, &cstep);
      break;
    case MAC_CMD_SET_ADSR_FROM_CTRL:
      mcmdSetADSRFromCtrl(svoice, &cstep);
      break;
    case MAC_CMD_RANDOM_KEY:
      mcmdRandomKey(svoice, &cstep);
      break;
    case MAC_CMD_ADD_KEY:
      ex = mcmdAddKey(svoice, &cstep);
      break;
    case MAC_CMD_SET_KEY:
      ex = mcmdSetKey(svoice, &cstep);
      break;
    case MAC_CMD_LAST_KEY:
      ex = mcmdLastKey(svoice, &cstep);
      break;
    case MAC_CMD_PORTAMENTO:
      mcmdPortamento(svoice, &cstep);
      break;
    case MAC_CMD_VIBRATO:
      mcmdVibrato(svoice, &cstep);
      break;
    case MAC_CMD_PITCH_SWEEP_UP:
      ex = mcmdPitchSweep(svoice, &cstep, 0);
      break;
    case MAC_CMD_PITCH_SWEEP_DOWN:
      ex = mcmdPitchSweep(svoice, &cstep, 1);
      break;
    case MAC_CMD_SET_PITCH:
      mcmdSetPitch(svoice, &cstep);
      break;
    case MAC_CMD_SET_PITCH_ADSR:
      mcmdSetPitchADSR(svoice, &cstep);
      break;
    case MAC_CMD_SCALE_VOLUME_DLS:
      mcmdScaleVolumeDLS(svoice, &cstep);
      break;
    case MAC_CMD_SET_MOD_VIBRATO:
      mcmdSetMod2Vibrato(svoice, &cstep);
      break;
    case MAC_CMD_SETUP_TREMOLO:
      mcmdSetupTremolo(svoice, &cstep);
      break;
    case MAC_CMD_RETURN:
      mcmdReturn(svoice);
      break;
    case MAC_CMD_GOSUB:
      ex = mcmdGosub(svoice, &cstep);
      break;
    case MAC_CMD_TRAP_EVENT:
      mcmdTrapEvent(svoice, &cstep);
      break;
    case MAC_CMD_UNTRAP_EVENT:
      mcmdUntrapEvent(svoice, &cstep);
      break;
    case MAC_CMD_SEND_MESSAGE:
      mcmdSendMessage(svoice, &cstep);
      break;
    case MAC_CMD_GET_MESSAGE:
      mcmdGetMessage(svoice, &cstep);
      break;
    case MAC_CMD_GET_VID:
      mcmdGetVID(svoice, &cstep);
      break;
    case MAC_CMD_ADD_AGE_COUNTER:
      mcmdAddAgeCounter(svoice, &cstep);
      break;
    case MAC_CMD_SET_AGE_COUNTER:
      mcmdSetAgeCounter(svoice, &cstep);
      break;
    case MAC_CMD_SEND_FLAG:
      mcmdSendFlag(&cstep);
      break;
    case MAC_CMD_SET_PITCH_WHEEL_RANGE:
      mcmdSetPitchWheelRange(svoice, &cstep);
      break;
    case MAC_CMD_SCALE_REVERB:
      mcmdScaleReverb(svoice, &cstep);
      break;
    case MAC_CMD_SET_PITCHBEND_AFTER_KEYOFF:
      mcmdSetPitchbendAfterKeyOff(svoice);
      break;
    case MAC_CMD_SET_PRIORITY:
      mcmdSetPriority(svoice, &cstep);
      break;
    case MAC_CMD_ADD_PRIORITY:
      mcmdAddPriority(svoice, &cstep);
      break;
    case MAC_CMD_SET_AGE_COUNTER_SPEED:
      mcmdSetAgeCounterSpeed(svoice, &cstep);
      break;
    case MAC_CMD_SET_AGE_COUNTER_BY_VOLUME:
      mcmdSetAgeCounterByVolume(svoice, &cstep);
      break;
    case MAC_CMD_VOLUME_SELECT:
      mcmdVolumeSelect(svoice, &cstep);
      break;
    case MAC_CMD_PANNING_SELECT:
      mcmdPanningSelect(svoice, &cstep);
      break;
    case MAC_CMD_PITCH_WHEEL_SELECT:
      mcmdPitchWheelSelect(svoice, &cstep);
      break;
    case MAC_CMD_MOD_WHEEL_SELECT:
      mcmdModWheelSelect(svoice, &cstep);
      break;
    case MAC_CMD_PEDAL_SELECT:
      mcmdPedalSelect(svoice, &cstep);
      break;
    case MAC_CMD_PORTAMENTO_SELECT:
      mcmdPortamentoSelect(svoice, &cstep);
      break;
    case MAC_CMD_REVERB_SELECT:
      mcmdReverbSelect(svoice, &cstep);
      break;
    case MAC_CMD_SURROUND_PANNING_SELECT:
      mcmdSurroundPanningSelect(svoice, &cstep);
      break;
    case MAC_CMD_DOPPLER_SELECT:
      mcmdDopplerSelect(svoice, &cstep);
      break;
    case MAC_CMD_TREMOLO_SELECT:
      mcmdTremoloSelect(svoice, &cstep);
      break;
    case MAC_CMD_PRE_AUX_A_SELECT:
      mcmdPreAuxASelect(svoice, &cstep);
      break;
    case MAC_CMD_PRE_AUX_B_SELECT:
      mcmdPreAuxBSelect(svoice, &cstep);
      break;
    case MAC_CMD_POST_AUX_B_SELECT:
      mcmdPostAuxBSelect(svoice, &cstep);
      break;
    case MAC_CMD_AUX_AFX_SELECT:
      mcmdAuxAFXSelect(svoice, &cstep);
      break;
    case MAC_CMD_AUX_BFX_SELECT:
      mcmdAuxBFXSelect(svoice, &cstep);
      break;
    case MAC_CMD_SETUP_LFO:
      mcmdSetupLFO(svoice, &cstep);
      break;
    case MAC_CMD_MODE_SELECT:
      mcmdModeSelect(svoice, &cstep);
      break;
    case MAC_CMD_SET_KEY_GROUP:
      mcmdSetKeyGroup(svoice, &cstep);
      break;
    case MAC_CMD_SRC_MODE_SELECT:
      mcmdSRCModeSelect(svoice, &cstep);
      break;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
    case MAC_CMD_FILTER_SWITCH:
      mcmdFilterSwitchSelect(svoice, &cstep);
      break;
    case MAC_CMD_FILTER_PARAMETER:
      mcmdFilterParameterSelect(svoice, &cstep);
      break;
#endif
    case MAC_CMD_VAR_ADD:
      mcmdVarCalculation(svoice, &cstep, 0);
      break;
    case MAC_CMD_VAR_SUB:
      mcmdVarCalculation(svoice, &cstep, 1);
      break;
    case MAC_CMD_VAR_MUL:
      mcmdVarCalculation(svoice, &cstep, 2);
      break;
    case MAC_CMD_VAR_DIV:
      mcmdVarCalculation(svoice, &cstep, 3);
      break;
    case MAC_CMD_VAR_ADD_IMMEDIATE:
      mcmdVarCalculation(svoice, &cstep, 4);
      break;
    case MAC_CMD_SET_VAR_IMMEDIATE:
      mcmdSetVarImmediate(svoice, &cstep);
      break;
    case MAC_CMD_IF_VAR_EQUAL:
      mcmdIfVarCompare(svoice, &cstep, 0);
      break;
    case MAC_CMD_IF_VAR_LESS:
      mcmdIfVarCompare(svoice, &cstep, 1);
      break;
    }
  } while (!ex);
}

void macHandle(u32 deltaTime) {
  SYNTH_VOICE* sv;     // r31
  SYNTH_VOICE* nextSv; // r30
  u64 w;               // r28

  for (sv = macTimeQueueRoot; sv != NULL && sv->wait <= macRealTime;) {
    nextSv = sv->nextTimeQueueMacro;
    w = sv->wait;
    macMakeActive(sv);
    sv->waitTime = w;
    sv = nextSv;
  }

  for (sv = macActiveMacroRoot; sv != NULL; sv = sv->nextMacActive) {
    nextSv = sv->nextMacActive;
    if (HasHWEventTrap(sv) != 0) {
      CheckHWEventTrap(sv);
    }
    macHandleActive(sv);
  }
  macRealTime += deltaTime;
}

void macSampleEndNotify(SYNTH_VOICE* sv) {
  if (sv->macState != MAC_STATE_YIELDED) {
    return;
  }
  /* clang-format off */
  MUSY_ASSERT(sv->addr!=NULL);
  /* clang-format on */

  if (!ExecuteTrap(sv, 1) && (sv->cFlags & MAC_CFLAG_HW_EVENT_WAIT)) {
    macMakeActive(sv);
  }
}
void macSetExternalKeyoff(SYNTH_VOICE* sv) {
  sv->cFlags |= 8;
  if (!sv->addr) {
    return;
  }

  if (!(sv->cFlags & MAC_CFLAG_PEDAL)) {
    if (!ExecuteTrap(sv, 0) && (sv->cFlags & MAC_CFLAG_WAITING)) {
      macMakeActive(sv);
    }
  } else {
    sv->cFlags |= MAC_CFLAG_TRAP_PENDING;
  }
}

void macSetPedalState(SYNTH_VOICE* svoice, u32 state) {
  if (state != 0) {
    svoice->cFlags |= MAC_CFLAG_PEDAL;
  } else {
    if (svoice->addr && (svoice->cFlags & MAC_CFLAG_TRAP_PENDING)) {
      if (!ExecuteTrap(svoice, 0) && (svoice->cFlags & MAC_CFLAG_WAITING)) {
        macMakeActive(svoice);
      }
    }

    svoice->cFlags &= ~(MAC_CFLAG_PEDAL | MAC_CFLAG_TRAP_PENDING);
  }
}

static void TimeQueueAdd(SYNTH_VOICE* svoice) {
  SYNTH_VOICE* sv;     // r31
  SYNTH_VOICE* lastSv; // r30

  lastSv = NULL;
  for (sv = macTimeQueueRoot; sv != NULL && sv->wait < svoice->wait;) {
    lastSv = sv;
    sv = sv->nextTimeQueueMacro;
  }

  if (sv == NULL) {
    if (lastSv == NULL) {
      macTimeQueueRoot = svoice;
      svoice->nextTimeQueueMacro = NULL;
      svoice->prevTimeQueueMacro = NULL;
    } else {
      lastSv->nextTimeQueueMacro = svoice;
      svoice->prevTimeQueueMacro = lastSv;
      svoice->nextTimeQueueMacro = NULL;
    }
  } else {
    svoice->nextTimeQueueMacro = sv;
    if (svoice->prevTimeQueueMacro = sv->prevTimeQueueMacro) {
      sv->prevTimeQueueMacro->nextTimeQueueMacro = svoice;
    } else {
      macTimeQueueRoot = svoice;
    }
    sv->prevTimeQueueMacro = svoice;
  }
}
static void UnYieldMacro(SYNTH_VOICE* svoice, bool disableUpdate) {
  if (svoice->wait != 0) {
    if (svoice->wait != -1) {
      if (svoice->prevTimeQueueMacro == NULL) {
        macTimeQueueRoot = svoice->nextTimeQueueMacro;
      } else {
        svoice->prevTimeQueueMacro->nextTimeQueueMacro = svoice->nextTimeQueueMacro;
      }

      if (svoice->nextTimeQueueMacro) {
        svoice->nextTimeQueueMacro->prevTimeQueueMacro = svoice->prevTimeQueueMacro;
      }
    }

    if (!disableUpdate) {
      synthForceLowPrecisionUpdate(svoice);
    }

    svoice->wait = 0;
    svoice->waitTime = macRealTime;
    svoice->cFlags &= ~MAC_CFLAG_UNYIELD_MASK;
  }
}
void macMakeActive(SYNTH_VOICE* sv) {
  if (sv->macState == MAC_STATE_RUNNABLE) {
    return;
  }
  /* clang-format off */
  MUSY_ASSERT(sv->addr!=NULL);
  /* clang-format on */
  UnYieldMacro(sv, 0);
  if (sv->nextMacActive = macActiveMacroRoot) {
    macActiveMacroRoot->prevMacActive = sv;
  }
  sv->prevMacActive = NULL;
  macActiveMacroRoot = sv;
  sv->macState = MAC_STATE_RUNNABLE;
}

void macMakeInactive(SYNTH_VOICE* svoice, MAC_STATE newState) {
  if (svoice->macState == newState) {
    return;
  }

  /* clang-format off */
  MUSY_ASSERT(svoice->addr!=NULL);
  /* clang-format on */
  if (svoice->macState == MAC_STATE_RUNNABLE) {
    if (svoice->prevMacActive == NULL) {
      macActiveMacroRoot = svoice->nextMacActive;
    } else {
      svoice->prevMacActive->nextMacActive = svoice->nextMacActive;
    }

    if (svoice->nextMacActive != NULL) {
      svoice->nextMacActive->prevMacActive = svoice->prevMacActive;
    }
  }

  if (newState == MAC_STATE_STOPPED) {
    UnYieldMacro(svoice, 1);
  }
  svoice->macState = newState;
}

u32 macStart(u16 macid, u8 priority, u8 maxVoices,
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
             u32 allocId,
#else
             u16 allocId,
#endif
             u8 key, u8 vol, u8 panning, u8 midi, u8 midiSet, u8 section, u16 step, u16 trackid,
             u8 new_vid, u8 vGroup, u8 studio, u32 itd) {
  u32 voice;           // r30
  u32 vid;             // r25
  MSTEP* addr;         // r28
  SYNTH_VOICE* svoice; // r31
  u16 seqPrio;         // r24

  if ((addr = dataGetMacro(macid))) {
    if (!(key & MAC_VOICE_KEY_FX) && (seqPrio = seqGetMIDIPriority(midiSet, midi)) != MAC_PRIORITY_NONE) {
      priority = seqPrio;
    }

    if ((voice = voiceAllocate(priority, maxVoices, allocId, (key & MAC_VOICE_KEY_FX) ? 1 : 0)) != -1) {
      svoice = &synthVoice[voice];
      vidRemoveVoiceReferences(svoice);
      macMakeInactive(svoice, MAC_STATE_STOPPED);
      svoice->cFlags = (svoice->cFlags & MAC_CFLAG_PERSISTENT) | 2;

      if (hwIsActive(voice)) {
        svoice->cFlags |= 1;
      }

      svoice->wait = 0;

      if ((key & MAC_VOICE_KEY_FX) != 0) {
        svoice->fxFlag = 01;
        key &= MAC_PACK_MASK_7BIT;
        inpResetMidiCtrl(voice, MAC_MIDI_NONE, 1);
        inpResetChannelDefaults(voice, MAC_MIDI_NONE);
        svoice->setup.midi = voice;
        svoice->setup.midiSet = MAC_MIDI_NONE;
        svoice->setup.section = 0;
      } else {
        svoice->fxFlag = 0;
        svoice->setup.midi = midi;
        svoice->setup.midiSet = midiSet;
        svoice->setup.section = section;
      }

      svoice->macroId = macid;
      svoice->allocId = allocId;
      svoice->age = MAC_AGE_INITIAL;
      svoice->ageSpeed = MAC_AGE_SPEED_INITIAL;
      svoice->addr = addr;
      svoice->curAddr = addr + step;
      svoice->orgNote = key;
      svoice->curNote = key;
      svoice->curDetune = 0;
      svoice->setup.vol = vol;
      svoice->setup.pan = panning;
      svoice->setup.track = trackid;
      svoice->callStackEntryNum = 0;
      svoice->callStackIndex = 0;
      svoice->child = -1;
      svoice->parent = -1;
      svoice->lastVID = -1;
      svoice->setup.vGroup = vGroup;
      svoice->setup.studio = studio;
      svoice->setup.itdMode = itd != 0 ? 0 : 1;
      svoice->mesgNum = svoice->mesgRead = svoice->mesgWrite = 0;
      svoice->id = voice | ((macid << 16) | (key << 8));
      voiceSetPriority(svoice, priority);

      if ((vid = vidMakeNew(svoice, new_vid)) != -1) {
        macMakeActive(svoice);
        return vid;
      }

      if (hwIsActive(voice)) {
        hwBreak(voice);
      }

      voiceFree(svoice);
    }
  }

  return -1;
}

void macInit() {
  u32 i; // r31

  macActiveMacroRoot = 0;
  macTimeQueueRoot = 0;
  macRealTime = 0;
  for (i = 0; i < synthInfo.voiceNum; ++i) {
    synthVoice[i].addr = NULL;
    synthVoice[i].macState = MAC_STATE_STOPPED;
    synthVoice[i].loop = 0;
  }
}
