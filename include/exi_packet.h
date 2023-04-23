#pragma once
#include "utils.h"
#include "EXI_hooks.h"
#include <types.h>
enum EXICommand
{
    CMD_UNKNOWN = 0,

    // Online

    CMD_ONLINE_INPUTS = 1U, // sending inputs from game to emulator
    CMD_CAPTURE_SAVESTATE = 2U,
    CMD_LOAD_SAVESTATE = 3U,

    CMD_FIND_OPPONENT = 5U,
    CMD_START_MATCH = 13U,
    CMD_SETUP_PLAYERS = 14U,
    CMD_FRAMEDATA = 15U, // game is requesting inputs for some frame
    CMD_TIMESYNC = 16U,
    CMD_ROLLBACK = 17U,
    CMD_FRAMEADVANCE = 18U,

    CMD_REPLAY_START_REPLAYS_STRUCT = 19U,
    CMD_REPLAY_REPLAYS_STRUCT = 20U,
    CMD_REPLAYS_REPLAYS_END = 21U,
    CMD_GET_NEXT_FRAME = 22U,
    CMD_BAD_INDEX = 23U,
    CMD_GET_NUM_REPLAYS = 24U,
    CMD_SET_CUR_INDEX = 25U,
    CMD_GET_START_REPLAY = 26U,
    CMD_SEND_ALLOCS = 30U,
    CMD_SEND_DEALLOCS = 31U,
    CMD_SEND_DUMPALL = 32U,
    CMD_SEND_FRAMECOUNTERLOC = 33U,
    
    CMD_MATCH_END = 4U,
    CMD_SET_MATCH_SELECTIONS = 6U,

    CMD_TIMER_START = 7U,
    CMD_TIMER_END = 8U,
    CMD_UPDATE = 9U,
    
    CMD_GET_ONLINE_STATUS = 10U,
    CMD_CLEANUP_CONNECTION = 11U,
    CMD_GET_NEW_SEED = 12U,
};


class EXIPacket {
public:
    EXIPacket(u8 EXICmd, void* source, u32 size);
    EXIPacket(u8 EXICmd);
    EXIPacket();
    ~EXIPacket();
    bool Send();
    static void CreateAndSend(u8 EXICmd, void* source = NULL, u32 size = 0);
    EXICommand getCmd();
private:
    u8* source;
    u32 size;
    EXICommand cmd;
};