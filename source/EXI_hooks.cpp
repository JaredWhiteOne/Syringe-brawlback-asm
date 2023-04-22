#include "EXI_hooks.h"

MEMHeapHandle mainHeap;
namespace EXIHooks {
    void writeEXI(void* data, u32 size, EXIChannel channel, u32 device, EXIFreq frequency) {
        //need to make new buffer to ensure data is aligned to cache block
        void* alignedData = MEMAllocFromExpHeapEx(mainHeap, size, 32);
        memmove(alignedData, data, size);

        DCFlushRange(alignedData, size);
        setupEXIDevice(channel, device, frequency);
        EXIDma(channel, alignedData, size, 1, 0);
        syncEXITransfer(channel);
        removeEXIDevice(channel);
    }

    void readEXI(void* destination, u32 size, EXIChannel channel, u32 device, EXIFreq frequency) {
        void* alignedDestination = MEMAllocFromExpHeapEx(mainHeap, size, 32);

        setupEXIDevice(channel, device, frequency);
        EXIDma(channel, alignedDestination, size, 0, 0);
        syncEXITransfer(channel);
        removeEXIDevice(channel);
        DCFlushRange(alignedDestination, size);

        memmove(destination, alignedDestination, size);
    }
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

    bool syncEXITransfer(EXIChannel channel) {
        return EXISync(channel);
    }
}