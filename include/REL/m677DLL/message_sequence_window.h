// Controls the event message window and its sprites.

#ifndef REL_SEER_TERROR_MESSAGE_SEQUENCE_WINDOW_H
#define REL_SEER_TERROR_MESSAGE_SEQUENCE_WINDOW_H

#include "dolphin/types.h"
#include "REL/framework/frame_interpolator.h"
#include <vector>

typedef std::vector<u32> MessageSequence;
// Tracks the current message window and its animation state.
struct MessageSequenceWindow {
    typedef void (MessageSequenceWindow::*State)();
    explicit MessageSequenceWindow(u32);
    explicit MessageSequenceWindow(const MessageSequence &);
    ~MessageSequenceWindow();
    State state;
    // Selects the next state handler.
    void setState(State next) { state = next; }
    s16 window;
    MessageSequence messages;
    u8 closing;
    FrameInterpolator<f32> transition;
    void setDisplayedMessage(unsigned long message);
    bool run();
    int queuedMessageCount() const;
    void beginClosing();
    void prepareWindowOpening();
    void rotateWindowOpen();
    void revealMessageSprite();
    void waitForMessage();
    void prepareWindowClosing();
    void shrinkMessageSprite();
    void rotateWindowClosed();
};

void setBlockedPadMask(int playerNo);
void createWindowFrame(MessageSequenceWindow *owner);

#endif
