// Defines the event records and access to the selected event.

#ifndef REL_SEER_TERROR_EVENT_RECORD_H
#define REL_SEER_TERROR_EVENT_RECORD_H

// Holds an event's result message IDs and the host motions and cues around the message.
struct EventRecord {
    // Event number used to select the event callback.
    int eventNumber;
    // Number of result messages, also used as the weight for event selection.
    int resultMessageCount;
    // Result message IDs. The board displays one for the selected event.
    unsigned int resultMessages[2];
    // Host motion before the result message. Motion 0xB skips the shift.
    unsigned int beforeResultMessageMotion;
    // Host motion after the result message.
    int afterResultMessageMotion;
    // Starting frame of the host motion after the result message.
    int afterResultMessageMotionStart;
    // Sound cue for the motion before the result message. A value of -1 suppresses it.
    int beforeResultMessageMotionSound;
    // Sound cue played between the two host motions.
    int betweenMotionSound;
    // Sound cue for the motion after the result message. A value of -1 suppresses it.
    int afterResultMessageMotionSound;
    // Event label. No code names this field.
    const char *unk28;
};

const EventRecord *getEventRecord(int index);
const EventRecord *getSelectedEventRecord();
void selectEventRecord(int index);

#endif
