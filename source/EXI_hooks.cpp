#include "EXI_hooks.h"
#include "mem.h"
#include "MEM/mem_expHeap.h"
/*
void writeEXI(void* data, u32 size, EXIChannel channel, u32 device, EXIFreq frequency) {
    //need to make new buffer to ensure data is aligned to cache block
    void* alignedData = MEMAllocFromExpHeapEx(mainHeap, size, 32);
    memmove(alignedData, data, size);

    flushDataCache(alignedData, size);
    setupEXIDevice(channel, device, frequency);
    transferDataEXI(channel, alignedData, size, EXIType::EXI_WRITE);
    syncEXITransfer(channel);
    removeEXIDevice(channel);

    free(alignedData);
}

void readEXI(void* destination, u32 size, EXIChannel channel, u32 device, EXIFreq frequency) {
    void* alignedDestination = allocFromExpHeap(mainHeap, size, 32);

    setupEXIDevice(channel, device, frequency);
    transferDataEXI(channel, alignedDestination, size, EXIType::EXI_READ);
    syncEXITransfer(channel);
    removeEXIDevice(channel);
    flushDataCache(alignedDestination, size);

    memcpy(destination, alignedDestination, size);
    free(alignedDestination);
}
*/
void setupEXIDevice(EXIChannel channel, u32 device, EXIFreq frequency) {
    attachEXIDevice(channel);
    lockEXIDevice(channel, device);
    selectEXIDevice(channel, device, frequency);
}

void removeEXIDevice(EXIChannel channel) {
    deselectEXIDevice(channel);
    unlockEXIDevice(channel);
    detachEXIDevice(channel);
}

bool attachEXIDevice(EXIChannel channel, EXICallback callback) {
    return EXIAttach(channel, callback);
}

bool detachEXIDevice(EXIChannel channel) {
    return EXIDetach(channel);
}

bool lockEXIDevice(EXIChannel channel, u32 device, EXICallback callback) {
    return EXILock(channel, device, callback);
}

bool unlockEXIDevice(EXIChannel channel) {
    return EXIUnlock(channel);
}

bool selectEXIDevice(EXIChannel channel, u32 device, EXIFreq frequency) {
    return EXISelect(channel, device, frequency);
}

bool deselectEXIDevice(EXIChannel channel) {
    return EXIDeselect(channel);
}

bool transferDataEXI(EXIChannel channel, void* data, u32 size, EXIType transferType, EXICallback callback) {
    return EXIDma(channel, data, size, transferType, callback);
}

bool syncEXITransfer(EXIChannel channel) {
    return EXISync(channel);
}