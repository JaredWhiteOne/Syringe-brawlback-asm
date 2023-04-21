#include "EXI.h"
//can make these default to the standard chanels and stuff
void writeEXI(void* source, u32 size, EXIChannel channel, u32 device, EXIFreq frequency);
void readEXI(void* destination, u32 size, EXIChannel channel, u32 device, EXIFreq frequency);

void setupEXIDevice(EXIChannel channel, u32 device, EXIFreq frequency);
void removeEXIDevice(EXIChannel channel);

//These all return false if they fail, true if the succeed

bool attachEXIDevice(EXIChannel channel, EXICallback callback = 0);
bool detachEXIDevice(EXIChannel channel);
bool lockEXIDevice(EXIChannel channel, u32 device, EXICallback callback = 0);
bool unlockEXIDevice(EXIChannel channel);
bool selectEXIDevice(EXIChannel channel, u32 device, EXIFreq frequency);
bool deselectEXIDevice(EXIChannel channel);

bool transferDataEXI(EXIChannel channel, void* data, u32 size, EXIFreq transferType, EXICallback callback = 0);

bool syncEXITransfer(EXIChannel channel);