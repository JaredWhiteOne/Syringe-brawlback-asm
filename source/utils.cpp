#include <types.h>

namespace utils {
    u32 EncodeBranch(u32 start, u32 dest, bool linked)
    {
        u32 offset;
        if (start > dest)
        {
            offset = dest - start;
        }
        else
        {
            offset = -(start - dest);
        }
        u32 instr = 0x48000000 | offset & 0x3FFFFFF;
        return linked ? instr + 1 : instr;
    }
    u32 EncodeBranch(u32 start, u32 dest)
    {
        return EncodeBranch(start, dest, false);
    }
    asm void SaveRegs()
    {
        stw r0, -4(sp)
        mflr r0
        stw r0, 4(sp)
        mfctr r0
        stw r0, -8(sp)
        stfd f0, -0x10(sp)
        stfd f1, -0x18(sp)
        stfd f2, -0x20(sp)
        stfd f3, -0x28(sp)
        stfd f4, -0x30(sp)
        stfd f5, -0x38(sp)
        stfd f6, -0x40(sp)
        stfd f7, -0x48(sp)
        stfd f8, -0x50(sp)
        stfd f9, -0x58(sp)
        stfd f10, -0x60(sp)
        stfd f11, -0x68(sp)
        stfd f12, -0x70(sp)
        stfd f13, -0x78(sp)
        stwu r1, -248(sp)
        stmw rtoc, 8(sp)
        blr
    }
    asm void RestoreRegs()
    {
        lmw rtoc, 8(sp)
        addi sp, sp, 248
        lfd f0, -0x10(sp)
        lfd f1, -0x18(sp)
        lfd f2, -0x20(sp)
        lfd f3, -0x28(sp)
        lfd f4, -0x30(sp)
        lfd f5, -0x38(sp)
        lfd f6, -0x40(sp)
        lfd f7, -0x48(sp)
        lfd f8, -0x50(sp)
        lfd f9, -0x58(sp)
        lfd f10, -0x60(sp)
        lfd f11, -0x68(sp)
        lfd f12, -0x70(sp)
        lfd f13, -0x78(sp)
        lwz r0, -8(sp)
        mtctr r0
        lwz r0, 4(sp)
        mtlr r0
        lwz r0, -4(sp)
        blr
    }
} // namespace utils
