#pragma once

#include <types.h>
#include <vector.h>
#include <mem.h>
#include <OS/OSError.h>

namespace utils {
    u32 EncodeBranch(u32 start, u32 dest, bool linked);
    u32 EncodeBranch(u32 start, u32 dest);
    asm void SaveRegs();
    asm void RestoreRegs();

    asm void nop();

    Vector<u8> uint16ToVector(u16 num);
    Vector<u8> uint32ToVector(u32 num);
    void print_byte(u8 byte);
    void print_half(u16 half);
    void print_word(u32 word);

    void swapByteOrder(u16& val);
    void swapByteOrder(u32& val);
    void swapByteOrder(u64& val);
    void swapByteOrder(float& val);

    void AddValueToByteArray(u32 value, Vector<u8> &Array);
    void AddValueToByteArray(u16 value, Vector<u8> &Array);
    void AddValueToByteArray(u8 value, Vector<u8> &Array);
    void AddValueToByteArray(int value, Vector<u8> &Array);
    void AddValueToByteArray(short value, Vector<u8> &Array);
    void AddValueToByteArray(char value, Vector<u8> &Array);
} // namespace utils

template<typename T>
T bufferToObject(u8* buffer)
{
  T obj;
  memmove(&obj, buffer, sizeof(T));
  return obj;
}

template <class T>
T swap_endian(T in)
{
    char *const p = reinterpret_cast<char *>(&in);
    for (size_t i = 0; i < sizeof(T) / 2; ++i)
        std::swap(p[i], p[sizeof(T) - i - 1]);
    return in;
}
