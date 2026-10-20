#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (test / and): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffu) == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// Flags of a 16-bit logic result (test / xor): CF = OF = AF = 0.
static inline uint32_t logicFlags16(uint32_t result) {
    return parityFlag8(result)
        | ((result & 0xffffu) == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u);
}

// Flags of the 16-bit `left - right` (cmp / sub / neg); operands already masked to 16 bits.
static inline uint32_t subFlags16(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `left - right` (cmp).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Leaves the general registers alone (pushal / popal) and sets the flags of the last flag-writing
// instruction with CF forced by the clc / stc epilogue.
static inline void finish(gp4::Regs* r, uint32_t flags, bool carry) {
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~0x01u) | (carry ? 0x01u : 0u);
}

static inline bool testByte(uint8_t* record, uint32_t off, uint32_t mask, uint32_t* flags) {
    const uint32_t value = GP4_FIELD(uint8_t, record, off) & mask;
    *flags = logicFlags8(value);
    return value != 0u;
}

// FUNCTION: GP4 0x0042eee3
// CF = 1 when the record in ebx (a car) is about to overtake / run into the one in esi: neither is
// flagged (+0xe5 bit 4, +0xce bit 0x80, words +0xea zero, +0xfe and +0x135 bit 0x40 clear, bit 4 of
// byte +0x12 of the dword-+0x1c blocks clear), the distance (word +0xe2 difference, or 0x40dc8a when
// bit 1 of byte +0x9c is set on either) is below word 0x603af6, dword +0x1a0 of ebx is below dword
// 0x62236c, the word +0x68 difference d is positive with 2*d < (byte +0x10 of the esi block & 0x7f),
// and the words +0x90 / +0x94 of ebx have opposite signs after the subtraction with |+0x90 - +0x94|
// at least 0x140. All general registers are preserved; the flags are those of the last compare.
void __cdecl FUN_0042eee3(gp4::Regs* r) {
    uint8_t* second = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->ebx));
    uint8_t* first = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;

    if (testByte(second, 0xe5, 4u, &flags)) { finish(r, flags, false); return; }
    if (testByte(first, 0xce, 0x80u, &flags)) { finish(r, flags, false); return; }

    uint32_t word = GP4_FIELD(uint16_t, first, 0xea);
    flags = subFlags16(word, 0u);
    if (word != 0u) { finish(r, flags, false); return; }
    word = GP4_FIELD(uint16_t, second, 0xea);
    flags = subFlags16(word, 0u);
    if (word != 0u) { finish(r, flags, false); return; }

    if (testByte(first, 0xfe, 0x40u, &flags)) { finish(r, flags, false); return; }
    if (testByte(second, 0xfe, 0x40u, &flags)) { finish(r, flags, false); return; }

    uint8_t* firstBlock = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, first, 0x1c)));
    uint8_t* secondBlock = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_FIELD(uint32_t, second, 0x1c)));
    if (testByte(firstBlock, 0x12, 4u, &flags)) { finish(r, flags, false); return; }
    if (testByte(secondBlock, 0x12, 4u, &flags)) { finish(r, flags, false); return; }

    uint32_t distance;
    if ((GP4_FIELD(uint8_t, first, 0x9c) & 2u) != 0u || (GP4_FIELD(uint8_t, second, 0x9c) & 2u) != 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        gp4::call_regs(0x0040dc8a, call);
        distance = call.eax & 0xffffu;
    } else {
        distance = (GP4_FIELD(uint16_t, second, 0xe2) - static_cast<uint32_t>(GP4_FIELD(uint16_t, first, 0xe2))) & 0xffffu;
    }
    const uint32_t limit = GP4_GLOBAL(uint16_t, 0x00603af6);
    flags = subFlags16(distance, limit);
    if (static_cast<int16_t>(distance) >= static_cast<int16_t>(limit)) { finish(r, flags, false); return; }

    if (testByte(first, 0x135, 0x40u, &flags)) { finish(r, flags, false); return; }
    if (testByte(second, 0x135, 0x40u, &flags)) { finish(r, flags, false); return; }

    const uint32_t speed = GP4_FIELD(uint32_t, second, 0x1a0);
    const uint32_t speedLimit = GP4_GLOBAL(uint32_t, 0x0062236c);
    flags = subFlags32(speed, speedLimit);
    if (static_cast<int32_t>(speed) >= static_cast<int32_t>(speedLimit)) { finish(r, flags, false); return; }

    const uint32_t reach = GP4_FIELD(uint8_t, firstBlock, 0x10) & 0x7fu;
    const uint32_t lapDiff = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(
        (GP4_FIELD(uint16_t, second, 0x68) - static_cast<uint32_t>(GP4_FIELD(uint16_t, first, 0x68))) & 0xffffu)));
    flags = subFlags32(lapDiff, 1u);
    if (static_cast<int32_t>(lapDiff) < 1) { finish(r, flags, false); return; }
    const uint32_t doubled = lapDiff << 1;
    flags = subFlags32(doubled, reach);
    if (static_cast<int32_t>(doubled) >= static_cast<int32_t>(reach)) { finish(r, flags, false); return; }

    const uint32_t reference = GP4_FIELD(uint16_t, second, 0x94);
    uint32_t delta = (GP4_FIELD(uint16_t, second, 0x90) - reference) & 0xffffu;
    const uint32_t signs = (delta ^ reference) & 0xffffu;
    flags = logicFlags16(signs);
    if ((signs & 0x8000u) == 0u) { finish(r, flags, false); return; }

    flags = logicFlags16(delta);
    if ((delta & 0x8000u) != 0u) {
        flags = subFlags16(0u, delta);
        delta = (0u - delta) & 0xffffu;
    }
    flags = subFlags16(delta, 0x140u);
    if (static_cast<int16_t>(delta) < 0x140) { finish(r, flags, false); return; }
    finish(r, flags, true);
}
GP4_IMPL(0x0042eee3, FUN_0042eee3, "regs(ebx:rec[0x66fef0,0x41c,0..21], esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+pair_0042e8e6+state_0042eee3; cases=1000")
