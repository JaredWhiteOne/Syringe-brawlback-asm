#include "exi_packet.h"
#include <OS/OSInterrupt.h>
EXICommand EXIPacket::getCmd() { return this->cmd; }

EXIPacket::EXIPacket() { 
    EXIPacket(CMD_UNKNOWN, 0, 0);
}
EXIPacket::EXIPacket(u8 EXICmd) { 
    EXIPacket(EXICmd, 0, 0);
}

EXIPacket::EXIPacket(u8 EXICmd, void* source, u32 size) {
    if (!source) size = 0; if (size <= 0) source = 0; //sanity checks

    // enough for the EXICmd byte + size of the packet
    u32 new_size = sizeof(EXICmd) + size;

    u8* new_packet = (u8*)MEMAllocFromExpHeapEx(MemExpHooks::mainHeap, new_size, 32);
    if (!new_packet) {
        OSReport("Failed to alloc %u bytes! Heap space available: %u\n", size, MemExpHooks::getFreeSize(MemExpHooks::mainHeap, 4));
        return;
    }

    // copy EXICmd byte into packet
    memmove(new_packet, &EXICmd, sizeof(EXICmd));

    if (source) {
        // copy actual packet into our buffer
        memmove(new_packet + sizeof(EXICmd), source, size);
    }

    // set our size/src ptr so the Send() function knows how much/what to send
    this->size = new_size;
    this->source = new_packet;
    this->cmd = (EXICommand)EXICmd;
}

EXIPacket::~EXIPacket() {
    if (this->source) {
        MemExpHooks::freeExp(this->source);
    }
}

bool EXIPacket::Send() {
    OSDisableInterrupts();
    bool success = false;
    if (!this->source || this->size <= 0) {
        OSReport("Invalid EXI packet source or size! source: %x  size: %u\n", source, size);
    }
    else {
        EXIHooks::writeEXI(this->source, this->size, EXI_CHAN_1, 0, EXI_FREQ_32HZ);
        success = true;
    }

    OSEnableInterrupts();
    return success;
}

void EXIPacket::CreateAndSend(u8 EXICmd, const void* source, u32 size) {

    // enough for the EXICmd byte + size of the packet
    u32 new_size = sizeof(EXICmd) + size;

    u8* new_packet = new u8[new_size];

    // copy EXICmd byte into packet
    memmove(new_packet, &EXICmd, sizeof(EXICmd));

    if (source) {
        // copy actual packet into our buffer
        memmove(new_packet + sizeof(EXICmd), source, size);
    }

    EXIHooks::writeEXI(new_packet, new_size, EXI_CHAN_1, 0, EXI_FREQ_32HZ);
} 