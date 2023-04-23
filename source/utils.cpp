#include "utils.h"
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

    asm void nop() 
    {
        nop
    }

    Vector<u8> uint16ToVector(u16 num)
    {
        u8 byte0 = num >> 8;
        u8 byte1 = num & 0xFF;
        Vector<u8> vec;
        vec.push(byte0);
        vec.push(byte1);
        return vec;
    }

    Vector<u8> uint32ToVector(u32 num)
    {
        u8 byte0 = num >> 24;
        u8 byte1 = (num & 0xFF0000) >> 16;
        u8 byte2 = (num & 0xFF00) >> 8;
        u8 byte3 = num & 0xFF;
        Vector<u8> vec;
        vec.push(byte0);
        vec.push(byte1);
        vec.push(byte2);
        vec.push(byte3);
        return vec;
    }
    const char* bit_rep[16] = {
        "0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111",
        "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111",
    };

    void print_byte(u8 byte)
    {
        OSReport("%s%s", bit_rep[byte >> 4], bit_rep[byte & 0x0F]);
    }
    void print_half(u16 half) {
        u8 byte0 = half >> 8;
        u8 byte1 = half & 0xFF;

        print_byte(byte0);
        print_byte(byte1);
    }
    void print_word(u32 word) {
        u8 byte0 = word >> 24;
        u8 byte1 = (word & 0xFF0000) >> 16;
        u8 byte2 = (word & 0xFF00) >> 8;
        u8 byte3 = word & 0xFF;

        print_byte(byte0);
        print_byte(byte1);
        print_byte(byte2);
        print_byte(byte3);
    }



    // https://mklimenko.github.io/english/2018/08/22/robust-endian-swap/
    void swapByteOrder(u16& val)
    {
        u16 us = val;
        us = (us >> 8) |
            (us << 8);
        val = us;
    }

    void swapByteOrder(u32& val)
    {
        u32 ui = val;
        ui = ((ui << 8) & 0xFF00FF00) | ((ui >> 8) & 0xFF00FF);
        ui = (ui << 16) | (ui >> 16);
        val = ui;
    }
    void swapByteOrder(float& val)
    {
        u32 ui = *((u32*)&val); // pretend it's a u32
        ui = ((ui << 8) & 0xFF00FF00) | ((ui >> 8) & 0xFF00FF);
        ui = (ui << 16) | (ui >> 16);
        val = *((float*)&ui); // back to float
    }

    void swapByteOrder(u64& val)
    {
        u64 ull = val;
        ull = ((ull & 0x00000000FFFFFFFFull) << 32) | ((ull & 0xFFFFFFFF00000000ull) >> 32);
        ull = ((ull & 0x0000FFFF0000FFFFull) << 16) | ((ull & 0xFFFF0000FFFF0000ull) >> 16);
        ull = ((ull & 0x00FF00FF00FF00FFull) << 8)  | ((ull & 0xFF00FF00FF00FF00ull) >> 8);
        val = ull;
    }





    void AddValueToByteArray(u32 value, Vector<u8> &Array)
    {
        for (int i = 0; i < 4; i++) {
            Array.push((value >> (3 * 8)) & 0xFF);
            value <<= 8;
        }
    }

    void AddValueToByteArray(u16 value, Vector<u8> &Array)
    {
        for (int i = 0; i < 2; i++) {
            Array.push((value >> 8) & 0xFF);
            value <<= 8;
        }
    }

    void AddValueToByteArray(u8 value, Vector<u8> &Array)
    {
        Array.push(value);
    }

    void AddValueToByteArray(int value, Vector<u8> &Array)
    {
        for (int i = 0; i < 4; i++) {
            Array.push((value >> (3 * 8)) & 0xFF);
            value <<= 8;
        }
    }

    void AddValueToByteArray(short value, Vector<u8> &Array)
    {
        for (int i = 0; i < 2; i++) {
            Array.push((value >> 8) & 0xFF);
            value <<= 8;
        }
    }

    void AddValueToByteArray(char value, Vector<u8> &Array)
    {
        Array.push(value);
    }
} // namespace utils
