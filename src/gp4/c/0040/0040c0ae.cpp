#include <gp4/gp4.h>

// Flags of "add left, right".
static inline uint32_t additionFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((parity & 1u) == 0u ? 4u : 0u)
        | (result < left ? 1u : 0u)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Every record of the 0x160-byte ring at 0x008943a0 holds, at +0x7a, a word that selects a
// 5-word (10-byte) entry of the table whose entry 1 starts at 0x00675a82.
static inline uint32_t entryOffset(uint32_t record) {
    return static_cast<uint32_t>(GP4_FIELD(uint16_t, static_cast<uintptr_t>(record), 0x7a)) * 10u;
}

static inline uint16_t* entryAt(uint32_t offset) {
    return reinterpret_cast<uint16_t*>(static_cast<uintptr_t>(0x00675a82u + offset));
}

// FUNCTION: GP4 0x0040c0ae
void __cdecl FUN_0040c0ae(gp4::Regs* r) {
    const uint32_t ringStart = 0x008943a0u;

    // Exchange the entries of the first record and of the record before the ring end.
    {
        uint16_t* a = entryAt(entryOffset(ringStart));
        uint16_t* b = entryAt(entryOffset(GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x160u));
        for (int i = 0; i < 5; ++i) {
            const uint16_t ax = a[i];
            const uint16_t dx = b[i];
            b[5 + i] = ax;
            a[i - 5] = dx;
        }
    }
    {
        uint16_t* a = entryAt(entryOffset(GP4_GLOBAL(uint32_t, 0x00891500)));
        uint16_t* b = entryAt(entryOffset(GP4_GLOBAL(uint32_t, 0x00866f00) - 0x160u));
        for (int i = 0; i < 5; ++i) {
            a[i - 5] = b[i];
        }
    }
    const uint32_t lastOffset = entryOffset(GP4_GLOBAL(uint32_t, 0x00866f04));
    {
        uint16_t* a = entryAt(entryOffset(GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u));
        uint16_t* b = entryAt(lastOffset);
        for (int i = 0; i < 5; ++i) {
            a[5 + i] = b[i];
        }
    }
    // eax, ebx, edi and edx are restored; the flags are those of the last "add ebx, 0x675a82".
    r->eflags = (r->eflags & ~0x8d5u) | additionFlags32(lastOffset, 0x00675a82u);
}
GP4_IMPL(0x0040c0ae, FUN_0040c0ae, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0040c0ae")
