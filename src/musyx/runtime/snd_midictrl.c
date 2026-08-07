
/*















*/

#include "musyx/assert.h"
#include "musyx/sal.h"
#include "musyx/seq.h"
#include "musyx/synth.h"

#include <string.h>

#define SYNTH_FX_MIDISET 255
#define MIDI_CTRL_DEFAULT_ZERO 0
#define MIDI_CTRL_DEFAULT_CENTER 64
#define MIDI_CTRL_DEFAULT_MAX 127
#define MIDI_CTRL_WARM_UNCHANGED 255
#define INPUT_SOURCE_VARIABLE 16
#define INPUT_COMBINE_MASK 15
#define INPUT_SIGNED_CENTER 8192
#define INPUT_SIGNED_MAX 8191
#define INPUT_VALUE_MAX 16383
#define INPUT_FRACTION_BITS 14
#define INPUT_FILTER_SWITCH_DIRTY 8192
#define INPUT_FILTER_PARAMETER_DIRTY 16384
#define INPUT_DIRTY_ALL 32767
#define INPUT_FILTER_SWITCH_CTRL 79
#define INPUT_FILTER_PARAMETER_CTRL 31
#define INPUT_DEFAULT_SCALE 65536
#define INPUT_GLOBAL_DIRTY_ALL 255
#define INPUT_MIDI_VALUE_MASK 127
#define INPUT_RPN_LPF_LOWER 32637
#define INPUT_RPN_LPF_UPPER 32638
#define INPUT_LPF_FREQUENCY_MASK 511
#define INPUT_DIRTY_LEGACY 8191
#define INPUT_LPF_LOW_DEFAULT 80
#define INPUT_LPF_HIGH_DEFAULT 16000
#define INPUT_CTRL_LSB_MASK 31
#define INPUT_CTRL_LSB_OFFSET 32
#define INPUT_SWITCH_CTRL_LIMIT 70
#define INPUT_RPN_CTRL_FIRST 96
#define INPUT_RPN_CTRL_LIMIT 102
#define INPUT_MIDI_PAIR_MASK 254
#define INPUT_DIRTY_VOLUME 1
#define INPUT_DIRTY_PANNING 2
#define INPUT_DIRTY_SURROUND_PANNING 4
#define INPUT_DIRTY_PITCH_BEND 8
#define INPUT_DIRTY_DOPPLER 16
#define INPUT_DIRTY_MODULATION 32
#define INPUT_DIRTY_PEDAL 64
#define INPUT_DIRTY_PRE_AUX_A 256
#define INPUT_DIRTY_REVERB 512
#define INPUT_DIRTY_PRE_AUX_B 1024
#define INPUT_DIRTY_POST_AUX_B 2048
#define INPUT_DIRTY_TREMOLO 4096
#define INPUT_RUNTIME_LFO_0 160
#define INPUT_RUNTIME_LFO_1 161
#define INPUT_RUNTIME_NOTE 162
#define INPUT_RUNTIME_VOLUME 163
#define INPUT_RUNTIME_TIME 164
#define INPUT_TRANSLATE_CTRL_PITCH_BEND 128
#define INPUT_TRANSLATE_CTRL_PITCH_BEND_FINE 129
#define INPUT_TRANSLATE_CTRL_LFO_0 130
#define INPUT_TRANSLATE_CTRL_LFO_1 131
#define INPUT_TRANSLATE_CTRL_SURROUND_PANNING 132
#define INPUT_TRANSLATE_CTRL_DOPPLER 133
#define INPUT_TRANSLATE_CTRL_NOTE 134
#define INPUT_TRANSLATE_CTRL_VOLUME 135
#define INPUT_TRANSLATE_CTRL_TIME 136
#define INPUT_AUX_A_DIRTY_STUDIO_0 ((u32)2147483649UL)
#define INPUT_AUX_A_DIRTY_STUDIO_1 ((u32)2147483650UL)
#define INPUT_AUX_A_DIRTY_STUDIO_2 ((u32)2147483652UL)
#define INPUT_AUX_A_DIRTY_STUDIO_3 ((u32)2147483656UL)
#define INPUT_AUX_B_DIRTY_STUDIO_0 ((u32)2147483664UL)
#define INPUT_AUX_B_DIRTY_STUDIO_1 ((u32)2147483680UL)
#define INPUT_AUX_B_DIRTY_STUDIO_2 ((u32)2147483712UL)
#define INPUT_AUX_B_DIRTY_STUDIO_3 ((u32)2147483776UL)

static u8 midi_lastNote[8][16];

static u8 fx_lastNote[64];

static u8 midi_ctrl[8][16][134];

static u8 fx_ctrl[64][134];

static u32 inpGlobalMIDIDirtyFlags[8][16];

static CHANNEL_DEFAULTS inpChannelDefaults[8][16];

static CHANNEL_DEFAULTS inpFXChannelDefaults[64];

static inline bool GetGlobalFlagSet(u8 chan, u8 midiSet, s32 flag) {
  return (flag & inpGlobalMIDIDirtyFlags[midiSet][chan]) != 0;
}

/*






*/
static void inpResetGlobalMIDIDirtyFlags() {
  u32 i, j;
  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 16; ++j) {
      inpGlobalMIDIDirtyFlags[i][j] = INPUT_GLOBAL_DIRTY_ALL;
    }
  }
}

static u32 inpResetGlobalMIDIDirtyFlag(u8 chan, u8 midiSet, u32 flag) {
  u32 ret;
  // clang-format off
  MUSY_ASSERT(midiSet!=SYNTH_FX_MIDISET);
  // clang-format on
  ret = (flag & inpGlobalMIDIDirtyFlags[midiSet][chan]) != 0;
  if (ret != 0) {
    inpGlobalMIDIDirtyFlags[midiSet][chan] &= ~flag;
  }
  return ret;
}

void inpSetGlobalMIDIDirtyFlag(u8 chan, u8 midiSet, s32 flag) {
  // clang-format off
  MUSY_ASSERT(midiSet!=SYNTH_FX_MIDISET);
  // clang-format on
  inpGlobalMIDIDirtyFlags[midiSet][chan] |= flag;
}

void inpSetRPNHi(u8 set, u8 channel, u8 value) {
  u16 rpn;  // r28
  u32 i;    // r31
  u8 range; // r29

  rpn = (midi_ctrl[set][channel][100]) | (midi_ctrl[set][channel][101] << 8);
  switch (rpn) {
  case 0:
    range = value > 24 ? 24 : value;
    inpChannelDefaults[set][channel].pbRange = range;

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].pbUpperKeyRange = range;
        synthVoice[i].pbLowerKeyRange = range;
      }
    }
    break;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
  case INPUT_RPN_LPF_LOWER: {
    u32 frq = (value << 9) | (inpChannelDefaults[set][channel].lpfLowerFrqBoundary & INPUT_LPF_FREQUENCY_MASK);
    inpChannelDefaults[set][channel].lpfLowerFrqBoundary = frq;

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].lpfLowerFrqBoundary = frq;
      }
    }
  } break;

  case INPUT_RPN_LPF_UPPER: {
    u32 frq = (value << 9) | (inpChannelDefaults[set][channel].lpfUpperFrqBoundary & INPUT_LPF_FREQUENCY_MASK);
    inpChannelDefaults[set][channel].lpfUpperFrqBoundary = frq;

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].lpfUpperFrqBoundary = frq;
      }
    }
  } break;
#endif
  default:
    break;
  }
}

void inpSetRPNLo(u8 set, u8 channel, u8 value) {
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
  u16 rpn;
  u32 i;

  rpn = (midi_ctrl[set][channel][100]) | (midi_ctrl[set][channel][101] << 8);
  switch (rpn) {
  case INPUT_RPN_LPF_LOWER: {
    u32 frq = (value << 2) |
              (inpChannelDefaults[set][channel].lpfLowerFrqBoundary & (u16)(~INPUT_LPF_FREQUENCY_MASK));
    inpChannelDefaults[set][channel].lpfLowerFrqBoundary = frq;

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].lpfLowerFrqBoundary = frq;
      }
    }
  } break;

  case INPUT_RPN_LPF_UPPER: {
    u32 frq = (value << 2) |
              (inpChannelDefaults[set][channel].lpfUpperFrqBoundary & (u16)(~INPUT_LPF_FREQUENCY_MASK));
    inpChannelDefaults[set][channel].lpfUpperFrqBoundary = frq;

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].lpfUpperFrqBoundary = frq;
      }
    }
  } break;
  }
#endif
}

void inpSetRPNDec(u8 set, u8 channel) {
  u16 rpn;  // r28
  u32 i;    // r31
  u8 range; // r30

  rpn = (midi_ctrl[set][channel][100]) | (midi_ctrl[set][channel][101] << 8);
  switch (rpn) {
  case 0:
    range = inpChannelDefaults[set][channel].pbRange;
    if (range != 0) {
      --range;
    }
    inpChannelDefaults[set][channel].pbRange = range;
    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].pbUpperKeyRange = range;
        synthVoice[i].pbLowerKeyRange = range;
      }
    }
    break;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
  case INPUT_RPN_LPF_LOWER: {
    if (inpChannelDefaults[set][channel].lpfLowerFrqBoundary != 0) {
      --inpChannelDefaults[set][channel].lpfLowerFrqBoundary;
    }

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].lpfLowerFrqBoundary = inpChannelDefaults[set][channel].lpfLowerFrqBoundary;
      }
    }
  } break;

  case INPUT_RPN_LPF_UPPER: {
    if (inpChannelDefaults[set][channel].lpfUpperFrqBoundary != 0) {
      --inpChannelDefaults[set][channel].lpfUpperFrqBoundary;
    }

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].lpfUpperFrqBoundary = inpChannelDefaults[set][channel].lpfUpperFrqBoundary;
      }
    }
  } break;
#endif
  default:
    break;
  }
}

void inpSetRPNInc(u8 set, u8 channel) {
  u16 rpn;  // r28
  u32 i;    // r31
  u8 range; // r30

  rpn = (midi_ctrl[set][channel][100]) | (midi_ctrl[set][channel][101] << 8);
  switch (rpn) {
  case 0:
    range = inpChannelDefaults[set][channel].pbRange;
    if (range < 24) {
      ++range;
    }

    inpChannelDefaults[set][channel].pbRange = range;
    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].pbUpperKeyRange = range;
        synthVoice[i].pbLowerKeyRange = range;
      }
    }
    break;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
  case INPUT_RPN_LPF_LOWER: {
    if (inpChannelDefaults[set][channel].lpfLowerFrqBoundary != INPUT_VALUE_MAX) {
      ++inpChannelDefaults[set][channel].lpfLowerFrqBoundary;
    }

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].lpfLowerFrqBoundary = inpChannelDefaults[set][channel].lpfLowerFrqBoundary;
      }
    }
  } break;

  case INPUT_RPN_LPF_UPPER: {
    if (inpChannelDefaults[set][channel].lpfUpperFrqBoundary != INPUT_VALUE_MAX) {
      ++inpChannelDefaults[set][channel].lpfUpperFrqBoundary;
    }

    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].lpfUpperFrqBoundary = inpChannelDefaults[set][channel].lpfUpperFrqBoundary;
      }
    }
  } break;
#endif
  default:
    break;
  }
}

void inpSetMidiCtrl(u8 ctrl, u8 channel, u8 set, u8 value) {
  u32 i;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
  bool changed;
#endif
  if (channel == SYNTH_FX_MIDISET) {
    return;
  }

  if (set != SYNTH_FX_MIDISET) {
    switch (ctrl) {
    case 6:
      inpSetRPNHi(set, channel, value);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
      changed = TRUE;
#endif
      break;
    case 38:
      inpSetRPNLo(set, channel, value);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
      changed = TRUE;
#endif
      break;
    case 96:
      inpSetRPNDec(set, channel);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
      changed = TRUE;
#endif
      break;
    case 97:
      inpSetRPNInc(set, channel);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
      changed = TRUE;
#endif
      break;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
    default:
      changed = midi_ctrl[set][channel][ctrl] != (u8)(value & INPUT_MIDI_VALUE_MASK);
      break;
#endif
    }

    midi_ctrl[set][channel][ctrl] = (value & INPUT_MIDI_VALUE_MASK);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
    if (changed) {
      for (i = 0; i < synthInfo.voiceNum; ++i) {
        if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
          synthVoice[i].midiDirtyFlags = INPUT_DIRTY_ALL;
          synthKeyStateUpdate(&synthVoice[i]);
        }
      }
      inpGlobalMIDIDirtyFlags[set][channel] = INPUT_GLOBAL_DIRTY_ALL;
    }
#else
    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].midiDirtyFlags = INPUT_DIRTY_LEGACY;
        synthKeyStateUpdate(&synthVoice[i]);
      }
    }
    inpGlobalMIDIDirtyFlags[set][channel] = INPUT_GLOBAL_DIRTY_ALL;
#endif

  } else {
    switch (ctrl) {
    case 6:
      inpSetRPNHi(set, channel, value);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
      changed = TRUE;
#endif
      break;
    case 38:
      inpSetRPNLo(set, channel, value);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
      changed = TRUE;
#endif
      break;
    case 96:
      inpSetRPNDec(set, channel);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
      changed = TRUE;
#endif
      break;
    case 97:
      inpSetRPNInc(set, channel);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
      changed = TRUE;
#endif
      break;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
    default:
      changed = fx_ctrl[channel][ctrl] != (u8)(value & INPUT_MIDI_VALUE_MASK);
      break;
#endif
    }

    fx_ctrl[channel][ctrl] = (value & INPUT_MIDI_VALUE_MASK);
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
    if (changed) {
      for (i = 0; i < synthInfo.voiceNum; ++i) {
        if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
          synthVoice[i].midiDirtyFlags = INPUT_DIRTY_ALL;
          synthKeyStateUpdate(&synthVoice[i]);
        }
      }
    }
#else
    for (i = 0; i < synthInfo.voiceNum; ++i) {
      if (set == synthVoice[i].midiSet && channel == synthVoice[i].midi) {
        synthVoice[i].midiDirtyFlags = INPUT_DIRTY_LEGACY;
        synthKeyStateUpdate(&synthVoice[i]);
      }
    }
#endif
  }
}

void inpSetMidiCtrl14(u8 ctrl, u8 channel, u8 set, u16 value) {

  if (channel == SYNTH_FX_MIDISET) {
    return;
  }

  if (ctrl < 64) {
    inpSetMidiCtrl(ctrl & 31, channel, set, value >> 7);
    inpSetMidiCtrl((ctrl & 31) + 32, channel, set, value & INPUT_MIDI_VALUE_MASK);
  } else if (ctrl == 128 || ctrl == 129) {
    inpSetMidiCtrl(ctrl & 254, channel, set, value >> 7);
    inpSetMidiCtrl((ctrl & 254) + 1, channel, set, value & INPUT_MIDI_VALUE_MASK);
  } else if (ctrl == 132 || ctrl == 133) {
    inpSetMidiCtrl(ctrl & 254, channel, set, value >> 7);
    inpSetMidiCtrl((ctrl & 254) + 1, channel, set, value & INPUT_MIDI_VALUE_MASK);
  } else {
    inpSetMidiCtrl(ctrl, channel, set, value >> 7);
  }
}

static const u8 inpColdMIDIDefaults[134] = {
    0, 0, 0, 0, 0, 0, 0, 127, 0, 0, 64, 127, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 127, 127, 127, 127, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 64, 0,
};
static const u8 inpWarmMIDIDefaults[134] = {
    255, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 127, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 127, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 0, 0, 0, 0, 255, 0, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 64, 255, 255, 255, 255, 255,

};

void inpResetMidiCtrl(u8 ch, u8 set, u32 coldReset) {
  const u8* values; // r30
  u8* dest;         // r29
  u32 i;            // r31

  values = coldReset ? inpColdMIDIDefaults : inpWarmMIDIDefaults;
  dest = set != SYNTH_FX_MIDISET ? midi_ctrl[set][ch] : fx_ctrl[ch];

  if (coldReset) {
    memcpy(dest, values, 134);
  } else {
    for (i = 0; i < 134; ++i) {
      if (values[i] != SND_MIDI_NONE) {
        dest[i] = values[i];
      }
    }
  }

  inpSetMidiLastNote(ch, set, SND_MIDI_NONE);
}

u16 inpGetMidiCtrl(u8 ctrl, u8 channel, u8 set) {

  if (channel != SYNTH_FX_MIDISET) {
    if (set != SYNTH_FX_MIDISET) {

      if (ctrl < SND_MIDICTRL_PEDAL) {
        return midi_ctrl[set][channel][ctrl & INPUT_CTRL_LSB_MASK] << 7 |
               midi_ctrl[set][channel][(ctrl & INPUT_CTRL_LSB_MASK) + INPUT_CTRL_LSB_OFFSET];
      }
      if (ctrl < INPUT_SWITCH_CTRL_LIMIT) {
        return midi_ctrl[set][channel][ctrl] < SND_MIDICTRL_PEDAL ? 0 : INPUT_VALUE_MAX;
      }
      if (ctrl >= INPUT_RPN_CTRL_FIRST && ctrl < INPUT_RPN_CTRL_LIMIT) {
        return 0;
      }

      if ((ctrl == SND_MIDICTRL_PITCHBEND) || (ctrl == SND_MIDICTRL_PITCHBEND + 1)) {
        return midi_ctrl[set][channel][ctrl & INPUT_MIDI_PAIR_MASK] << 7 |
               midi_ctrl[set][channel][(ctrl & INPUT_MIDI_PAIR_MASK) + 1];
      }
      if ((ctrl == SND_MIDICTRL_DOPPLER) || (ctrl == SND_MIDICTRL_DOPPLER + 1)) {
        return midi_ctrl[set][channel][ctrl & INPUT_MIDI_PAIR_MASK] << 7 |
               midi_ctrl[set][channel][(ctrl & INPUT_MIDI_PAIR_MASK) + 1];
      }

      return midi_ctrl[set][channel][ctrl] << 7;
    }
    if (ctrl < SND_MIDICTRL_PEDAL) {
      return fx_ctrl[channel][ctrl & INPUT_CTRL_LSB_MASK] << 7 |
             fx_ctrl[channel][(ctrl & INPUT_CTRL_LSB_MASK) + INPUT_CTRL_LSB_OFFSET];
    }
    if (ctrl < INPUT_SWITCH_CTRL_LIMIT) {
      return fx_ctrl[channel][ctrl] < SND_MIDICTRL_PEDAL ? 0 : INPUT_VALUE_MAX;
    }
    if (ctrl >= INPUT_RPN_CTRL_FIRST && ctrl < INPUT_RPN_CTRL_LIMIT) {
      return 0;
    }
    if ((ctrl == SND_MIDICTRL_PITCHBEND) || (ctrl == SND_MIDICTRL_PITCHBEND + 1)) {
      return fx_ctrl[channel][ctrl & INPUT_MIDI_PAIR_MASK] << 7 |
             fx_ctrl[channel][(ctrl & INPUT_MIDI_PAIR_MASK) + 1];
    }
    if ((ctrl == SND_MIDICTRL_DOPPLER) || (ctrl == SND_MIDICTRL_DOPPLER + 1)) {
      return fx_ctrl[channel][ctrl & INPUT_MIDI_PAIR_MASK] << 7 |
             fx_ctrl[channel][(ctrl & INPUT_MIDI_PAIR_MASK) + 1];
    }
    return fx_ctrl[channel][ctrl] << 7;
  }
  return 0;
}

CHANNEL_DEFAULTS* inpGetChannelDefaults(u8 midi, u8 midiSet) {
  if (midiSet == SYNTH_FX_MIDISET) {
    return &inpFXChannelDefaults[midi];
  }

  return &inpChannelDefaults[midiSet][midi];
}

void inpResetChannelDefaults(u8 midi, u8 midiSet) {
  CHANNEL_DEFAULTS* channelDefaults; // r31
  channelDefaults =
      midiSet != SYNTH_FX_MIDISET ? &inpChannelDefaults[midiSet][midi]
                                 : &inpFXChannelDefaults[midi];
  channelDefaults->pbRange = 2;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
  channelDefaults->lpfLowerFrqBoundary = INPUT_LPF_LOW_DEFAULT;
  channelDefaults->lpfUpperFrqBoundary = INPUT_LPF_HIGH_DEFAULT;
#endif
}

void inpAddCtrl(CTRL_DEST* dest, u8 ctrl, s32 scale, u8 comb, u32 isVar) {
  u8 n; // r30
  if (comb == 0) {
    dest->numSource = 0;
  }

  if (dest->numSource < 4) {
    n = dest->numSource++;
    if (isVar == 0) {
      ctrl = inpTranslateExCtrl(ctrl);
    } else {
      comb |= INPUT_SOURCE_VARIABLE;
    }

    dest->source[n].midiCtrl = ctrl;
    dest->source[n].combine = comb;
    dest->source[n].scale = scale;
  }
}

void inpFXCopyCtrl(u8 ctrl, SYNTH_VOICE* dvoice, SYNTH_VOICE* svoice) {
  u8 di; // r30
  u8 si; // r29
  di = dvoice->id;
  si = svoice->id;

  if (ctrl < 64) {
    fx_ctrl[di][ctrl & 31] = fx_ctrl[si][ctrl & 31];
    fx_ctrl[di][(ctrl & 31) + 32] = fx_ctrl[si][(ctrl & 31) + 32];
  } else if (ctrl == 128 || ctrl == 129) {
    fx_ctrl[di][ctrl & 254] = fx_ctrl[si][ctrl & 254];
    fx_ctrl[di][(ctrl & 254) + 1] = fx_ctrl[si][(ctrl & 254) + 1];
  } else if (ctrl == 132 || ctrl == 133) {
    fx_ctrl[di][ctrl & 254] = fx_ctrl[si][ctrl & 254];
    fx_ctrl[di][(ctrl & 254) + 1] = fx_ctrl[si][(ctrl & 254) + 1];
  } else {
    fx_ctrl[di][ctrl] = fx_ctrl[si][ctrl];
  }
}

void inpSetMidiLastNote(u8 midi, u8 midiSet, u8 key) {
  if (midiSet != SYNTH_FX_MIDISET) {
    midi_lastNote[midiSet][midi] = key;
  } else {
    fx_lastNote[midi] = key;
  }
}

u8 inpGetMidiLastNote(u8 midi, u8 midiSet) {
  if (midiSet != SYNTH_FX_MIDISET) {
    return midi_lastNote[midiSet][midi];
  }
  return fx_lastNote[midi];
}

static u16 _GetInputValue(struct SYNTH_VOICE* svoice /* r27 */, struct CTRL_DEST* inp /* r24 */,
                          u8 midi /* r22 */, u8 midiSet /* r23 */) {
  u32 i;     // r26
  u32 value; // r29
  u8 ctrl;   // r28
  s32 tmp;   // r31
  s32 vtmp;  // r30
  bool sign; // r25

  for (value = 0, i = 0; i < inp->numSource; ++i) {
    if (inp->source[i].combine & INPUT_SOURCE_VARIABLE) {
      tmp = (svoice != NULL ? varGet(svoice, 0, inp->source[i].midiCtrl) : 0);
      goto block_18;
    }
    ctrl = inp->source[i].midiCtrl;
    if (ctrl == 128 || ctrl == 1 || ctrl == 10 || ctrl == 160 || ctrl == 161 || ctrl == 131) {
      switch (ctrl) {
      case 160:
      case 161:
        if (svoice != NULL) {
          tmp = svoice->lfo[ctrl - 160].value << 1;
          svoice->lfoUsedByInput[ctrl - 160] = 1;
        } else {
          tmp = 0;
        }
        break;
      default:
        tmp = inpGetMidiCtrl(ctrl, midi, midiSet) - INPUT_SIGNED_CENTER;
        break;
      }
    block_18:
      tmp = (tmp * (inp->source[i].scale >> 1)) >> 15;
      tmp = CLAMP_INV(tmp, -INPUT_SIGNED_CENTER, INPUT_SIGNED_MAX);
      switch (inp->source[i].combine & INPUT_COMBINE_MASK) {
      case 0:
        value = tmp + INPUT_SIGNED_CENTER;
        sign = TRUE;
        break;
      case 1:
        if (sign != FALSE) {
          vtmp = (value + tmp);
          vtmp -= INPUT_SIGNED_CENTER;
          value = CLAMP_INV(vtmp, -INPUT_SIGNED_CENTER, INPUT_SIGNED_MAX) + INPUT_SIGNED_CENTER;
        } else {
          vtmp = value + tmp;
          value = CLAMP(vtmp, 0, INPUT_VALUE_MAX);
        }
        break;
      case 2:
        if (sign != FALSE) {
          vtmp = (s32)((value - INPUT_SIGNED_CENTER) * tmp) >> 13;
        } else {
          vtmp = (tmp * value) >> 13;
          sign = TRUE;
        }
        value = CLAMP_INV(vtmp, -INPUT_SIGNED_CENTER, INPUT_SIGNED_MAX) + INPUT_SIGNED_CENTER;
        break;
      case 3:
        if (sign != FALSE) {
          vtmp = (value - INPUT_SIGNED_CENTER) - tmp;
          value = CLAMP_INV(vtmp, -INPUT_SIGNED_CENTER, INPUT_SIGNED_MAX) + INPUT_SIGNED_CENTER;
        } else {
          vtmp = value - tmp;
          value = CLAMP(vtmp, 0, INPUT_VALUE_MAX);
        }
        break;
      }
    } else {
      switch (ctrl) {
      case 162:
        if (svoice != NULL) {
          tmp = svoice->orgNote << 7;
        } else {
          tmp = 0;
        }
        break;
      case 163:
        tmp = svoice != NULL ? svoice->orgVolume >> 9 : 0;
        break;
      case 164:
        if (svoice != NULL) {
          tmp = (synthRealTime - svoice->macStartTime) >> 8;
          if (tmp > INPUT_VALUE_MAX) {
            tmp = INPUT_VALUE_MAX;
          }
          svoice->timeUsedByInput = 1;
        } else {
          tmp = 0;
        }
        break;
      default:
        tmp = inpGetMidiCtrl(ctrl, midi, midiSet);
        break;
      }
      tmp = (tmp * (inp->source[i].scale >> 1)) >> 15;
      if (tmp > INPUT_VALUE_MAX) {
        tmp = INPUT_VALUE_MAX;
      }
      switch (inp->source[i].combine & INPUT_COMBINE_MASK) {
      case 0:
        value = tmp;
        sign = FALSE;
        break;
      case 1:
        if (sign != FALSE) {
          vtmp = (value + tmp);
          vtmp -= INPUT_SIGNED_CENTER;
          value = CLAMP_INV(vtmp, -INPUT_SIGNED_CENTER, INPUT_SIGNED_MAX) + INPUT_SIGNED_CENTER;
        } else {
          value += tmp;
          value = MIN(value, INPUT_VALUE_MAX);
        }
        break;
      case 2:
        if (sign != FALSE) {
          vtmp = (s32)(tmp * (value - INPUT_SIGNED_CENTER)) >> INPUT_FRACTION_BITS;
          value = CLAMP_INV(vtmp, -INPUT_SIGNED_CENTER, INPUT_SIGNED_MAX) + INPUT_SIGNED_CENTER;
        } else {
          value = ((value * tmp) >> INPUT_FRACTION_BITS);
          value = MIN(value, INPUT_VALUE_MAX);
        }
        break;
      case 3:
        if (sign != FALSE) {
          vtmp = (value - INPUT_SIGNED_CENTER) - tmp;
          value = CLAMP_INV(vtmp, -INPUT_SIGNED_CENTER, INPUT_SIGNED_MAX) + INPUT_SIGNED_CENTER;
        } else {
          vtmp = value - tmp;
          value = CLAMP(vtmp, 0, INPUT_VALUE_MAX);
        }
        break;
      }
    }
  }
  inp->oldValue = value;
  return value;
}

static u16 GetInputValue(SYNTH_VOICE* svoice, CTRL_DEST* inp, u32 dirtyMask) {

  if (!(svoice->midiDirtyFlags & dirtyMask)) {
    return inp->oldValue;
  }

  svoice->midiDirtyFlags &= ~dirtyMask;

  return _GetInputValue(svoice, inp, svoice->midi, svoice->midiSet);
}

static u16 GetGlobalInputValue(CTRL_DEST* inp, u32 dirtyMask, u8 midi, u8 midiSet) {
  if (!inpResetGlobalMIDIDirtyFlag(midi, midiSet, dirtyMask)) {
    return inp->oldValue;
  }
  return _GetInputValue(NULL, inp, midi, midiSet);
}

u16 inpGetVolume(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpVolume, INPUT_DIRTY_VOLUME);
}

u16 inpGetPanning(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpPanning, INPUT_DIRTY_PANNING);
}

u16 inpGetSurPanning(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpSurroundPanning, INPUT_DIRTY_SURROUND_PANNING);
}

u16 inpGetPitchBend(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpPitchBend, INPUT_DIRTY_PITCH_BEND);
}

u16 inpGetDoppler(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpDoppler, INPUT_DIRTY_DOPPLER);
}

u16 inpGetModulation(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpModulation, INPUT_DIRTY_MODULATION);
}

u16 inpGetPedal(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpPedal, INPUT_DIRTY_PEDAL);
}

u16 inpGetPreAuxA(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpPreAuxA, INPUT_DIRTY_PRE_AUX_A);
}

u16 inpGetReverb(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpReverb, INPUT_DIRTY_REVERB);
}

u16 inpGetPreAuxB(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpPreAuxB, INPUT_DIRTY_PRE_AUX_B);
}

u16 inpGetPostAuxB(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpPostAuxB, INPUT_DIRTY_POST_AUX_B);
}

u16 inpGetTremolo(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpTremolo, INPUT_DIRTY_TREMOLO);
}

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
u16 inpGetFilterSwitch(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpFilterSwitch, INPUT_FILTER_SWITCH_DIRTY);
}

u16 inpGetFilterParameter(SYNTH_VOICE* svoice) {
  return GetInputValue(svoice, &svoice->inpFilterParameter, INPUT_FILTER_PARAMETER_DIRTY);
}
#endif

u16 inpGetAuxA(u8 studio, u8 index, u8 midi, u8 midiSet) {
  static u32 dirtyMask[4] = {
      INPUT_AUX_A_DIRTY_STUDIO_0,
      INPUT_AUX_A_DIRTY_STUDIO_1,
      INPUT_AUX_A_DIRTY_STUDIO_2,
      INPUT_AUX_A_DIRTY_STUDIO_3,
  };
  return GetGlobalInputValue(&inpAuxA[studio][index], dirtyMask[index], midi, midiSet);
}

u16 inpGetAuxB(u8 studio, u8 index, u8 midi, u8 midiSet) {
  static u32 dirtyMask[4] = {
      INPUT_AUX_B_DIRTY_STUDIO_0,
      INPUT_AUX_B_DIRTY_STUDIO_1,
      INPUT_AUX_B_DIRTY_STUDIO_2,
      INPUT_AUX_B_DIRTY_STUDIO_3,
  };

  return GetGlobalInputValue(&inpAuxB[studio][index], dirtyMask[index], midi, midiSet);
}

void inpInit(SYNTH_VOICE* svoice) {
  u32 i; // r30
  u32 s; // r29

  if (svoice != NULL) {
    svoice->inpVolume.source[0].midiCtrl = 7;
    svoice->inpVolume.source[0].combine = 0;
    svoice->inpVolume.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpVolume.source[1].midiCtrl = 11;
    svoice->inpVolume.source[1].combine = 2;
    svoice->inpVolume.source[1].scale = INPUT_DEFAULT_SCALE;
    svoice->inpVolume.numSource = 2;
    svoice->inpPanning.source[0].midiCtrl = 10;
    svoice->inpPanning.source[0].combine = 0;
    svoice->inpPanning.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpPanning.numSource = 1;
    svoice->inpSurroundPanning.source[0].midiCtrl = 131;
    svoice->inpSurroundPanning.source[0].combine = 0;
    svoice->inpSurroundPanning.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpSurroundPanning.numSource = 1;
    svoice->inpPitchBend.source[0].midiCtrl = 128;
    svoice->inpPitchBend.source[0].combine = 0;
    svoice->inpPitchBend.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpPitchBend.numSource = 1;
    svoice->inpModulation.source[0].midiCtrl = 1;
    svoice->inpModulation.source[0].combine = 0;
    svoice->inpModulation.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpModulation.numSource = 1;
    svoice->inpPedal.source[0].midiCtrl = 64;
    svoice->inpPedal.source[0].combine = 0;
    svoice->inpPedal.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpPedal.numSource = 1;
    svoice->inpPortamento.source[0].midiCtrl = 65;
    svoice->inpPortamento.source[0].combine = 0;
    svoice->inpPortamento.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpPortamento.numSource = 1;
    svoice->inpPreAuxA.numSource = 0;
    svoice->inpReverb.source[0].midiCtrl = 91;
    svoice->inpReverb.source[0].combine = 0;
    svoice->inpReverb.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpReverb.numSource = 1;
    svoice->inpPreAuxB.numSource = 0;
    svoice->inpPostAuxB.source[0].midiCtrl = 93;
    svoice->inpPostAuxB.source[0].combine = 0;
    svoice->inpPostAuxB.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpPostAuxB.numSource = 1;
    svoice->inpDoppler.source[0].midiCtrl = 132;
    svoice->inpDoppler.source[0].combine = 0;
    svoice->inpDoppler.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpDoppler.numSource = 1;
    svoice->inpTremolo.numSource = 0;

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
    svoice->inpFilterSwitch.source[0].midiCtrl = INPUT_FILTER_SWITCH_CTRL;
    svoice->inpFilterSwitch.source[0].combine = 0;
    svoice->inpFilterSwitch.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpFilterSwitch.numSource = 1;
    svoice->inpFilterParameter.source[0].midiCtrl = INPUT_FILTER_PARAMETER_CTRL;
    svoice->inpFilterParameter.source[0].combine = 0;
    svoice->inpFilterParameter.source[0].scale = INPUT_DEFAULT_SCALE;
    svoice->inpFilterParameter.numSource = 1;
#endif

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 3)
    svoice->midiDirtyFlags = INPUT_DIRTY_ALL;
#else
    svoice->midiDirtyFlags = INPUT_DIRTY_LEGACY;
#endif
    svoice->lfoUsedByInput[0] = 0;
    svoice->lfoUsedByInput[1] = 0;
    svoice->timeUsedByInput = 0;
  } else {
    for (s = 0; s < 8; ++s) {
      for (i = 0; i < 4; ++i) {
        inpAuxA[s][i].numSource = 0;
        inpAuxB[s][i].numSource = 0;
      }
    }

    inpResetGlobalMIDIDirtyFlags();
  }
}

u8 inpTranslateExCtrl(u8 ctrl) {
  switch (ctrl) {
  case INPUT_TRANSLATE_CTRL_PITCH_BEND:
    ctrl = INPUT_TRANSLATE_CTRL_PITCH_BEND;
    break;
  case INPUT_TRANSLATE_CTRL_PITCH_BEND_FINE:
    ctrl = INPUT_TRANSLATE_CTRL_LFO_0;
    break;
  case INPUT_TRANSLATE_CTRL_LFO_0:
    ctrl = INPUT_RUNTIME_LFO_0;
    break;
  case INPUT_TRANSLATE_CTRL_LFO_1:
    ctrl = INPUT_RUNTIME_LFO_1;
    break;
  case INPUT_TRANSLATE_CTRL_SURROUND_PANNING:
    ctrl = INPUT_TRANSLATE_CTRL_LFO_1;
    break;
  case INPUT_TRANSLATE_CTRL_DOPPLER:
    ctrl = INPUT_TRANSLATE_CTRL_SURROUND_PANNING;
    break;
  case INPUT_TRANSLATE_CTRL_NOTE:
    ctrl = INPUT_RUNTIME_NOTE;
    break;
  case INPUT_TRANSLATE_CTRL_VOLUME:
    ctrl = INPUT_RUNTIME_VOLUME;
    break;
  case INPUT_TRANSLATE_CTRL_TIME:
    ctrl = INPUT_RUNTIME_TIME;
    break;
  }
  return ctrl;
}
u16 inpGetExCtrl(SYNTH_VOICE* svoice, u8 ctrl) {
  u16 v; // r30
  switch (inpTranslateExCtrl(ctrl)) {
  case INPUT_RUNTIME_LFO_0:
    v = (svoice->lfo[0].value << 1) + INPUT_SIGNED_CENTER;
    break;
  case INPUT_RUNTIME_LFO_1:
    v = (svoice->lfo[1].value << 1) + INPUT_SIGNED_CENTER;
    break;
  default:
    v = svoice->midi != SYNTH_FX_MIDISET ? inpGetMidiCtrl(ctrl, svoice->midi, svoice->midiSet) : 0;
    break;
  }

  return v;
}
void inpSetExCtrl(SYNTH_VOICE* svoice, u8 ctrl, s16 v) {
  v = v < 0 ? 0 : v > INPUT_VALUE_MAX ? INPUT_VALUE_MAX : v;

  switch (inpTranslateExCtrl(ctrl)) {
  case INPUT_RUNTIME_LFO_1:
  case INPUT_RUNTIME_LFO_0:
    break;
  default:
    if (svoice->midi != SYNTH_FX_MIDISET) {
      inpSetMidiCtrl14(ctrl, svoice->midi, svoice->midiSet, v);
    }
    break;
  }
}
