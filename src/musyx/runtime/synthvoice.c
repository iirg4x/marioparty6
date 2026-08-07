
#include "musyx/musyx.h"

#include "musyx/assert.h"
#include "musyx/hardware.h"
#include "musyx/macros.h"
#include "musyx/stream.h"
#include "musyx/voice.h"

#define VOICE_INDEX_NONE 255
#define VOICE_PRIORITY_NONE 65535
#define VOICE_ALLOC_ID_LEGACY_NONE 65535
#define SAMPLE_REFERENCE_END 65535
#define SAMPLE_REFERENCE_RANGE_FLAG 32768
#define SAMPLE_REFERENCE_ID_MASK 16383
#define VOICE_ID_NONE ((u32)4294967295U)
#define VOICE_INDEX_MASK 255
#define VOICE_MIDI_NONE 255
#define VOICE_BLOCK_ID_MASK ((u32)4294967040U)

void voiceResetLastStarted(SYNTH_VOICE* svoice);

static VID_LIST vidList[128];
static u8 synth_last_started[8][16];
static u8 synth_last_fxstarted[64];
SYNTH_VOICELIST voiceList[64];
SYNTH_ROOTLIST voicePrioSortRootList[256];
u8 voicePrioSortVoicesRoot[256];
SYNTH_VOICELIST voicePrioSortVoices[64];
static VID_LIST* vidFree = NULL;
static VID_LIST* vidRoot = NULL;
static u32 vidCurrentId = 0;
u16 voicePrioSortRootListRoot = 0;
u8 voiceMusicRunning = 0;
u8 voiceFxRunning = 0;
u8 voiceListInsert = 0;
u8 voiceListRoot = 0;

void vidInit() {
  int i;
  VID_LIST* lvl;
  vidCurrentId = 0;
  vidRoot = NULL;
  vidFree = vidList;
  for (lvl = NULL, i = 0; i < 128; lvl = &vidList[i], ++i) {
    vidList[i].prev = lvl;
    if (lvl != NULL) {
      lvl->next = &vidList[i];
    }
  }
  lvl->next = NULL;
}

static VID_LIST* get_vidlist(u32 vid) {
  VID_LIST* vl = vidRoot;
  while (vl != NULL) {
    if (vl->vid == vid) {
      return vl;
    }
    if (vl->vid > vid) {
      break;
    }
    vl = vl->next;
  }

  return NULL;
}

static u32 get_newvid() {
  u32 vid; // r31
  do {
    vid = vidCurrentId++;
  } while (vid == VOICE_ID_NONE);

  return vid;
}

static void vidRemove(VID_LIST** vidList) {
  if ((*vidList)->prev != NULL) {
    (*vidList)->prev->next = (*vidList)->next;
  } else {
    vidRoot = (*vidList)->next;
  }

  if ((*vidList)->next != NULL) {
    (*vidList)->next->prev = (*vidList)->prev;
  }

  (*vidList)->next = vidFree;

  if (vidFree != NULL) {
    vidFree->prev = *vidList;
  }

  (*vidList)->prev = NULL;
  vidFree = *vidList;
  *vidList = NULL;
}

void vidRemoveVoiceReferences(SYNTH_VOICE* svoice) {
  if (svoice->id == VOICE_ID_NONE) {
    return;
  }

  voiceResetLastStarted(svoice);
  if (svoice->parent != VOICE_ID_NONE) {
    synthVoice[svoice->parent & VOICE_INDEX_MASK].child = svoice->child;
    if (svoice->child != VOICE_ID_NONE) {
      synthVoice[svoice->child & VOICE_INDEX_MASK].parent = svoice->parent;
    }

    vidRemove(&svoice->vidList);
  } else if (svoice->child != VOICE_ID_NONE) {
    svoice->vidList->root = svoice->child;
    synthVoice[svoice->child & VOICE_INDEX_MASK].parent = VOICE_ID_NONE;
    synthVoice[svoice->child & VOICE_INDEX_MASK].vidMasterList = svoice->vidMasterList;
    if (svoice->vidList != svoice->vidMasterList) {
      vidRemove(&svoice->vidList);
    }

    svoice->vidMasterList = svoice->vidList = NULL;
  } else if (svoice->vidList != svoice->vidMasterList) {
    vidRemove(&svoice->vidList);
    vidRemove(&svoice->vidMasterList);
  } else {
    vidRemove(&svoice->vidList);
    svoice->vidMasterList = NULL;
  }
}

u32 vidMakeRoot(SYNTH_VOICE* svoice) {
  svoice->vidMasterList = svoice->vidList;
  return svoice->vidList->vid;
}

u32 vidMakeNew(SYNTH_VOICE* svoice, u32 isMaster) {
  u32 vid;       // r29
  VID_LIST* nvl; // r30
  VID_LIST* lvl; // r28
  VID_LIST* vl;  // r31

  vid = get_newvid();
  lvl = NULL;
  nvl = vidRoot;

  while (nvl != NULL) {
    if (nvl->vid > vid) {
      break;
    }

    if (nvl->vid == vid) {
      vid = get_newvid();
    }

    lvl = nvl;
    nvl = nvl->next;
  }

  if ((vl = vidFree) == NULL) {
    return VOICE_ID_NONE;
  }

  if ((vidFree = vidFree->next) != NULL) {
    vidFree->prev = NULL;
  }

  if (lvl == NULL) {
    vidRoot = vl;
  } else {
    lvl->next = vl;
  }

  vl->prev = lvl;
  vl->next = nvl;

  if (nvl != NULL) {
    nvl->prev = vl;
  }

  vl->vid = vid;
  vl->root = svoice->id;
  svoice->vidMasterList = isMaster ? vl : NULL;
  svoice->vidList = vl;

  return isMaster ? vid : svoice->id;
}

u32 vidGetInternalId(u32 vid) {
  VID_LIST* vl;

  if (vid != VOICE_ID_NONE) {
    if ((vl = get_vidlist(vid)) != NULL) {
      return vl->root;
    }
  }

  return VOICE_ID_NONE;
}

static void voiceInitPrioSort() {
  u32 i;

  for (i = 0; i < synthInfo.voiceNum; ++i) {
    voicePrioSortVoices[i].user = 0;
  }

  for (i = 0; i < 256; ++i) {
    voicePrioSortVoicesRoot[i] = VOICE_INDEX_NONE;
  }

  voicePrioSortRootListRoot = VOICE_PRIORITY_NONE;
}

void voiceRemovePriority(SYNTH_VOICE* svoice) {
  SYNTH_VOICELIST* vps; // r31
  SYNTH_ROOTLIST* rps;  // r30

  vps = &voicePrioSortVoices[svoice->id & VOICE_INDEX_MASK];
  if (vps->user != 1) {
    return;
  }

  if (vps->prev != VOICE_INDEX_NONE) {
    voicePrioSortVoices[vps->prev].next = vps->next;
  } else {
    voicePrioSortVoicesRoot[svoice->prio] = vps->next;
  }

  if (vps->next != VOICE_INDEX_NONE) {
    voicePrioSortVoices[vps->next].prev = vps->prev;
  } else if (vps->prev == VOICE_INDEX_NONE) {
    rps = &voicePrioSortRootList[svoice->prio];

    if (rps->prev != VOICE_PRIORITY_NONE) {
      voicePrioSortRootList[rps->prev].next = rps->next;

    } else {
      voicePrioSortRootListRoot = rps->next;
    }

    if (rps->next != VOICE_PRIORITY_NONE) {
      voicePrioSortRootList[rps->next].prev = rps->prev;
    }
  }

  vps->user = 0;
}

void voiceSetPriority(SYNTH_VOICE* svoice, u8 prio) {
  u16 li;               // r25
  SYNTH_VOICELIST* vps; // r27
  u16 i;                // r29
  u32 v;                // r26
  v = (u8)svoice->id;
  vps = &voicePrioSortVoices[v];
  if (vps->user == 1) {
    if (svoice->prio == prio) {
      return;
    }

    voiceRemovePriority(svoice);
  }

  vps->user = 1;
  vps->prev = VOICE_INDEX_NONE;
  if ((vps->next = voicePrioSortVoicesRoot[prio]) != VOICE_INDEX_NONE) {
    voicePrioSortVoices[voicePrioSortVoicesRoot[prio]].prev = v;
  } else if (voicePrioSortRootListRoot != VOICE_PRIORITY_NONE) {
    if (prio >= voicePrioSortRootListRoot) {
      for (i = voicePrioSortRootListRoot; i != VOICE_PRIORITY_NONE; i = voicePrioSortRootList[i].next) {
        if ((u16)i > prio) {
          break;
        }
        li = i;
      }

      voicePrioSortRootList[li].next = (u16)prio;
      voicePrioSortRootList[prio].prev = li;
      voicePrioSortRootList[prio].next = i;
      if (i != VOICE_PRIORITY_NONE) {
        voicePrioSortRootList[i].prev = prio;
      }

    } else {
      voicePrioSortRootList[prio].next = voicePrioSortRootListRoot;
      voicePrioSortRootList[prio].prev = VOICE_PRIORITY_NONE;
      voicePrioSortRootList[voicePrioSortRootListRoot].prev = prio;
      voicePrioSortRootListRoot = prio;
    }
  } else {
    voicePrioSortRootList[prio].next = VOICE_PRIORITY_NONE;
    voicePrioSortRootList[prio].prev = VOICE_PRIORITY_NONE;
    voicePrioSortRootListRoot = prio;
  }

  voicePrioSortVoicesRoot[prio] = v;
  svoice->prio = prio;
  hwSetPriority(svoice->id & VOICE_INDEX_MASK, ((u32)prio << 24) | (svoice->age >> 15));
}

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
static s32 voiceAllocateFind(u8 priority, u8 maxVoices, u32 allocId, u8 fxFlag) {
  s32 i;          // r31
  s32 num;        // r27
  s32 voice;      // r30
  u16 p;          // r29
  u32 type_alloc; // r26

  if (synthIdleWaitActive == 0) {
    if (fxFlag != 0) {
      type_alloc = (voiceFxRunning >= synthInfo.maxSFX) && (synthInfo.voiceNum > synthInfo.maxSFX);
      if (synthInfo.maxSFX <= maxVoices) {
        goto _skip_alloc;
      }
      goto _do_alloc;
    }
    type_alloc =
        (voiceMusicRunning >= synthInfo.maxMusic) && (synthInfo.voiceNum > synthInfo.maxMusic);
    if (synthInfo.maxMusic > maxVoices) {
    _do_alloc:
      num = 0;
      voice = -1;
      for (p = voicePrioSortRootListRoot;
           (p != VOICE_PRIORITY_NONE) && (priority >= p) && (voice == -1);
           p = voicePrioSortRootList[p].next) {
        for (i = voicePrioSortVoicesRoot[p]; i != VOICE_INDEX_NONE;
             i = voicePrioSortVoices[i].next) {
          if (allocId == synthVoice[i].allocId) {
            num++;
            if ((synthVoice[i].block == 0) &&
                ((type_alloc == 0) || (fxFlag == synthVoice[i].fxFlag))) {
              if ((synthVoice[i].cFlags & 2) == 0) {
                if (voice != -1) {
                  if (synthVoice[i].age < synthVoice[voice].age) {
                    voice = i;
                  }
                } else {
                  voice = i;
                }
              }
            }
          }
        }
      }
      if (num >= maxVoices) {
        return voice;
      }
      for (; (p != VOICE_PRIORITY_NONE) && (num < maxVoices);
           p = voicePrioSortRootList[p].next) {
        for (i = voicePrioSortVoicesRoot[p]; i != VOICE_INDEX_NONE;
             i = voicePrioSortVoices[i].next) {
          if (allocId == synthVoice[i].allocId) {
            num++;
          }
        }
      }
      if (num >= maxVoices) {
        return voice;
      }
    }
  _skip_alloc:
    if ((voiceListRoot != VOICE_INDEX_NONE) && (type_alloc == 0)) {
      return voiceListRoot;
    }
    if (priority < voicePrioSortRootListRoot) {
      return -1;
    }
    voice = -1;
    for (p = voicePrioSortRootListRoot;
         (p != VOICE_PRIORITY_NONE) && (priority >= p) && (voice == -1);
         p = voicePrioSortRootList[p].next) {
      for (i = voicePrioSortVoicesRoot[p]; i != VOICE_INDEX_NONE;
           i = voicePrioSortVoices[i].next) {
        if ((synthVoice[i].block == 0) && ((type_alloc == 0) || (fxFlag == synthVoice[i].fxFlag))) {
          if ((synthVoice[i].cFlags & 2) == 0) {
            if (voice != -1) {
              if (synthVoice[voice].age > synthVoice[i].age) {
                voice = i;
              }
            } else {
              voice = i;
            }
          }
        }
      }
    }
    if (voice == -1) {
      return -1;
    }
    if (synthVoice[voice].prio <= priority) {
      return voice;
    }
  }
  return -1;
}

static u32 voiceAllocateDo(s32 voice, u8 fxFlag) {
  s32 i;                // r30
  SYNTH_VOICELIST* sfv; // r31

  if (voice != -1) {
    if (voiceList[voice].user == 1) {
      sfv = &voiceList[voice];
      i = sfv->prev;
      if (i != VOICE_INDEX_NONE) {
        voiceList[i].next = sfv->next;
      } else {
        voiceListRoot = sfv->next;
      }
      i = sfv->next;
      if (i != VOICE_INDEX_NONE) {
        voiceList[i].prev = sfv->prev;
      }
      if (voice == voiceListInsert) {
        voiceListInsert = sfv->prev;
      }
      sfv->user = 0;
    } else if (synthVoice[voice].fxFlag != 0) {
      voiceFxRunning -= 1;
    } else {
      voiceMusicRunning -= 1;
    }
    if (fxFlag != 0) {
      voiceFxRunning++;
      return;
    }
    voiceMusicRunning++;
  }
}
#endif

u32 voiceAllocate(u8 priority, u8 maxVoices,
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
                  u32 allocId,
#else
                  u16 allocId,
#endif
                  u8 fxFlag) {
#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
  voiceAllocateDo(voiceAllocateFind(priority, maxVoices, allocId, fxFlag), fxFlag);
#else
  s32 i;                // r31
  s32 num;              // r26
  s32 voice;            // r30
  u16 p;                // r29
  u32 type_alloc;       // r25
  SYNTH_VOICELIST* sfv; // r27

  if (!synthIdleWaitActive) {
      if (fxFlag) {
        type_alloc = (voiceFxRunning >= synthInfo.maxSFX && synthInfo.voiceNum > synthInfo.maxSFX);

        if (synthInfo.maxSFX <= maxVoices) {
            goto _skip_alloc;
        }

        goto _do_alloc;
      } else {
        type_alloc = (voiceMusicRunning >= synthInfo.maxMusic && synthInfo.voiceNum > synthInfo.maxMusic);

        if (synthInfo.maxMusic <= maxVoices) {
            goto _skip_alloc;
        }

_do_alloc:
          num = 0;
          voice = -1;

          p = voicePrioSortRootListRoot;
          while (p != VOICE_PRIORITY_NONE &&  priority >= p && voice == -1) {
            for (i = voicePrioSortVoicesRoot[p]; i != VOICE_INDEX_NONE; i = voicePrioSortVoices[i].next) {
#if MUSY_VERSION <= MUSY_VERSION_CHECK(1, 5, 3)
              if (synthVoice[i].block)
                continue;
#endif
              if (allocId != synthVoice[i].allocId)
                  continue;
                ++num;
#if MUSY_VERSION >= MUSY_VERSION_CHECK(1, 5, 4)
                if(synthVoice[i].block)
                  continue;
#endif

                if (!type_alloc || fxFlag == synthVoice[i].fxFlag) {
                    if((synthVoice[i].cFlags & 2))
                        continue;
                    if (voice != -1) {
                        if(synthVoice[i].age < synthVoice[voice].age)
                            voice = i;
                    }
                    else
                        voice = i;

                }
            }

              p = voicePrioSortRootList[p].next;
          }
        }

      if (num < maxVoices) {
          while (p != VOICE_PRIORITY_NONE && num < maxVoices) {
              i = voicePrioSortVoicesRoot[p];
              while (i != VOICE_INDEX_NONE) {
#if MUSY_VERSION <= MUSY_VERSION_CHECK(1, 5, 3)
                if (!synthVoice[i].block) {
#endif
                    if (allocId == synthVoice[i].allocId) {
                        num++;
                    }
#if MUSY_VERSION <= MUSY_VERSION_CHECK(1, 5, 3)
                }
#endif

                    i = voicePrioSortVoices[i].next;
              }

              p = voicePrioSortRootList[p].next;
          }

        if (num < maxVoices) {
      _skip_alloc:
#if MUSY_VERSION <= MUSY_VERSION_CHECK(1, 5, 3)
        voice = -1;
#endif
            if (voiceListRoot != VOICE_INDEX_NONE && type_alloc == 0) {
                voice = voiceListRoot;
                goto _update;
            }

            if (priority < voicePrioSortRootListRoot) {
                return -1;
            }

#if MUSY_VERSION >= MUSY_VERSION_CHECK(1, 5, 4)
            voice = -1;
#endif
            p = voicePrioSortRootListRoot;

          while (p != VOICE_PRIORITY_NONE &&  priority >= p && voice == -1) {
            for (i = voicePrioSortVoicesRoot[p]; i != VOICE_INDEX_NONE; i = voicePrioSortVoices[i].next) {
              if (synthVoice[i].block != 0)
                  continue;

                if (!type_alloc || fxFlag == synthVoice[i].fxFlag) {
                    if((synthVoice[i].cFlags & 2))
                        continue;
                    if (voice != -1) {
                        if(synthVoice[voice].age > synthVoice[i].age)
                            voice = i;
                    }
                    else
                        voice = i;
                }
            }
                p = voicePrioSortRootList[p].next;
          }

              if (voice == -1) {
                return VOICE_ID_NONE;
              }

#if MUSY_VERSION <= MUSY_VERSION_CHECK(1, 5, 3)
      _update:
#endif
              if (synthVoice[voice].prio > priority) {
                  goto _fail;
              }
        }
      }

#if MUSY_VERSION >= MUSY_VERSION_CHECK(1, 5, 4)
  _update:
#endif
      if (voice == -1) {
          goto _fail;
      }

      if (voiceList[voice].user == 1) {
        sfv = voiceList + voice;
        i = sfv->prev;

        if (i != VOICE_INDEX_NONE) {
          voiceList[i].next = sfv->next;
        } else {
          voiceListRoot = sfv->next;
        }

        i = sfv->next;
        if (i != VOICE_INDEX_NONE) {
          voiceList[i].prev = sfv->prev;
        }

        if (voice == voiceListInsert) {
          voiceListInsert = sfv->prev;
        }

        sfv->user = 0;
      } else if (synthVoice[voice].fxFlag) {
        voiceFxRunning--;
      } else {
        voiceMusicRunning--;
      }
      if (fxFlag != FALSE) {
        ++voiceFxRunning;
      } else {
        ++voiceMusicRunning;
      }
      return voice;
  }

_fail:
  return -1;
#endif
}

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
int voiceAllocatePeek(u8 priority, u8 maxVoices, u32 allocId, u8 fxFlag, u32* currentAllocId) {
  s32 voice = voiceAllocateFind(priority, maxVoices, allocId, fxFlag);
  if (voice == -1) {
    return 0;
  }
  if (voiceList[voice].user == 1) {
    return 0;
  }
  *currentAllocId = synthVoice[voice].allocId;
  return 1;
}
#endif

void voiceFree(SYNTH_VOICE* svoice) {
  u32 i;                // r29
  SYNTH_VOICELIST* sfv; // r30
  i = 1;
  MUSY_ASSERT(svoice->id != VOICE_ID_NONE);
  macMakeInactive(svoice, MAC_STATE_STOPPED);
  voiceRemovePriority(svoice);
  svoice->addr = NULL;
  svoice->prio = 0;
  sfv = &voiceList[(i = svoice->id & VOICE_INDEX_MASK)];
  if (sfv->user == 0) {
    sfv->user = 1;
    if (voiceListRoot != VOICE_INDEX_NONE) {
      sfv->next = VOICE_INDEX_NONE;
      sfv->prev = voiceListInsert;
      voiceList[voiceListInsert].next = i;
    } else {
      sfv->next = VOICE_INDEX_NONE;
      sfv->prev = VOICE_INDEX_NONE;
      voiceListRoot = i;
    }

    voiceListInsert = i;
    if (svoice->fxFlag != 0) {
      --voiceFxRunning;
    } else {
      --voiceMusicRunning;
    }
  }

  svoice->id = VOICE_ID_NONE;
}

static void voiceInitFreeList() {
  u32 i; // r31

  for (i = 0; i < synthInfo.voiceNum; ++i) {
    voiceList[i].prev = i - 1;
    voiceList[i].next = i + 1;
    voiceList[i].user = 1;
  }

  voiceList[0].prev = VOICE_INDEX_NONE;
  voiceList[synthInfo.voiceNum - 1].next = VOICE_INDEX_NONE;
  voiceListRoot = 0;
  voiceListInsert = synthInfo.voiceNum - 1;
}

void synthInitAllocationAids() {
  voiceInitFreeList();
  voiceInitPrioSort();
  voiceFxRunning = 0;
  voiceMusicRunning = 0;
}

u32 voiceBlock(u8 prio) {
  u32 voice;

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
  voice = voiceAllocateFind(prio, VOICE_INDEX_NONE, VOICE_ALLOC_ID_LEGACY_NONE, 1);
  voiceAllocateDo(voice, 1);
#else
  voice = voiceAllocate(prio, VOICE_INDEX_NONE, VOICE_ALLOC_ID_LEGACY_NONE, 1);
#endif
  if (voice != (u32)-1) {
    synthVoice[voice].block = 1;
    synthVoice[voice].fxFlag = 1;

#if MUSY_VERSION >= MUSY_VERSION_CHECK(1, 5, 4)
    synthVoice[voice].allocId =
        MUSY_VERSION <= MUSY_VERSION_CHECK(2, 0, 1) ? VOICE_ALLOC_ID_LEGACY_NONE : -1;
#endif

    vidRemoveVoiceReferences(&synthVoice[voice]);
    synthVoice[voice].id = voice | VOICE_BLOCK_ID_MASK;

    if (hwIsActive(voice)) {
      hwBreak(voice);
    }

    macMakeInactive(&synthVoice[voice], MAC_STATE_STOPPED);
    synthVoice[voice].addr = NULL;
    voiceSetPriority(&synthVoice[voice], prio);
  }

  return voice;
}

void voiceUnblock(u32 voice) {
  if (voice == VOICE_ID_NONE) {
    return;
  }

  if (hwIsActive(voice)) {
    hwBreak(voice);
  }

  synthVoice[voice].id = voice;
  voiceFree(&synthVoice[voice]);
  synthVoice[voice].block = 0;
}

void voiceKill(u32 vi) {
  SYNTH_VOICE* sv = &synthVoice[vi]; // r31
  if (sv->addr != NULL) {
    vidRemoveVoiceReferences(sv);
    sv->cFlags &= ~3;
    sv->age = 0;
    voiceFree(sv);
  }

  if (sv->block != 0) {
    streamKill(vi);
  }

  hwBreak(vi);
}

s32 voiceKillSound(u32 voiceid) {
  s32 ret = -1;     // r29
  u32 next_voiceid; // r28
  u32 i;            // r30
  if (sndActive != FALSE) {
    for (voiceid = vidGetInternalId(voiceid); voiceid != -1; voiceid = next_voiceid) {
      i = voiceid & VOICE_INDEX_MASK;
      next_voiceid = synthVoice[i].child;
      if (voiceid == synthVoice[i].id) {
        voiceKill(i);
        ret = 0;
      }
    }
  }

  return ret;
}

void synthKillAllVoices(unsigned char musiconly) {
  u32 i;

  for (i = 0; i < synthInfo.voiceNum; ++i) {
    if (synthVoice[i].addr != NULL) {
      if (musiconly == 0 || (musiconly != 0 && synthVoice[i].fxFlag == 0)) {
        voiceKill(i);
      }
    } else {
      voiceKill(i);
    }
  }
}

void synthKillVoicesByMacroReferences(u16* ref) {
  u32 i;  // r31
  u16 id; // r29

  for (i = 0; i < synthInfo.voiceNum; ++i) {
    if (synthVoice[i].addr == NULL && synthVoice[i].block == 0) {
      voiceKill(i);
    }
  }

  while (*ref != SAMPLE_REFERENCE_END) {
    if ((*ref & SAMPLE_REFERENCE_RANGE_FLAG)) {
      id = *ref & SAMPLE_REFERENCE_ID_MASK;
      while (id <= ref[1]) {
        for (i = 0; i < synthInfo.voiceNum; ++i) {
          if (synthVoice[i].addr != NULL && id == synthVoice[i].macroId) {
            voiceKill(i);
          }
        }
        ++id;
      }
      ref += 2;
    } else {
      for (i = 0; i < synthInfo.voiceNum; ++i) {
        if (synthVoice[i].addr != NULL && *ref == synthVoice[i].macroId) {
          voiceKill(i);
        }
      }
      ++ref;
    }
  }
}

#if MUSY_VERSION >= MUSY_VERSION_CHECK(2, 0, 1)
void synthKillVoicesBySampleReferences(u16* ref) {
  u32 i;  // r31
  u16 id; // r29

  for (i = 0; i < synthInfo.voiceNum; i++) {
    if (synthVoice[i].addr == 0 && synthVoice[i].block == 0) {
      voiceKill(i);
    }
  }
  while (ref[0] != SAMPLE_REFERENCE_END) {
    if ((ref[0] & SAMPLE_REFERENCE_RANGE_FLAG) != 0) {
      for (id = ref[0] & SAMPLE_REFERENCE_ID_MASK; id <= ref[1]; id++) {
        for (i = 0; i < synthInfo.voiceNum; i++) {
          if (synthVoice[i].addr != 0 && (id == synthVoice[i].sampleId)) {
            voiceKill(i);
          }
        }
      }
      ref += 2;
    } else {
      for (i = 0; i < synthInfo.voiceNum; i++) {
        if (synthVoice[i].addr != 0 && (ref[0] == synthVoice[i].sampleId)) {
          voiceKill(i);
        }
      }
      ref++;
    }
  }
}
#endif

u32 voiceIsLastStarted(SYNTH_VOICE* svoice) {
  u32 i; // r31

  if (svoice->id != VOICE_ID_NONE && svoice->midi != VOICE_MIDI_NONE) {
    i = svoice->id & VOICE_INDEX_MASK;
    if (svoice->midiSet == VOICE_MIDI_NONE) {
      if (synth_last_fxstarted[i] == i) {
        return TRUE;
      }
    } else if (synth_last_started[svoice->midiSet][svoice->midi] == i) {
      return TRUE;
    }
  }

  return FALSE;
}

void voiceSetLastStarted(SYNTH_VOICE* svoice) {
  u32 i; // r31

  if (svoice->id != VOICE_ID_NONE && svoice->midi != VOICE_MIDI_NONE) {
    i = svoice->id & VOICE_INDEX_MASK;
    if (svoice->midiSet == VOICE_MIDI_NONE) {
      synth_last_fxstarted[i] = i;
    } else {
      synth_last_started[svoice->midiSet][svoice->midi] = i;
    }
  }
}

void voiceResetLastStarted(struct SYNTH_VOICE* svoice) {
  u32 i;

  if ((svoice->id != VOICE_ID_NONE) && (svoice->midi != VOICE_MIDI_NONE)) {
    i = svoice->id & VOICE_INDEX_MASK;
    if (svoice->midiSet == VOICE_MIDI_NONE) {
      if (synth_last_fxstarted[i] == i) {
        synth_last_fxstarted[i] = VOICE_MIDI_NONE;
      }
    } else if (i == synth_last_started[svoice->midiSet][svoice->midi]) {
      synth_last_started[svoice->midiSet][svoice->midi] = VOICE_MIDI_NONE;
    }
  }
}

void voiceInitLastStarted() {
  u32 i;
  u32 j;

  for (i = 0; i < 8; ++i) {
    for (j = 0; j < 16; ++j) {
      synth_last_started[i][j] = VOICE_MIDI_NONE;
    }
  }

  for (j = 0; j < 64; ++j) {
    synth_last_fxstarted[j] = VOICE_MIDI_NONE;
  }
}
