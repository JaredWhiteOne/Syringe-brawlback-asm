#pragma once
#include "types.h"
#include "EXI_hooks.h"
#include "OS/OSInterrupt.h"
#include "Wii/OS/OSInterrupt.h"

#include "Wii/PAD/PADStatus.h"
#include "Brawl/GF/GameFrame.h"
#include "Brawl/GF/gfPadSystem.h"
#include "Brawl/FT/ftManager.h"
#include <Brawl/GF/GameGlobal.h>
#include "Brawl/gmGlobalModeMelee.h"
#include "Brawl/GF/gfApplication.h"
#include "Brawl/SC/scMelee.h"
#include "mtRand.h"
#include "OS/OSTime.h"
#include "ExiStructures.h"

inline void updateGamePadSystem() { updateGame(PAD_SYSTEM); }

u32 getCurrentFrame();

void fillOutGameSettings(GameSettings& settings);
void MergeGameSettingsIntoGame(GameSettings& settings);
extern bool canRollback;
extern u32 frameCounter;
extern bool shouldTrackAllocs;
extern bool doDumpList;
extern bool isRollback;
extern u32 frameCounter;
namespace FrameLogic {
    void FrameDataLogic(u32 currentFrame);
    void SaveState(u32 frame);
    void GetInputsForFrame(u32 frame, FrameData* inputs);
    void FixFrameDataEndianness(FrameData* fd);
}
namespace FrameAdvance {
    u32 getFramesToAdvance();
}

// TODO: put this in the submodule and pack it
struct GameReport {
    f64 damage[MAX_NUM_PLAYERS];
    s32 stocks[MAX_NUM_PLAYERS];
    s32 frame_duration;
};