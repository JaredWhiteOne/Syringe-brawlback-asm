#include "Rollback_Hooks.h"
#include "sy_core.h"
#include <EXI/EXIBios.h>
#include <gf/gf_heap_manager.h>
#include <modules.h>
#define P1_CHAR_ID_IDX 0x98
#define P2_CHAR_ID_IDX P1_CHAR_ID_IDX + 0x5C
#define P3_CHAR_ID_IDX P2_CHAR_ID_IDX + 0x5C
#define P4_CHAR_ID_IDX P3_CHAR_ID_IDX + 0x5C
u32 frameCounter = 0;
bool shouldTrackAllocs = false;
bool doDumpList = false;
bool isRollback = false;

u32 getCurrentFrame() {
    return frameCounter;
}

bool gameHasStarted() {
    return reinterpret_cast<scMelee*>(gfSceneManager::getInstance()->searchScene("scMelee"))->m_operatorRuleGameMode->isEnd() != 0;
}

void fillOutGameSettings(GameSettings& settings) {
    settings.randomSeed = g_mtRandDefault->seed;
    settings.stageID = g_gmGlobalModeMelee->m_meleeInitData.m_stageID;

    u8 p1_id = *(((u8*)g_gmGlobalModeMelee)+P1_CHAR_ID_IDX);
    OSReport("P1 pre-override char id: %u\n", (unsigned int)p1_id);
    
    u8 p2_id = *(((u8*)g_gmGlobalModeMelee)+P2_CHAR_ID_IDX);
    OSReport("P2 pre-override char id: %u\n", (unsigned int)p2_id);

    // brawl loads all players into the earliest slots.
    // I.E. if players choose P1 and P3, they will get loaded into P1 and P2
    // this means we can use the number of players in a match to iterate over
    // players since we know they'll always be next to each other

    // TODO: replace this with some way to get the actual number of players in a match.
    // unfortunately FIGHTER_MANAGER->getEntryCount() isn't filled out at this point in the game loading
    // sequence. Gotta find another way to get it, or some better spot to grab the number of players
    settings.numPlayers = 2;
    OSReport("Num Players: %u\n", (unsigned int)settings.numPlayers);
    PlayerSettings* playerSettings = new PlayerSettings[settings.numPlayers];
    playerSettings[0].charID = p1_id;
    playerSettings[1].charID = p2_id;
    
    for (int i = 0; i < settings.numPlayers; i++) {
        settings.playerSettings[i] = playerSettings[i];
    }
}


// take gamesettings and apply it to our game
void MergeGameSettingsIntoGame(GameSettings& settings) {
    //DEFAULT_MT_RAND->seed = settings->randomSeed;
    g_mtRandDefault->seed = 0x496ffd00; // hardcoded for testing (arbitrary number)
    g_mtRandOther->seed = 0x496ffd00;

    //GM_GLOBAL_MODE_MELEE->stageID = settings->stageID;
    //GM_GLOBAL_MODE_MELEE->stageID = 2;

    Netplay::localPlayerIdx = settings.localPlayerIdx;
    OSReport("Local player index is %u\n", (unsigned int)Netplay::localPlayerIdx);

    u8 p1_char = settings.playerSettings[0].charID;
    u8 p2_char = settings.playerSettings[1].charID;
    OSReport("P1 char: %u  P2 char: %u\n", (unsigned int)p1_char, (unsigned int)p2_char);
    OSReport("Stage id: %u\n", (unsigned int)settings.stageID);

    int chars[MAX_NUM_PLAYERS] = {p1_char, p2_char, -1, -1};
    GMMelee::PopulateMatchSettings(chars, settings.stageID );
}

namespace Util {

    void printInputs(const BrawlbackPad& pad) {
        OSReport(" -- Pad --\n");
        OSReport("StickX: %hhu ", pad.stickX);
        OSReport("StickY: %hhu ", pad.stickY);
        OSReport("CStickX: %hhu ", pad.cStickX);
        OSReport("CStickY: %hhu\n", pad.cStickY);
        OSReport("Buttons: ");
        OSReport("Buttons: 0x%x", pad.buttons);
        OSReport("holdButtons: 0x%x", pad.holdButtons);
        OSReport(" LTrigger: %u    RTrigger %u\n", pad.LTrigger, pad.RTrigger);
        //OSReport(" ---------\n"); 
    }

    void printGameInputs(const gfPadStatus& pad) {
        OSReport(" -- Pad --\n");
        OSReport(" LAnalogue: %u    RAnalogue %u\n", pad.LAnalogue, pad.RAnalogue);
        OSReport("StickX: %hhu ", pad.stickX);
        OSReport("StickY: %hhu ", pad.stickY);
        OSReport("CStickX: %hhu ", pad.cStickX);
        OSReport("CStickY: %hhu\n", pad.cStickY);
        OSReport("Buttons: ");
        OSReport("B1: 0x%x ", pad.buttons.bits);
        OSReport("B2: 0x%x ", pad._buttons.bits);
        OSReport("B3: 0x%x \n", pad.newPressedButtons.bits);
        OSReport(" LTrigger: %u    RTrigger %u\n", pad.LTrigger, pad.RTrigger);
        //OSReport(" ---------\n"); 
    }

    void printFrameData(const FrameData& fd) {
        for (int i = 1; i < MAX_NUM_PLAYERS; i++) {
            OSReport("Frame %u pIdx %u\n", fd.playerFrameDatas[i].frame, (unsigned int)fd.playerFrameDatas[i].playerIdx);
            printInputs(fd.playerFrameDatas[i].pad);
        }
    }

    void SyncLog(const BrawlbackPad& pad, u8 playerIdx) {
        OSReport("[Sync] Injecting inputs for player %u on frame %u\n", (unsigned int)playerIdx, getCurrentFrame());
        printInputs(pad);
        OSReport("[/Sync]\n");
    }

    void FixFrameDataEndianness(FrameData* fd) {
        utils::swapByteOrder(fd->randomSeed);
        for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
            utils::swapByteOrder(fd->playerFrameDatas[i].frame);
            utils::swapByteOrder(fd->playerFrameDatas[i].syncData.anim);
            utils::swapByteOrder(fd->playerFrameDatas[i].syncData.locX);
            utils::swapByteOrder(fd->playerFrameDatas[i].syncData.locY);
            utils::swapByteOrder(fd->playerFrameDatas[i].syncData.percent);
            utils::swapByteOrder(fd->playerFrameDatas[i].pad._buttons);
            utils::swapByteOrder(fd->playerFrameDatas[i].pad.buttons);
            utils::swapByteOrder(fd->playerFrameDatas[i].pad.holdButtons);
            utils::swapByteOrder(fd->playerFrameDatas[i].pad.releasedButtons);
            utils::swapByteOrder(fd->playerFrameDatas[i].pad.rapidFireButtons);
            utils::swapByteOrder(fd->playerFrameDatas[i].pad.newPressedButtons);
            utils::swapByteOrder(fd->playerFrameDatas[i].sysPad._buttons);
            utils::swapByteOrder(fd->playerFrameDatas[i].sysPad.buttons);
            utils::swapByteOrder(fd->playerFrameDatas[i].sysPad.holdButtons);
            utils::swapByteOrder(fd->playerFrameDatas[i].sysPad.releasedButtons);
            utils::swapByteOrder(fd->playerFrameDatas[i].sysPad.rapidFireButtons);
            utils::swapByteOrder(fd->playerFrameDatas[i].sysPad.newPressedButtons);
        }
    }
    

    // TODO: fix pause by making sure that the sys data thingy is also checking for one of the other button bits

    BrawlbackPad GamePadToBrawlbackPad(const gfPadStatus& pad) {
        BrawlbackPad ret;
        ret._buttons = pad._buttons.bits;
        ret.buttons = pad.buttons.bits;
        // *(ret.newPressedButtons-0x2) = (int)*(pad+0x14);
        ret.holdButtons = pad.holdButtons.bits;
        ret.rapidFireButtons = pad.rapidFireButtons.bits;
        ret.releasedButtons = pad.releasedButtons.bits;
        ret.newPressedButtons = pad.newPressedButtons.bits;
        ret.LAnalogue = pad.LAnalogue;
        ret.RAnalogue = pad.RAnalogue;
        ret.cStickX = pad.cStickX;
        ret.cStickY = pad.cStickY;
        ret.stickX = pad.stickX;
        ret.stickY = pad.stickY;
        ret.LTrigger = pad.LTrigger;
        ret.RTrigger = pad.RTrigger;


        // OSReport("BUTTONS======\n");
        // OSReport("Buttons: 0x%x\n", pad._buttons.bits);
        // OSReport("Buttons: 0x%x\n", pad.buttons.bits);
        // OSReport("Buttons holdButtons: 0x%x\n", pad.holdButtons.bits);
        // OSReport("Buttons rapidFireButtons: 0x%x\n", pad.rapidFireButtons.bits);
        // OSReport("Buttons releasedButtons: 0x%x\n", pad.releasedButtons.bits);
        // OSReport("Buttons newPressedButtons: 0x%x\n", pad.newPressedButtons.bits);

        return ret;
    }

    void PopulatePlayerFrameData(PlayerFrameData& pfd, u8 port, u8 pIdx) {
        if(gameHasStarted())
        {
            ftManager* fighterManager = g_ftManager;
            Fighter* fighter = fighterManager->getFighter(fighterManager->getEntryIdFromIndex(pIdx));
            ftOwner* ftowner = fighterManager->getOwner(fighterManager->getEntryIdFromIndex(pIdx));
            
            pfd.syncData.facingDir = fighter->m_moduleAccesser->getPostureModule()->getLr() < 0.0 ? -1 : 1;
            pfd.syncData.locX = fighter->m_moduleAccesser->getPostureModule()->getPos().m_x;
            pfd.syncData.locY = fighter->m_moduleAccesser->getPostureModule()->getPos().m_y;
            pfd.syncData.anim = fighter->m_moduleAccesser->getStatusModule()->getStatusKind();
            
            pfd.syncData.percent = (float)ftowner->getDamage();
            pfd.syncData.stocks = (u8)ftowner->getStockCount();
        }
        
        pfd.pad = Util::GamePadToBrawlbackPad(g_PadSystem->gcPads[port]);
        OSReport("STICK X: %d\n", pfd.pad.stickX);
        pfd.sysPad = Util::GamePadToBrawlbackPad(g_PadSystem->gcSysPads[port]);
    }
    void InjectBrawlbackPadToPadStatus(gfPadStatus& gamePad, const BrawlbackPad& pad, int port) {
        
        // TODO: do this once on match start or whatever, so we don't need to access this so often and lose cpu cycles
        //bool isNotConnected = Netplay::getGameSettings().playerSettings[port].playerType == PlayerType::PLAYERTYPE_NONE;
        // get current char selection and if none, the set as not connected
        u8 charId = *(((u8*)g_gmGlobalModeMelee)+P1_CHAR_ID_IDX + (port*92));
        // u8 charId = GM_GLOBAL_MODE_MELEE->playerData[port].charId;
        bool isNotConnected = charId == -1;
        // GM_GLOBAL_MODE_MELEE->playerData[port].playerType = isNotConnected ? 03 : 0 ; // Set to Human
        

        gamePad.isNotConnected = isNotConnected;
        gamePad._buttons.bits = pad._buttons;
        gamePad.buttons.bits = pad.buttons;
        // int* addr  = (int*) &gamePad;
        // *(addr+0x14+0x2) = pad.buttons;
        gamePad.holdButtons.bits = pad.holdButtons;
        gamePad.rapidFireButtons.bits = pad.rapidFireButtons;
        gamePad.releasedButtons.bits = pad.releasedButtons;
        gamePad.newPressedButtons.bits = pad.newPressedButtons;
        gamePad.LAnalogue = pad.LAnalogue;
        gamePad.RAnalogue = pad.RAnalogue;
        gamePad.cStickX = pad.cStickX;
        gamePad.cStickY = pad.cStickY;
        gamePad.stickX = pad.stickX;
        gamePad.stickY = pad.stickY;
        gamePad.LTrigger = pad.LTrigger;
        gamePad.RTrigger = pad.RTrigger;
        // OSReport("Port 0x%x Inputs\n", port);
        // OSReport("Buttons: 0x%x\n", pad.buttons);
        // OSReport("Buttons: 0x%x\n", pad.newPressedButtons);

    }

    void SaveState(u32 currentFrame) {
        OSReport("SENDING CAPTURE SAVESTATE!\n");
        EXIPacket::CreateAndSend(CMD_CAPTURE_SAVESTATE, &currentFrame, sizeof(currentFrame));
    }
}

namespace Match {
    const char* relevantHeaps = "System WiiPad IteamResource InfoResource CommonResource CopyFB Physics ItemInstance Fighter1Resoruce Fighter2Resoruce Fighter1Resoruce2 Fighter2Resoruce2 Fighter1Instance Fighter2Instance FighterTechqniq InfoInstance InfoExtraResource GameGlobal FighterKirbyResource1 GlobalMode OverlayCommon Tmp OverlayStage ItemExtraResource FighterKirbyResource2 FighterKirbyResource3";

    void PopulateGameReport(GameReport& report)
    {
        ftManager* fighterManager = g_ftManager;

        for (int i = 0; i < Netplay::getGameSettings().numPlayers; i++) {
            Fighter* const fighter = fighterManager->getFighter(fighterManager->getEntryIdFromIndex(i));
            s32 stocks = fighter->getOwner()->getStockCount();
            OSReport("Stock count player idx %i = %i\n", i, stocks);
            report.stocks[i] = stocks;
            f64 damage = fighter->getOwner()->getDamage();
            OSReport("Damage for player idx %i = %f\n", i, damage);
            report.damage[i] = damage;
        }
        report.frame_duration = getCurrentFrame();
    }
    void SendGameReport(GameReport& report)
    {
        OSReport("Sending end match report to emu. Num players = %u\n", (u32)Netplay::getGameSettings().numPlayers);
        EXIPacket::CreateAndSend(CMD_MATCH_END, &report, sizeof(report));
    }
    void StopGameScMeleeHook()
    {
        utils::SaveRegs();
        OSReport("Game report in stopGameScMeleeBeginningHook hook\n");
        if (Netplay::getGameSettings().numPlayers > 1) {
            #if 0  // toggle for sending end match game stats
            GameReport report;
            PopulateGameReport(report);
            SendGameReport(report);
            #endif
        }
        utils::RestoreRegs();
        asm(
            "mr r17, r15\n\r"
        );
    }
    void StartSceneMelee()
    {
        utils::SaveRegs();
        OSDisableInterrupts();
        OSReport("  ~~~~~~~~~~~~~~~~  Start Scene Melee  ~~~~~~~~~~~~~~~~  \n");
        #ifdef NETPLAY_IMPL
        //Netplay::StartMatching(); // now moved to GmGlobalModeMelee.cpp
        Netplay::SetIsInMatch(true);
        #else
        // 'debug' values
        Netplay::getGameSettings().localPlayerIdx = 0;
        Netplay::localPlayerIdx = 0;
        Netplay::getGameSettings().numPlayers = 2;
        #endif
        OSEnableInterrupts();
        utils::RestoreRegs();
        asm(
            "addi sp, sp, 112\n\r"
        );
    }
    void ExitSceneMelee()
    {
        utils::SaveRegs();
        OSDisableInterrupts();
        OSReport("  ~~~~~~~~~~~~~~~~  Exit Scene Melee  ~~~~~~~~~~~~~~~~  \n");
        frameCounter = 0;
        #ifdef NETPLAY_IMPL
        Netplay::EndMatch();
        Netplay::SetIsInMatch(false);
        #endif
        OSEnableInterrupts();
        utils::RestoreRegs();
        asm(
            "li r4, 0x0\n\r"
        );
    }
    void dumpAll_gfMemoryPool_hook()
    {
        register void* heap;
        asm (
            "mr heap, r29\n\t"
        );
        utils::SaveRegs();
        if(!DumpAllGfMemoryPoolHook(heap))
        {
            utils::RestoreRegs();
            asm(
                "lis r12, 0x8002\n\t"
                "ori r12, r12, 0x4ab0\n\t"
                "mtctr r12\n\t"
                "bctr\n\t"
            );
        }
        else {
            utils::RestoreRegs(); 
            asm(
                "lis r12, 0x8002\n\t"
                "ori r12, r12, 0x4ab4\n\t"
                "mtctr r12\n\t"
                "bctr\n\t"
            );
        }
    }
    bool DumpAllGfMemoryPoolHook(void* heap)
    {
        return strstr(relevantHeaps, *(char**)(heap)) != NULL;
    }
    void dump_gfMemoryPool_hook()
    {
        utils::SaveRegs();
        register char** r30_reg_val;
        register u32 addr_start;
        register u32 addr_end;
        register u32 mem_size;
        register u8 id;
        asm(
            "mr r30_reg_val, r30\n\t"
            "mr addr_start, r4\n\t"
            "mr addr_end, r5\n\t"
            "mr mem_size, r6\n\t"
            "mr id, r7\n\t"
        );
        DumpGfMemoryPoolHook(r30_reg_val, addr_start, addr_end, mem_size, id);
        utils::RestoreRegs();
    }
    void DumpGfMemoryPoolHook(char** r30_reg_val, u32 addr_start, u32 addr_end, u32 mem_size, u8 id)
    {
        char* heap_name = *r30_reg_val;
        SavestateMemRegionInfo memRegion;
        memRegion.address = addr_start; // might be bad cast... 64 bit ptr to 32 bit int
        memRegion.size = mem_size;
        memCpy(memRegion.nameBuffer, heap_name, strlen(heap_name));
        memRegion.nameBuffer[strlen(heap_name)] = '\0';
        memRegion.nameSize = strlen(heap_name);
        OSReport("SENDING DUMPALL!\n");
        EXIPacket::CreateAndSend(CMD_SEND_DUMPALL, &memRegion, sizeof(SavestateMemRegionInfo));
    }
    void ProcessGameAllocation(u8* allocated_addr, u32 size, char* heap_name)
    {
        if (shouldTrackAllocs && strstr(relevantHeaps, heap_name) != NULL && !isRollback) {
            //OSReport("ALLOC: size = 0x%08x  allocated addr = 0x%08x\n", size, allocated_addr);
            SavestateMemRegionInfo memRegion;
            memRegion.address = reinterpret_cast<u32>(allocated_addr); // might be bad cast... 64 bit ptr to 32 bit int
            memRegion.size = size;
            memCpy(memRegion.nameBuffer, heap_name, strlen(heap_name));
            memRegion.nameBuffer[strlen(heap_name)] = '\0';
            memRegion.nameSize = strlen(heap_name);
            OSReport("SENDING ALLOCS!\n");
            EXIPacket::CreateAndSend(CMD_SEND_ALLOCS, &memRegion, sizeof(memRegion));
        }
    }
    void ProcessGameFree(u8* address, char* heap_name)
    {
        if (shouldTrackAllocs && strstr(relevantHeaps, heap_name) != NULL && !isRollback) {
            //OSReport("FREE: addr = 0x%08x\n", address);
            SavestateMemRegionInfo memRegion;
            memCpy(memRegion.nameBuffer, heap_name, strlen(heap_name));
            memRegion.nameBuffer[strlen(heap_name)] = '\0';
            memRegion.nameSize = strlen(heap_name);
            memRegion.address = reinterpret_cast<u32>(address);
            OSReport("SENDING DEALLOCS!\n");
            EXIPacket::CreateAndSend(CMD_SEND_DEALLOCS, &memRegion, sizeof(memRegion));
        }
    }
    u32 allocSizeTracker = 0;
    char allocHeapName[20];
    void alloc_gfMemoryPool_hook()
    {
        utils::SaveRegs();
        register char** internal_heap_data;
        register u32 size;
        register u32 alignment;
        asm(
            "mr internal_heap_data, r3\n\t"
            "mr size, r4\n\t"
            "mr alignment, r5\n\t"
        );
        AllocGfMemoryPoolBeginHook(internal_heap_data, size, alignment);
        utils::RestoreRegs();
        asm(
            "lbz r7, 0x0024 (r3)\n\r"
        );
    }
    void AllocGfMemoryPoolBeginHook(char** internal_heap_data, u32 size, u32 alignment)
    {
        char* heap_name = *internal_heap_data;
        size_t heapNameSize = strlen(heap_name);
        memCpy(allocHeapName, heap_name, heapNameSize);
        allocHeapName[heapNameSize] = '\0';
        allocSizeTracker = size;
    }
    void allocGfMemoryPoolEndHook()
    {
        utils::SaveRegs();
        register u8* alloc_addr;
        asm(
            "mr alloc_addr, r30\n\r"
        );
        ProcessGameAllocation(alloc_addr, allocSizeTracker, allocHeapName);
        utils::RestoreRegs();
    }
    void free_gfMemoryPool_hook()
    {
        utils::SaveRegs();
        register char** internal_heap_data;
        register u8* address;
        asm(
            "mr internal_heap_data, r3\n\r"
            "mr address, r4 \n\r"
        );
        FreeGfMemoryPoolHook(internal_heap_data, address);
        utils::RestoreRegs();
        asm(
            "addi r31, r3, 40\n\r"
        );
    }
    void FreeGfMemoryPoolHook(char** internal_heap_data, u8* address)
    {
        char* heap_name = *internal_heap_data;
        ProcessGameFree(address, heap_name);
    }
}

namespace FrameAdvance {
    u32 framesToAdvance = 1;
    FrameData currentFrameData;
    u32 getFramesToAdvance() 
    { 
        return framesToAdvance; 
    }

    void TriggerFastForwardState(u8 numFramesToFF) 
    {
        if (framesToAdvance == 1 && numFramesToFF > 0) {
            framesToAdvance = numFramesToFF;
        }
    }
    void StallOneFrame() 
    { 
        if (framesToAdvance == 1) {
            framesToAdvance = 0; 
        }
    }

    void ResetFrameAdvance() 
    { 
        //OSReport("Resetting frameadvance to normal\n");
        framesToAdvance = 1;
        isRollback = false;
    }


    // requests FrameData for specified frame from emulator and assigns it to inputs
    void GetInputsForFrame(u32 frame, FrameData* inputs) 
    {
        OSReport("SENDING FRAMEDATA!\n");
        EXIPacket::CreateAndSend(CMD_FRAMEDATA, &frame, sizeof(frame));
        EXIHooks::readEXI(inputs, sizeof(FrameData), EXI_CHAN_1, 0, EXI_FREQ_32HZ);
        Util::FixFrameDataEndianness(inputs);
    }

    // should be called on every simulation frame
    void ProcessGameSimulationFrame(FrameData* inputs) 
    {
        OSDisableInterrupts();
        u32 gameLogicFrame = getCurrentFrame();
        //OSReport("ProcessGameSimulationFrame %u \n", gameLogicFrame);

        // save state on each simulated frame (this includes resim frames)
        Util::SaveState(gameLogicFrame);

        GetInputsForFrame(gameLogicFrame, inputs);
        gameLogicFrame = getCurrentFrame();

        //OSReport("Using inputs %u %u  game frame: %u\n", inputs->playerFrameDatas[0].frame, inputs->playerFrameDatas[1].frame, gameLogicFrame);
        
        //Util::printFrameData(*inputs);

        OSEnableInterrupts();
    }

    void updateIpSwitchPreProcess() 
    {
        utils::SaveRegs();
        OSDisableInterrupts();
        if (Netplay::IsInMatch()) {
            ProcessGameSimulationFrame(&currentFrameData);
        }
        OSEnableInterrupts();
        utils::RestoreRegs();
        asm("addi r29, r31, 0x8\r\n");
    }

    void updateLowHook() 
    {
        utils::SaveRegs();
        register gfPadSystem* padSystem; 
        register u32 padStatus;
        asm(
            "mr padSystem, r25\r\n"
            "mr padStatus, r26\r\n"
        );
        getGamePadStatusInjection(padSystem, padStatus);
        utils::RestoreRegs();
        asm(
            "lwz r4, -0x4390 (r13)\r\n"
        );
    }
    void getGamePadStatusInjection(gfPadSystem* padSystem, u32 padStatus) 
    {
        OSDisableInterrupts();
        // OSReport("PAD %i 0x%x\n", 0, &PAD_SYSTEM->sysPads[0]);
        // OSReport("PAD %i 0x%x\n", 1, &PAD_SYSTEM->sysPads[1]);
        // OSReport("PAD %i 0x%x\n", 2, &PAD_SYSTEM->sysPads[2]);
        // OSReport("PAD %i 0x%x\n", 3, &PAD_SYSTEM->sysPads[3]);
        //Util::printGameInputs(PAD_SYSTEM->sysPads[0]);
        //  if((ddst->releasedButtons.bits + ddst->newPressedButtons.bits) != 0){
        //         OSReport("LOCAL BUTTONS(P%i)===GP(%i)===\n", port, isGamePad);
        //         OSReport("releasedButtons 0x%x\n", ddst->releasedButtons.bits);
        //         OSReport("newPressedButtons 0x%x\n", ddst->newPressedButtons.bits);
        // }

        // if((ddst->buttons.bits) != 0){
        //     // OSReport("LOCAL BUTTONS(P%i)===GP(%i)===\n", port, isGamePad);
        //     // OSReport("buttons 0x%x\n", ddst->buttons.bits);
        //     OSReport("Melee Info=====\n");
        //     OSReport("p1 charId=0x%x ptype=0x%x unk1=0x%x unk2=0x%x\n", GM_GLOBAL_MODE_MELEE->playerData[0].charId, GM_GLOBAL_MODE_MELEE->playerData[0].playerType, GM_GLOBAL_MODE_MELEE->playerData[0].unk1, GM_GLOBAL_MODE_MELEE->playerData[0].unk2);
        //     OSReport("p2 charId=0x%x ptype=0x%x unk1=0x%x unk2=0x%x\n", GM_GLOBAL_MODE_MELEE->playerData[1].charId, GM_GLOBAL_MODE_MELEE->playerData[1].playerType, GM_GLOBAL_MODE_MELEE->playerData[1].unk1, GM_GLOBAL_MODE_MELEE->playerData[1].unk2);
        //     OSReport("p3 charId=0x%x ptype=0x%x unk1=0x%x unk2=0x%x\n", GM_GLOBAL_MODE_MELEE->playerData[2].charId, GM_GLOBAL_MODE_MELEE->playerData[2].playerType, GM_GLOBAL_MODE_MELEE->playerData[2].unk1, GM_GLOBAL_MODE_MELEE->playerData[2].unk2);
        //     OSReport("p4 charId=0x%x ptype=0x%x unk1=0x%x unk2=0x%x\n", GM_GLOBAL_MODE_MELEE->playerData[3].charId, GM_GLOBAL_MODE_MELEE->playerData[3].playerType, GM_GLOBAL_MODE_MELEE->playerData[3].unk1, GM_GLOBAL_MODE_MELEE->playerData[3].unk2);

        // }
        if (Netplay::IsInMatch()) {
            //OSReport("Injecting pad for- frame %u port %i\n", currentFrameData.playerFrameDatas[port].frame, port);
            
            FrameLogic::FrameDataLogic(getCurrentFrame());
            for(int i  = 0; i < MAX_NUM_PLAYERS; i++)
            {
                gfPadStatus* gamePad = reinterpret_cast<gfPadStatus*>(padStatus + 0x40 * i);
                PlayerFrameData frameData = currentFrameData.playerFrameDatas[i];
                BrawlbackPad pad = frameData.pad;
                Util::InjectBrawlbackPadToPadStatus(*gamePad, pad, i);
                // if(ddst->newPressedButtons.bits == 0x1000){
                //     bp();
                // }

                // TODO: make whole game struct be filled in from dolphin based off a known good match between two characters on a stage like battlefield
                if((gamePad->releasedButtons.bits + gamePad->newPressedButtons.bits) != 0){
                    OSReport("BUTTONS(P%i)===GP(%i)===\n", i, true);
                    OSReport("releasedButtons 0x%x\n", gamePad->releasedButtons.bits);
                    OSReport("newPressedButtons 0x%x\n", gamePad->newPressedButtons.bits);
                }
            }

            // if((ddst->buttons.bits) != 0){
                // OSReport("BUTTONS(P%i)===GP(%i)===\n", port, isGamePad);
                // OSReport("buttons 0x%x\n", ddst->buttons.bits);
            // }

        }
        OSEnableInterrupts();
    }

    void handleFrameAdvanceHook() {
        utils::SaveRegs();
        setFrameAdvanceFromEmu();
        utils::RestoreRegs();
        asm("mr r24, %0"
            :
            : "r" (framesToAdvance)
        );
        asm(
            "cmplw r19, r24\n\t"
        );
    }
    void setFrameAdvanceFromEmu() {
        EXIPacket::CreateAndSend(CMD_FRAMEADVANCE);
        EXIHooks::readEXI(&framesToAdvance, sizeof(u32), EXI_CHAN_1, 0, EXI_FREQ_32HZ);
        utils::swapByteOrder(framesToAdvance);
    }
}

namespace FrameLogic {
    void WriteInputsForFrame(u32 currentFrame)
    {
        u8 localPlayerIdx = Netplay::localPlayerIdx;
        if (localPlayerIdx != Netplay::localPlayerIdxInvalid) {
            PlayerFrameData playerFrame;
            playerFrame.frame = currentFrame;
            playerFrame.playerIdx = localPlayerIdx;
            Util::PopulatePlayerFrameData(playerFrame, Netplay::getGameSettings().localPlayerPort, localPlayerIdx);
            // sending inputs + current game frame
            OSReport("SENDING ONLINE INPUTS!\n");
            EXIPacket::CreateAndSend(CMD_ONLINE_INPUTS, &playerFrame, sizeof(PlayerFrameData));
        }
        else {
            OSReport("Invalid player index! Can't send inputs to emulator!\n");
        }
    }
    void FrameDataLogic(u32 currentFrame)
    {
        WriteInputsForFrame(currentFrame);
    }
    void SendFrameCounterPointerLoc()
    {
        u32 frameCounterLocation = reinterpret_cast<u32>(&frameCounter);
        OSReport("SENDING FRAME COUNTER LOCATION!\n");
        EXIPacket::CreateAndSend(CMD_SEND_FRAMECOUNTERLOC, &frameCounterLocation, sizeof(u32));
    }
    const char* nonResimTasks = "ecMgr EffectManager";
    bool ShouldSkipGfTaskProcess(u32* gfTask, u32 task_type)
    {
        if (FrameAdvance::getFramesToAdvance() > 1) { // if we're resimulating, disable certain tasks that don't need to run on resim frames.
            char* taskName = (char*)(*gfTask); // 0x0 offset of gfTask* is the task name
            //OSReport("Processing task %s\n", taskName);
            return strstr(nonResimTasks, taskName) != NULL;
        }
        return false;
    }
    void initFrameCounter()
    {
        utils::SaveRegs();
        frameCounter = 0;
        utils::RestoreRegs();
        asm (
            "li r0, 0\n\t"
        );
    }
    void updateFrameCounter()
    {
        utils::SaveRegs();
        frameCounter++;
        utils::RestoreRegs();
        asm (
            "lbz r0, 0x00ED (r30)\n\t"
        );
    }
    void beginningOfMainGameLoop()
    {
        utils::SaveRegs();
        if (Netplay::IsInMatch()) {
            OSReport("SENDING CMD_TIMER_START!\n");
            EXIPacket::CreateAndSend(CMD_TIMER_START);
        }
        utils::RestoreRegs();
        
        asm (
            "li r25, 1\n\t"
        );
    }
    void beginFrame()
    {
        utils::SaveRegs();
        u32 currentFrame = getCurrentFrame();

        //Util::printGameInputs(PAD_SYSTEM->pads[0]);

        //Util::printGameInputs(PAD_SYSTEM->sysPads[0]);

        // this is the start of all our logic for each frame. Because EXI writes/reads are synchronous,
        // you can think of the control flow going like this
        // this function -> write data to emulator through exi -> emulator processes data and possibly queues up data
        // to send back to the game -> send data to the game if there is any -> game processes that data -> repeat

        if (Netplay::IsInMatch()) {
            OSDisableInterrupts();
            // reset flag to be used later
            // just resimulated/stalled/skipped/whatever, reset to normal
            FrameAdvance::ResetFrameAdvance();
            OSReport("------ Frame %u ------\n", frameCounter);
            OSReport("------ Frame Location 0x%x ------\n", &frameCounter);
            // lol
            g_mtRandDefault->seed = 0x496ffd00;


            #ifdef NETPLAY_IMPL
                if(!shouldTrackAllocs)
                {
                    SendFrameCounterPointerLoc();
                    gfHeapManager::dumpAll();
                    shouldTrackAllocs = true;
                }
            #endif
            OSEnableInterrupts();
        }
        utils::RestoreRegs();
        asm (
            "li r0, 0x1\n\t"
        );
    }
    void endFrame()
    {
        utils::SaveRegs();
        if (Netplay::IsInMatch()) {
            OSReport("SENDING CMD_TIMER_END!\n");
            EXIPacket::CreateAndSend(CMD_TIMER_END);
        }
        utils::RestoreRegs();
        asm (
            "li r0, 0x0\n\t"
        );
    }
    void endMainLoop()
    {
        utils::SaveRegs();
        utils::RestoreRegs();
        asm (
            "lwz r3, 0x100(r23)\n\t"
        );
    }
    #if 1
    void gfTaskProcessHook()
    {
        utils::SaveRegs();
        register u32* gfTask; 
        register u32 task_type;
        asm (
            "mr gfTask, r3\n\t"
            "mr task_type, r4\n\t"
        );
        if(!ShouldSkipGfTaskProcess(gfTask, task_type))
        {
            utils::RestoreRegs();
            asm (
                "cmpwi r4, 0x8\n\t"
            );
        }
        else 
        {
            utils::RestoreRegs();
            asm (
                "lis r12, 0x8002\n\t"
                "ori r12, r12, 0xdd28\n\t"
                "mtctr r12\n\t"
                "bctr\n\t"
            );
        }
    }
    #endif
}

namespace GMMelee {
    bool isMatchChoicesPopulated = false;
    int charChoices[MAX_NUM_PLAYERS] = {-1, -1, -1, -1};
    int stageChoice = -1;
    void PopulateMatchSettings(int chars[MAX_NUM_PLAYERS], int stageID) 
    {
        for (int i = 0; i < MAX_NUM_PLAYERS; i++) {
            charChoices[i] = chars[i];
        }
        stageChoice = stageID;
        isMatchChoicesPopulated = true;
    }
    // called on match end
    void ResetMatchChoicesPopulated() 
    { 
        isMatchChoicesPopulated = false; 
    }

    void postSetupMelee() {
        
        asm (
            "addi sp, sp, 48\n\t"
        );
        utils::SaveRegs();
        OSDisableInterrupts();
        OSReport("postSetupMelee\n");

        #ifdef NETPLAY_IMPL
        Netplay::StartMatching(); // start netplay logic
        #endif

        // falco, wolf, battlefield
        //PopulateMatchSettings( {0x15, 0x29, -1, -1}, 0x1 );

        if (isMatchChoicesPopulated) {
            OSReport("postSetupMelee stage: 0x%x p1: 0x%x p2: 0x%x\n", stageChoice, charChoices[0], charChoices[1]);

            memCpy(g_gmGlobalModeMelee, (u8*)defaultGmGlobalModeMelee, 0x320);
            u8* melee = (u8*)g_gmGlobalModeMelee;

            melee[P1_CHAR_ID_IDX] = charChoices[0];
            melee[P2_CHAR_ID_IDX] = charChoices[1];
            g_gmGlobalModeMelee->m_playersInitData[0].m_slotID = charChoices[0];
            g_gmGlobalModeMelee->m_playersInitData[1].m_slotID = charChoices[1];
            
            g_gmGlobalModeMelee->m_playersInitData[0].m_initState = 0;
            g_gmGlobalModeMelee->m_playersInitData[1].m_initState = 0;

            g_gmGlobalModeMelee->m_playersInitData[0].unk1 = 0x80;
            g_gmGlobalModeMelee->m_playersInitData[1].unk1 = 0x80;

            g_gmGlobalModeMelee->m_playersInitData[0].unk2 = 0x0;
            g_gmGlobalModeMelee->m_playersInitData[1].unk2 = 0x1;

            // melee[P1_CHAR_ID_IDX+1] = 0; // Set player type to human
            // melee[P2_CHAR_ID_IDX+1] = 0;
            // melee[STAGE_ID_IDX] = stageChoice;
            melee[STAGE_ID_IDX] = 0x01; // TODO uncomment and use above line, just testing with battlefield
        }
        
        OSEnableInterrupts();
        utils::RestoreRegs();
    }
}

namespace Netplay {
    GameSettings gameSettings;
    const u8 localPlayerIdxInvalid = 200;
    u8 localPlayerIdx = localPlayerIdxInvalid;
    bool isInMatch = false;
    bool IsInMatch() 
    { 
        return isInMatch; 
    }
    void SetIsInMatch(bool isMatch) 
    {
        isInMatch = isMatch; 
    }

    GameSettings& getGameSettings() 
    { 
        return gameSettings;
    }

    void FixGameSettingsEndianness(GameSettings& settings) 
    {
        utils::swapByteOrder(settings.stageID);
    }

    void StartMatching() 
    {
        OSReport("Filling in game settings from game\n");
        // populate game settings
        fillOutGameSettings(gameSettings);

        OSReport("Starting match gameside\n");
        // send our populated game settings to the emu
        EXIPacket startMatchPckt = EXIPacket(CMD_START_MATCH, &gameSettings, sizeof(GameSettings));
        startMatchPckt.Send();

        // start emu netplay thread so it can start trying to find an opponent
        EXIPacket findOpponentPckt = EXIPacket(CMD_FIND_OPPONENT);
        findOpponentPckt.Send();

        // Temporary. Atm, this just stalls main thread while we do our mm/connecting
        // in the future, when netmenu stuff is implemented, the organization of StartMatching and CheckIsMatched
        // will make more sense
        while (!CheckIsMatched()) {}

    }

    
    bool CheckIsMatched() {
        bool matched = false;
        u8 cmd_byte = CMD_UNKNOWN;

        // cmd byte + game settings
        size_t read_size = sizeof(GameSettings) + 1;
        u8* read_data = new u8[read_size];

        // stall until we get game settings from opponent, then load those in and continue to boot up the match
        //while (cmd_byte != EXICommand::CMD_SETUP_PLAYERS) {
            EXIHooks::readEXI(read_data, read_size, EXI_CHAN_1, 0, EXI_FREQ_32HZ);
            cmd_byte = read_data[0];

            if (cmd_byte == CMD_SETUP_PLAYERS) {
                OSReport("SETUP PLAYERS GAMESIDE\n");
                GameSettings gameSettingsFromOpponent = bufferToObject<GameSettings>(&read_data[1]);
                OSReport("--SETUP CHARS--\nP1 char: %u  P2 char: %u\n--END SETUP CHARS--\n", (unsigned int)gameSettingsFromOpponent.playerSettings[0].charID, (unsigned int)gameSettingsFromOpponent.playerSettings[1].charID);
                FixGameSettingsEndianness(gameSettingsFromOpponent);
                MergeGameSettingsIntoGame(gameSettingsFromOpponent);
                // TODO: we shoud assign the gameSettings var to the gameSettings from opponent since its merged with ours now.
                gameSettings.localPlayerPort = gameSettingsFromOpponent.localPlayerPort;
                matched = true;
            }
            else {
                //OSReport("Reading for setupplayers, didn't get it...\n");
            }
        //}
        return matched;
    }

    void EndMatch() {
        localPlayerIdx = localPlayerIdxInvalid;
        gameSettings = GameSettings();
        GMMelee::ResetMatchChoicesPopulated();
    }

}

namespace MemoryHooks {
    void fakeGFPoolAlloc1()
    {
        utils::SaveRegs();
        u8 r3Value;
        asm (
            "mr %0, r3\n\t" 
            : "=r" (r3Value)
        );
        if(r3Value != 0xFF)
        {
            utils::RestoreRegs();
            asm (
                "lis r6, 0x8049\n\t"
            );
        }
        else {
            utils::RestoreRegs();
            register size_t r4Value;
            asm (
                "mr %0, r4\n\t"
                "li r4, 32\n\t"
                : "=r"(r4Value)
            );
            MemExpHooks::mallocExp(r4Value);
        }
    }
    void fakeGFPoolAlloc2()
    {
        utils::SaveRegs();
        u8 r3Value;
        asm (
            "mr %0, r3\n\t"
            : "=r"(r3Value)
        );
        if(r3Value != 0xFF)
        {
            utils::RestoreRegs();
            asm (
                "lis r6, 0x8049\n\t"
            );
        }
        else {
            size_t r4Value;
            utils::RestoreRegs();
            asm (
                "mr %0, r4\n\t"
                "mr r4, r5\n\t"
                : "=r" (r4Value)
            );
            MemExpHooks::mallocExp(r4Value);
        }
    }
    void fakeGFPoolFree()
    {
        u32 r3PointerVal;
        utils::SaveRegs();
        asm (
            "mr %0, r3\n\t"
            : "=r" (r3PointerVal)
        );
        if(r3PointerVal <= reinterpret_cast<u32>(MemExpHooks::mainHeap))
        {
            utils::RestoreRegs();
            MemExpHooks::freeExp(MemExpHooks::mainHeap);
        }
        else {
            utils::RestoreRegs();
            asm (
                "stwu sp, -0x20(sp)\n\t"
            );
        }
    }
}

namespace RollbackHooks {
    void InstallHooks() 
    {
        MemExpHooks::initializeMemory((void*) 0x93604000, 0x4000);
        // Match Namespace
        SyringeCore::syHookFunction(0x806d4c10, reinterpret_cast<void*>(Match::StopGameScMeleeHook));
        SyringeCore::syHookFunction(0x806d176c, reinterpret_cast<void*>(Match::StartSceneMelee));
        SyringeCore::syHookFunction(0x806d4844, reinterpret_cast<void*>(Match::ExitSceneMelee));
        SyringeCore::syHookFunction(0x80024aac, reinterpret_cast<void*>(Match::dumpAll_gfMemoryPool_hook));
        SyringeCore::syHookFunction(0x8002625c, reinterpret_cast<void*>(Match::dump_gfMemoryPool_hook));
        SyringeCore::syHookFunction(0x80025c6c, reinterpret_cast<void*>(Match::alloc_gfMemoryPool_hook));
        SyringeCore::syHookFunction(0x80025ec4, reinterpret_cast<void*>(Match::allocGfMemoryPoolEndHook));
        SyringeCore::syHookFunction(0x80025f40, reinterpret_cast<void*>(Match::free_gfMemoryPool_hook));
        SyringeCore::syHookFunction(0x80024a78, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x80024a84, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x80024a90, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x80024adc, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x80024aec, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x80024ae0, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x80026288, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x8002619c, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x800261b4, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x800262e0, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x800260fc, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x80026114, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x800260f0, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x80026278, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x800262ac, reinterpret_cast<void*>(utils::nop));
        SyringeCore::syHookFunction(0x800260e0, reinterpret_cast<void*>(utils::nop));

        // FrameAdvance Namespace
        SyringeCore::syHookFunction(0x8004aa2c, reinterpret_cast<void*>(FrameAdvance::updateIpSwitchPreProcess));
        SyringeCore::syHookFunction(0x80029468, reinterpret_cast<void*>(FrameAdvance::updateLowHook));
        SyringeCore::syHookFunction(0x800173a4, reinterpret_cast<void*>(FrameAdvance::handleFrameAdvanceHook));

        // FrameLogic Namespace
        SyringeCore::syHookFunction(0x8002dc74, reinterpret_cast<void*>(FrameLogic::gfTaskProcessHook));
        SyringeCore::syHookFunction(0x800174fc, reinterpret_cast<void*>(FrameLogic::endMainLoop));
        SyringeCore::syHookFunction(0x801473a0, reinterpret_cast<void*>(FrameLogic::endFrame));
        SyringeCore::syHookFunction(0x800171b4, reinterpret_cast<void*>(FrameLogic::beginningOfMainGameLoop));
        SyringeCore::syHookFunction(0x80017760, reinterpret_cast<void*>(FrameLogic::updateFrameCounter));
        SyringeCore::syHookFunction(0x8004e884, reinterpret_cast<void*>(FrameLogic::initFrameCounter));
        SyringeCore::syHookFunction(0x80147394, reinterpret_cast<void*>(FrameLogic::beginFrame));

        // GMMelee Namespace
        SyringeCore::syHookFunction(0x806dd03c, reinterpret_cast<void*>(GMMelee::postSetupMelee));
    }
}