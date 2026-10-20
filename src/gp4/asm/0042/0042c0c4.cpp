#include <gp4/gp4.h>

static inline uint32_t parityEvenFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// SF/ZF/PF of a logic result (test/or/xor): CF = OF = AF = 0
static inline uint32_t logicFlags(uint32_t result, uint32_t signBit) {
    return parityEvenFlag(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & signBit) != 0u ? 0x080u : 0u);
}

// flags of `cmp left, right` (32-bit)
static inline uint32_t compareFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | parityEvenFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// 0x004062b1: signed distance between two track positions (eax - edx on the wrapping track)
static inline uint32_t trackDistance(const gp4::Regs& base, uint32_t position, uint32_t other) {
    gp4::Regs c = base;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = position;
    c.edx = other;
    gp4::call_regs(0x004062b1, c);
    return c.eax;
}

// Car record in esi: tests bit 0 of +0xe5 and bit 7 of +0x71, the track segment pointer +0x1c
// (below the pit boundary 0x00891500, with a non-zero dword +0x44) and the sign of
// hi(+0x90) ^ byte 0x00868582. CF is set when the car's segment is closer (signed track
// distance) to the pit-entry position 0x00866f00 than 0x0062d2d4 is, or, failing that,
// closer to 0x0062d2d8 than the pit-exit 0x00866f04 is; all registers are preserved
// (pushal/popal) and the other flags are those of the last test/cmp.
// FUNCTION: GP4 0x0042c0c4
void __cdecl FUN_0042c0c4(gp4::Regs* r) {
    uint8_t* const self = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t flags;
    bool carry = false;

    uint32_t t = self[0xe5] & 1u;
    flags = logicFlags(t, 0x80u);
    if (t != 0u) {
        t = self[0x71] & 0x80u;
        flags = logicFlags(t, 0x80u);
        if (t != 0u) {
            const uint32_t segment = GP4_FIELD(uint32_t, self, 0x1c);
            const uint32_t boundary = GP4_GLOBAL(uint32_t, 0x00891500);
            flags = compareFlags32(segment, boundary);
            if (segment < boundary) {
                const uint32_t link = GP4_FIELD(uint32_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(segment)), 0x44);
                flags = logicFlags(link, 0x80000000u);
                if (link != 0u) {
                    const uint32_t mixed = (GP4_FIELD(uint16_t, self, 0x90) >> 8) ^ GP4_GLOBAL(uint8_t, 0x00868582);
                    flags = logicFlags(mixed & 0xffu, 0x80u);
                    if ((mixed & 0x80u) == 0u) {
                        const uint32_t pitEntry = GP4_GLOBAL(uint32_t, 0x00866f00);
                        const uint32_t reference = trackDistance(*r, GP4_GLOBAL(uint32_t, 0x0062d2d4), pitEntry);
                        const uint32_t own = trackDistance(*r, segment, GP4_GLOBAL(uint32_t, 0x00866f00));
                        flags = logicFlags(own, 0x80000000u);
                        bool decided = false;
                        if (static_cast<int32_t>(own) >= 0) {
                            flags = compareFlags32(own, reference);
                            if (static_cast<int32_t>(own) < static_cast<int32_t>(reference)) {
                                carry = true;
                                decided = true;
                            }
                        }
                        if (!decided) {
                            const uint32_t pitExit = GP4_GLOBAL(uint32_t, 0x00866f04);
                            const uint32_t reference2 = trackDistance(*r, pitExit, GP4_GLOBAL(uint32_t, 0x0062d2d8));
                            const uint32_t own2 = trackDistance(*r, segment, GP4_GLOBAL(uint32_t, 0x0062d2d8));
                            flags = logicFlags(own2, 0x80000000u);
                            if (static_cast<int32_t>(own2) >= 0) {
                                flags = compareFlags32(own2, reference2);
                                if (static_cast<int32_t>(own2) < static_cast<int32_t>(reference2)) {
                                    carry = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    flags = (flags & ~0x001u) | (carry ? 0x001u : 0u);   // stc / clc
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0042c0c4, FUN_0042c0c4, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0+rel_0042c0c4")
