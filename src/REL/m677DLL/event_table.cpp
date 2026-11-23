#include "REL/m677DLL/event_record.h"
#include "msm_se.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
// Supplies two result message IDs and the common host motions and sound cues.
#define KOOPA_EVENT(number, first, second, label) \
    {number, 2, {MESSNUM(MESS_MESSAGE_677, first), MESSNUM(MESS_MESSAGE_677, second)}, \
     DATANUM(_m677 << 16, 0x5), DATANUM(_m677 << 16, 0x5), 0, MSM_SE_GUIDE_KOOPA_TALK, \
     MSM_SE_GUIDE_KOOPA_ANNOUNCE, MSM_SE_GUIDE_KOOPA_TALK, label}

// Lists the board events, their result message IDs, and their host motions and sound cues.
static const EventRecord events[] = {
    KOOPA_EVENT(1, 0x5, 0x6, "Ev01: Golden Koopa"),
    KOOPA_EVENT(2, 0x7, 0x8, "Ev02: Stage rotation"),
    KOOPA_EVENT(3, 0x9, 0xA, "Ev03: Taru falls"),
    KOOPA_EVENT(4, 0xB, 0xC, "Ev04: Dossun falls"),
    KOOPA_EVENT(5, 0xD, 0xE, "Ev05: Jump up!"),
    KOOPA_EVENT(6, 0xF, 0x10, "Ev06: Tarai falls"),
    KOOPA_EVENT(7, 0x11, 0x12, "Ev07: Pitfall"),
    KOOPA_EVENT(8, 0x13, 0x14, "Ev08: Togezoh falls"),
    {9, 1, {MESSNUM(MESS_MESSAGE_677, 0x15), MESSNUM(MESS_MESSAGE_677, 0x15)},
     DATANUM(_m677 << 16, 0xB), DATANUM(_m677 << 16, 0x6), 0, -1,
     MSM_SE_GUIDE_KOOPA_REACT, MSM_SE_GUIDE_KOOPA_ANNOUNCE, "Ev09: Meteoric fall"},
    KOOPA_EVENT(10, 0x16, 0x17, "Ev10: Bubble falls"),
    KOOPA_EVENT(11, 0x18, 0x19, "Ev11: Bomb-hey falls"),
    KOOPA_EVENT(12, 0x1A, 0x1B, "Ev12: Wan-wan chase"),
    KOOPA_EVENT(13, 0x1C, 0x1D, "Ev13: Coins stolen"),
    {14, 2, {MESSNUM(MESS_MESSAGE_677, 0x1E), MESSNUM(MESS_MESSAGE_677, 0x1F)},
     DATANUM(_m677 << 16, 0x8), DATANUM(_m677 << 16, 0xC), 0, MSM_SE_GUIDE_KOOPA_LAUGH,
     MSM_SE_GUIDE_KOOPA_ANNOUNCE, MSM_SE_GUIDE_KOOPA_REACT, "Ev14: Coins get!"},
    {15, 2, {MESSNUM(MESS_MESSAGE_677, 0x20), MESSNUM(MESS_MESSAGE_677, 0x21)},
     DATANUM(_m677 << 16, 0x8), DATANUM(_m677 << 16, 0xC), 0, MSM_SE_GUIDE_KOOPA_LAUGH,
     MSM_SE_GUIDE_KOOPA_ANNOUNCE, MSM_SE_GUIDE_KOOPA_REACT, "Ev15: Kinoko get"},
    {16, 1, {MESSNUM(MESS_MESSAGE_677, 0x22), MESSNUM(MESS_MESSAGE_677, 0x22)},
     DATANUM(_m677 << 16, 0x4), DATANUM(_m677 << 16, 0x4), 100, -1,
     MSM_SE_GUIDE_KOOPA_QUIZ_JEER, -1, "Ev16: Koopa falls"},
    {17, 1, {MESSNUM(MESS_MESSAGE_677, 0x23), MESSNUM(MESS_MESSAGE_677, 0x23)},
     DATANUM(_m677 << 16, 0xB), DATANUM(_m677 << 16, 0x6), 0, -1,
     MSM_SE_GUIDE_KOOPA_QUIZ_JEER, MSM_SE_GUIDE_KOOPA_ANNOUNCE, "Ev17: Koopa-bomb"},
};
// Index of the event selected for the board.
static int current_event = -1;
static const int events_num = sizeof(events) / sizeof(events[0]);
// Returns the event record at the given index.
const EventRecord *getEventRecord(int index) { return &events[index]; }
// Returns the record for the selected event.
const EventRecord *getSelectedEventRecord() {
    assert((current_event >= 0) && (current_event < events_num));
    return &events[current_event];
}

// Selects an event by its record index.
void selectEventRecord(int index)
{
    current_event = index;
}

#undef KOOPA_EVENT
