#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 16-bit `cmp left, right` except CF.
static inline uint32_t cmp16FlagsNoCarry(uint16_t left, uint16_t right) {
    const uint32_t result = static_cast<uint16_t>(left - right);
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x004080eb
void __cdecl FUN_004080eb(gp4::Regs* r) {
    // CF = 1 when byte [esi+0x8d] has bit 2, or (byte [esi+0x71] has bit 0x40, byte [esi+0x1ce]
    // has no bit 1 and, with the sign of word [esi+0x90] equal to the sign of the signed byte
    // [0x868582], |word [esi+0x90]| >= word [esi+0xb0] as signed words). eax is saved/restored.
    // The other flags are those of the last flag writer (test, xor or cmp) before stc/clc.
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;
    if ((GP4_FIELD(uint8_t, body, 0x8d) & 2u) != 0u) {
        flags = 1u;                                   // test result 2: PF = 0, ZF = 0; stc
    } else if ((GP4_FIELD(uint8_t, body, 0x71) & 0x40u) == 0u) {
        flags = 0x44u;                                // test result 0: PF = 1, ZF = 1; clc
    } else if ((GP4_FIELD(uint8_t, body, 0x1ce) & 1u) != 0u) {
        flags = 1u;                                   // test result 1: PF = 0, ZF = 0; stc
    } else {
        const uint16_t mixed = static_cast<uint16_t>(static_cast<int16_t>(GP4_GLOBAL(int8_t, 0x00868582)))
            ^ GP4_FIELD(uint16_t, body, 0x90);
        if ((mixed & 0x8000u) != 0u) {
            flags = parityFlag(mixed) | 0x80u;        // xor result negative: SF = 1; clc
        } else {
            int16_t value = GP4_FIELD(int16_t, body, 0x90);
            if (value < 0) value = static_cast<int16_t>(-value);
            const int16_t limit = GP4_FIELD(int16_t, body, 0xb0);
            flags = cmp16FlagsNoCarry(static_cast<uint16_t>(value), static_cast<uint16_t>(limit))
                | (value >= limit ? 1u : 0u);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004080eb, FUN_004080eb, "regs(esi:ptr[0x1d6]:bytes) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
