#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"

#include "REL/m639DLL/mode_game.h"

#include "REL/m639DLL/participant_owner.h"
#include "REL/framework/sound.h"

extern "C" {
#include "game/data.h"
}
#include <dolphin/types.h>


// The two-sided gauge display: its animations, the transition that fills it and each side's sprites.

extern "C" { extern float lbl_1_bss_C8; }

extern "C" { void __msl_assertion_failed(const char *, const char *, int); }

extern "C" { void fn_1_C858(); }

void fn_1_4078(void *sprite);

extern "C" { void *HuSprAnimRead(void *resource); }

extern "C" { int HuSprGrpCreate(short count); }

extern "C" { int HuSprFuncCreate(void (*callback)(void *), short priority); }

extern "C" { void HuSprGrpMemberSet(short group, short index, short function); }

extern "C" { int fn_1_67B4(unsigned int playerNumber); }

extern "C" { double fmod(double, double); }

extern "C" const int lbl_1_rodata_200[0x16] = {
    0x62000C, 0x62000D, 0x62000E, 0x62000F, 0x620010, 0x620011, 0x620012, 0x620013, 0x620014, 0x620015, 0x620016, 0x620017, 0x620018, 0x620019, 0x62001A, 0x62001B, 0x62001C, 0x62001D, 0x62001E, 0x62001F, 0x620020, 0x620021
};

ParticipantOwner::ParticipantOwner() : active(0)
{
    for (int animationIndex = 0; animationIndex < 0x16; ++animationIndex) {
        collectedParticipants.push_back((ParameterResource *)(HuSprAnimRead(fn_80008DD0(lbl_1_rodata_200[animationIndex], 0x10000000, 2))));
    }

    int group = HuSprGrpCreate(1);
    int function = HuSprFuncCreate(fn_1_4078, 0x65);
    HuSprGrpMemberSet((short)group, 0, (short)function);

    for (int index = 0; index < 2; ++index) {
        participants[index].reset(new ParticipantSetup(index));
    }
}

ParticipantOwner::~ParticipantOwner() {}

// Starts or disables the transition, then clears the handled event's mask bit.
void ParticipantOwner::flagEnabled(int event)
{
    switch (event) {
    case 2:
        transition.fn_1_50AC(1.0f, lbl_1_bss_84);
        active = 1;
        break;
    case 3:
        active = 0;
        break;
    }
    flags &= ~(1UL << event);
}

// Updates the transition and runs each participant's active and counter updates.
void ParticipantOwner::update()
{
    transition.fn_1_5124();
    if (active) {
        for (int index = 0; index < 2; index++) {
            participants[index]->fn_1_4A2C();
        }
    }
    for (int index = 0; index < 2; index++) {
        participants[index]->fn_1_4F44();
    }
}

// Obtains the current owner and runs its update callback.
void fn_1_4078(void *)
{
    ParticipantOwner::getChecked().fn_1_40EC();
}

// Publishes the sampled scalar before updating both participants.
void ParticipantOwner::fn_1_40EC()
{
    float value = transition.current;
    lbl_1_bss_C8 = value;
    fn_1_C858();
    for (int index = 0; index < 2; index++) {
        participants[index]->draw();
    }
}

ParticipantSetup::ParticipantSetup(int index)
    : participantIndex(index), firstPlayerIndex(index * 2), secondPlayerIndex(index * 2 + 1),
      primary(ParticipantOwner::getChecked().collectedParticipants[index], 0),
      first(ParticipantOwner::getChecked().collectedParticipants[fn_1_67B4(firstPlayerIndex) + 7],
            ParticipantOwner::getChecked().collectedParticipants[21], 0),
      second(ParticipantOwner::getChecked().collectedParticipants[fn_1_67B4(secondPlayerIndex) + 7],
             ParticipantOwner::getChecked().collectedParticipants[21], 0)
{
    primary.centerX = (index == 0) ? 17.0f + primary.halfWidth : 623.0f - primary.halfWidth;
    primary.centerY = 40.0f + primary.halfHeight;
    primary.opacity = 1.0f;
    float firstOffset = primary.halfWidth - 30.0f;
    float secondOffset = primary.halfWidth - 31.0f;
    first.centerX = primary.centerX - firstOffset;
    first.centerY = primary.centerY;
    first.opacity = 1.0f;
    second.centerX = primary.centerX + secondOffset;
    second.centerY = primary.centerY;
    second.opacity = 1.0f;
    for (int item = 0; item < 15; ++item) {
        int firstIndex = firstPlayerIndex + 2;
        int secondIndex = secondPlayerIndex + 2;
        sprites[0][item].reset(new ResourceParameters(ParticipantOwner::getChecked().collectedParticipants[firstIndex], 1));
        sprites[1][item].reset(new ResourceParameters(ParticipantOwner::getChecked().collectedParticipants[secondIndex], 1));
        float offset = -63.0f + 9.0f * item;
        sprites[0][item]->centerX = primary.centerX + offset;
        sprites[0][item]->centerY = primary.centerY;
        sprites[0][item]->opacity = 0.0f;
        sprites[1][item]->centerX = primary.centerX - offset;
        sprites[1][item]->centerY = primary.centerY;
        sprites[1][item]->opacity = 0.0f;
    }
    for (int item = 0; item < 2; ++item) {
        markers[item].reset(new ResourceParameters(ParticipantOwner::getChecked().collectedParticipants[6], 1));
        markers[item]->opacity = 0.0f;
    }
}

extern "C" { extern const int lbl_1_rodata_288[2] = { 0x76F, 0x770 }; }

void ParticipantSetup::fn_1_4A2C()
{
    float originalProgress[2][15];
    for (int group = 0; group < 2; ++group) {
        for (int index = 0; index < 15; ++index) {
            originalProgress[group][index] = sprites[group][index]->opacity;
        }
    }

    float firstTime = ModeGame::getChecked().fn_1_1CC(firstPlayerIndex);
    float secondTime = ModeGame::getChecked().fn_1_1CC(secondPlayerIndex);

    bool hasBothSteps = firstTime + secondTime >= 1.0;
    int firstCount = (int)(firstTime / 0.06666667f);
    int secondCount = (int)(secondTime / 0.06666667f);
    float firstFraction = (float)(fmod((double)firstTime, 0.06666667f) /
                                   0.06666667f);
    float secondFraction = (float)(fmod((double)secondTime, 0.06666667f) /
                                    0.06666667f);

    for (int index = 0; index < firstCount; ++index) {
        sprites[0][index]->opacity = 1.0f;
    }
    for (int index = 0; index < secondCount; ++index) {
        sprites[1][index]->opacity = 1.0f;
    }

    if (hasBothSteps) {
        double firstProgress;
        if (firstFraction >= secondFraction) {
            firstProgress = 1.0;
        } else {
            firstProgress = 0.0;
        }
        sprites[0][firstCount]->opacity = (float)firstProgress;

        double secondProgress;
        if (secondFraction > firstFraction) {
            secondProgress = 1.0;
        } else {
            secondProgress = 0.0;
        }
        sprites[1][secondCount]->opacity = (float)secondProgress;
    } else {
        sprites[0][firstCount]->opacity = 0.0f;
        sprites[1][secondCount]->opacity = 0.0f;
    }

    for (int index = firstCount + 1; index < 15; ++index) {
        sprites[0][index]->opacity = 0.0f;
    }
    for (int index = secondCount + 1; index < 15; ++index) {
        sprites[1][index]->opacity = 0.0f;
    }

    for (int group = 0; group < 2; ++group) {
        for (int index = 0; index < 15; ++index) {
            if (originalProgress[group][index] == sprites[group][index]->opacity) {
                continue;
            } else {
                markers[group]->textureIndex = -1;
                markers[group]->centerX = sprites[group][index]->centerX;
                markers[group]->centerY = sprites[group][index]->centerY;
                markers[group]->opacity = 1.0f;
                soundPlay(lbl_1_rodata_288[participantIndex]);
                break;
            }
        }
    }
}

void ParticipantSetup::fn_1_4F44()
{
    for (int group = 0; group < 2; group++) {
        if (markers[group]->opacity >= 1.0f) {
            if (++markers[group]->textureIndex >= 8) {
                markers[group]->opacity = 0.0f;
            }
        }
    }
}

/* Draws each participant's primary sprites, grid, and two auxiliary sprites. */
void ParticipantSetup::draw()
{
    primary.draw();
    first.draw();
    second.draw();
    for (int column = 0; column < 15; column++) {
        for (int row = 0; row < 2; row++) {
            sprites[row][column]->draw();
        }
    }
    for (int index = 0; index < 2; index++) {
        markers[index]->draw();
    }
}






extern "C" { s32 lbl_1_bss_78 = minutesToFrames(5L); }

extern "C" { s32 lbl_1_bss_7C = secondsToFrames(30L); }

extern "C" { float lbl_1_bss_80 = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f)); }

extern "C" { int lbl_1_bss_84 = (int)secondsToFrames(0.2); }
