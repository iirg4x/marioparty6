// Controls the event message window and its sprites.

#include "messdir_enum.h"
#include "REL/m677DLL/route_runner.h"
#include "REL/framework/range.h"
#include "REL/m677DLL/game_constants.h"

#include <new>

#include "REL/m677DLL/message_sequence_window.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "humath.h"

extern "C" {
#include <math.h>
#include "game/window.h"
#include "game/gamework.h"
}

#include <vector>

extern "C" {
extern unsigned char blockedPadMask;
}

// Mask of controller pads that cannot advance the message.
unsigned char blockedPadMask;

// Selects the input pad used by the message window.
void setBlockedPadMask(int playerNo)
{
    signed char padNo;

    blockedPadMask = 0;
    if (lbl_80265728[playerNo].type == 0) {
        padNo = lbl_80265728[playerNo].padNo;
        if (padNo >= 0 && padNo < 4) {
            blockedPadMask = ~(1 << padNo) & 0xF;
        }
    }
}

// Creates a window for one message ID.
MessageSequenceWindow::MessageSequenceWindow(u32 value) : messages(1, value), transition(0.0f)
{
    createWindowFrame(this);
}

// Creates a message window with the supplied message queue.
MessageSequenceWindow::MessageSequenceWindow(const MessageSequence &value)
    : messages(value), transition(0.0f)
{
    createWindowFrame(this);
}

// Destroys the underlying window.
MessageSequenceWindow::~MessageSequenceWindow()
{
    HuWinExKill(window);
}

// Sizes and positions the event message window.
void createWindowFrame(MessageSequenceWindow *owner)
{
    owner->closing = 0;
    HuVec2f size;
    HuWinMesMaxSizeBetGet(&size, MESSNUM(MESS_MESSAGE_677, 0x0), MESSNUM(MESS_MESSAGE_677, 0x23));
    size.y = rangeMaximum(88.0f, size.y);

    int frameYInteger = (int) ((480.0f - size.y) - 48.0f);
    owner->window =
        HuWinExCreateFrame(-10000.0f, (float) frameYInteger, (short) size.x, (short) size.y, 13, 1);
    HuWinDisablePlayerSet(owner->window, blockedPadMask);
    owner->setState(&MessageSequenceWindow::prepareWindowOpening);
}

// Sets the displayed message.
void MessageSequenceWindow::setDisplayedMessage(unsigned long message)
{
    HuWinMesSet(window, message);
}

// Returns the number of queued messages.
int MessageSequenceWindow::queuedMessageCount() const
{
    return messages.size();
}

// Runs the current window state and reports whether it will continue.
bool MessageSequenceWindow::run()
{
    if (state) {
        (this->*state)();
    }
    return state != 0;
}

// Clears the message queue and starts closing the window.
void MessageSequenceWindow::beginClosing()
{
    if (this->closing == 0) {
        this->closing = 1;
        this->messages.clear();
        HuWinHomeClear(this->window);
        this->setState(&MessageSequenceWindow::prepareWindowClosing);
    }
}

// Sets up the window sprites for the opening animation.
void MessageSequenceWindow::prepareWindowOpening()
{
    HUWIN *windowData = &winData[this->window];
    s16 *thirdSprite = &windowData->sprId[3];
    s16 *firstSprite = &windowData->sprId[0];
    s16 *secondSprite = &windowData->sprId[1];

    HuWinHomeClear(this->window);
    this->transition.set(90.0f, 0.0f, 10);
    HuSpr3DRotSet((s32) *firstSprite, 90.0f, 0.0f, 0.0f);
    if (windowData->animFrame[1]) {
        HuSpr3DRotSet((s32) *secondSprite, 90.0f, 0.0f, 0.0f);
    }
    if (*thirdSprite != -1) {
        HuSpr3DRotSet((s32) *thirdSprite, 90.0f, 0.0f, 0.0f);
    }
    HuWinDispOn(this->window);
    this->setState(&MessageSequenceWindow::rotateWindowOpen);
}

// Advances the rotation of the opening window frame.
void MessageSequenceWindow::rotateWindowOpen()
{
    HUWIN *windowData = &winData[this->window];
    const s16 *firstSpriteId = &windowData->sprId[0];
    const s16 *secondSpriteId = &windowData->sprId[1];
    f32 rotation = this->transition.update();
    HuSpr3DRotSet(*firstSpriteId, rotation, 0.0f, 0.0f);
    if (windowData->animFrame[1] != 0) {
        HuSpr3DRotSet(*secondSpriteId, rotation, 0.0f, 0.0f);
    }
    if (this->transition.isFinished()) {
        this->transition.set(90.0f, 0.0f, 10);
        setState(&MessageSequenceWindow::revealMessageSprite);
    }
}

// Animates the message sprite before showing its contents.
void MessageSequenceWindow::revealMessageSprite()
{
    HUWIN *windowData = &winData[this->window];
    const s16 *spriteId = &windowData->sprId[3];
    if (*spriteId != -1) {
        f32 angle = this->transition.update();
        HuSpr3DRotSet(*spriteId, -3.0f * angle, 0.0f, 0.0f);
        f32 scale = 0.5 * (1.0 - sin((M_PI * angle) / 180.0)) + 0.5;
        HuSprScaleSet(windowData->grpId, 3, scale, scale);
        if (!this->transition.isFinished())
            return;
    }
    setDisplayedMessage(messages.front());
    this->setState(&MessageSequenceWindow::waitForMessage);
}

// Removes a completed message and selects the next message if present.
void MessageSequenceWindow::waitForMessage()
{
    bool isActive;
    isActive = HuWinStatGet(window) != 0;
    if (isActive)
        return;
    messages.erase(messages.begin());
    if (!messages.empty()) {
        setDisplayedMessage(messages.front());
    } else {
        beginClosing();
    }
}

// Starts the closing animation for the message sprite.
void MessageSequenceWindow::prepareWindowClosing()
{
    HUWIN *windowData = &winData[this->window];
    s16 *thirdSprite = &windowData->sprId[3];
    if (*thirdSprite != -1) {
        this->transition.set(0.0f, 90.0f, 5);
        this->setState(&MessageSequenceWindow::shrinkMessageSprite);
    } else {
        this->transition.set(0.0f, 90.0f, 10);
        this->setState(&MessageSequenceWindow::rotateWindowClosed);
    }
}

// Shrinks the message sprite during the closing animation.
void MessageSequenceWindow::shrinkMessageSprite()
{
    f32 angle = this->transition.update();
    f32 scale = 1.0 - sin((M_PI * angle) / 180.0);
    HUWIN *windowData = &winData[this->window];
    HuSprScaleSet(windowData->grpId, 3, scale, scale);
    if (this->transition.isFinished()) {
        this->transition.set(0.0f, 90.0f, 10);
        this->setState(&MessageSequenceWindow::rotateWindowClosed);
    }
}

// Rotates the window frame closed.
void MessageSequenceWindow::rotateWindowClosed()
{
    HUWIN *windowData = &winData[this->window];
    const s16 *firstSpriteId = &windowData->sprId[0];
    const s16 *secondSpriteId = &windowData->sprId[1];
    f32 rotation = this->transition.update();
    HuSpr3DRotSet(*firstSpriteId, rotation, 0.0f, 0.0f);
    if (windowData->animFrame[1] != 0) {
        HuSpr3DRotSet(*secondSpriteId, rotation, 0.0f, 0.0f);
    }
    if (this->transition.isFinished()) {
        HuWinDispOff(this->window);
        setState(0);
    }
}
