#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte logic operation (test/and/or): CF = OF = AF = 0.
static inline uint32_t logic8Flags(uint32_t value) {
    return parityFlag8(value) | ((value & 0xffu) == 0u ? 0x40u : 0u) | ((value & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 32-bit `test dword, -1`.
static inline uint32_t test32Flags(uint32_t value) {
    return parityFlag8(value) | (value == 0u ? 0x40u : 0u) | ((value >> 24) & 0x80u);
}

static inline uint32_t add32Flags(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of a byte `cmp left, right`.
static inline uint32_t cmp8Flags(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

struct Unknown_00434930 {
    uint8_t Unknown00[0xe];
    int16_t UnknownE;
    uint8_t Unknown10[0x90 - 0x10];
    int16_t Unknown90;
    uint8_t Unknown92[0xcc - 0x92];
    uint8_t FlagsCC;
    uint8_t FlagsCD;
    uint8_t UnknownCE[0xf0 - 0xce];
    int32_t UnknownF0;
    uint8_t UnknownF4[0x200 - 0xf4];
    uint8_t Unknown200;
};

// FUNCTION: GP4 0x00434930
// Carry-flag predicate on a car body (esi) with a reference word at [edi]. CF = 1 ("busy") when
// flag 0x40 of byte +0xcd is already set, or while the timer +0xf0 (compared with the clock
// 0x0066d814) is running, or after (re)starting it; CF = 0 otherwise. When the timer has expired the
// body flags are updated: 0x40 set, 0x20 cleared and 0x10 set when
// ((0x4000 - |[esi+0xe] - [edi]|) ^ [esi+0x90]) is negative (16-bit); ax keeps that value.
// The flags are those of the last flag writer on the taken path with CF forced by stc/clc.
void __cdecl FUN_00434930(gp4::Regs* r) {
    Unknown_00434930* body = reinterpret_cast<Unknown_00434930*>(static_cast<uintptr_t>(r->esi));
    const int16_t* reference = reinterpret_cast<const int16_t*>(static_cast<uintptr_t>(r->edi));
    const uint32_t eaxIn = r->eax;
    uint32_t eax = eaxIn;
    uint32_t flags;
    bool carry = true;

    if ((body->FlagsCD & 0x40u) != 0u) {
        flags = logic8Flags(body->FlagsCD & 0x40u);
    } else if ((body->FlagsCD & 0x20u) != 0u) {
        bool expired = true;
        flags = test32Flags(static_cast<uint32_t>(body->UnknownF0));
        if (body->UnknownF0 != 0) {
            const uint32_t negated = 0u - static_cast<uint32_t>(body->UnknownF0);
            const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
            flags = add32Flags(negated, clock);
            expired = (negated + clock) < 0x80000000u;
        }
        if (expired) {
            body->FlagsCD = static_cast<uint8_t>(body->FlagsCD | 0x40u);
            body->FlagsCD = static_cast<uint8_t>(body->FlagsCD & 0xdfu);
            uint16_t ax = static_cast<uint16_t>(static_cast<uint16_t>(body->UnknownE) - static_cast<uint16_t>(*reference));
            if ((ax & 0x8000u) != 0u) ax = static_cast<uint16_t>(0u - ax);
            ax = static_cast<uint16_t>(0u - ax);
            ax = static_cast<uint16_t>(ax + 0x4000u);
            ax = static_cast<uint16_t>(ax ^ static_cast<uint16_t>(body->Unknown90));
            if ((ax & 0x8000u) != 0u) {
                body->FlagsCD = static_cast<uint8_t>(body->FlagsCD | 0x10u);
            } else {
                body->FlagsCD = static_cast<uint8_t>(body->FlagsCD & 0xefu);
            }
            flags = logic8Flags(body->FlagsCD);
            eax = (eaxIn & 0xffff0000u) | ax;
        }
    } else {
        flags = logic8Flags(body->FlagsCC & 4u);
        carry = false;
        if ((body->FlagsCC & 4u) != 0u) {
            flags = cmp8Flags(body->Unknown200, 2u);
            bool start = body->Unknown200 == 2u;
            if (!start) {
                flags = cmp8Flags(body->Unknown200, 1u);
                start = body->Unknown200 == 1u;
            }
            if (start) {
                const uint32_t clock = GP4_GLOBAL(uint32_t, 0x0066d814);
                body->UnknownF0 = static_cast<int32_t>(clock);
                body->UnknownF0 = static_cast<int32_t>(clock + 3000u);
                flags = add32Flags(clock, 3000u);
                body->FlagsCD = static_cast<uint8_t>(body->FlagsCD | 0x20u);
                flags = logic8Flags(body->FlagsCD);
                carry = true;
            }
        }
    }

    flags = (flags & ~1u) | (carry ? 1u : 0u);
    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00434930, FUN_00434930, "regs(eax:u32, esi:ptr[0x208]:bytes, edi:ptr[0xa]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
