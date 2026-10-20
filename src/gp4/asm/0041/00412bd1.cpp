#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `cmp left, right` (CF PF AF ZF SF OF).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

struct Unknown_00412bd1 {
    uint8_t Unknown00[0x90];
    int16_t Unknown90;
    uint8_t Unknown92[0xe5 - 0x92];
    uint8_t UnknownE5;
};

// Stores into [0x00601c28] the scaled magnitude of the word at +0x90, clamped to the
// range [0x00601c24, 0x00601c20], when bit 2 of the byte at +0xe5 is set and the byte
// [0x00622424] is zero; otherwise stores 0. eax/edx are only touched on the scaled path.
// FUNCTION: GP4 0x00412bd1
void __cdecl FUN_00412bd1(gp4::Regs* r) {
    const Unknown_00412bd1* body = reinterpret_cast<const Unknown_00412bd1*>(static_cast<uintptr_t>(r->esi));
    const uint8_t modeBits = static_cast<uint8_t>(body->UnknownE5 & 4u);
    uint32_t flags;

    if (modeBits == 0u) {
        flags = testFlags8(modeBits);
        GP4_GLOBAL(int32_t, 0x00601c28) = 0;
    } else {
        const uint8_t blocked = GP4_GLOBAL(uint8_t, 0x00622424);
        if (blocked != 0u) {
            flags = testFlags8(blocked);
            GP4_GLOBAL(int32_t, 0x00601c28) = 0;
        } else {
            int32_t magnitude = static_cast<int32_t>(body->Unknown90);
            if (magnitude < 0) {
                magnitude = -magnitude;
            }
            const int64_t product = static_cast<int64_t>(magnitude) * GP4_GLOBAL(int32_t, 0x00601c1c);
            int32_t value = static_cast<int32_t>(static_cast<uint32_t>(static_cast<uint64_t>(product) >> 14));
            const int32_t low = GP4_GLOBAL(int32_t, 0x00601c24);
            if (value < low) {
                value = low;
            }
            const int32_t high = GP4_GLOBAL(int32_t, 0x00601c20);
            flags = subFlags32(static_cast<uint32_t>(value), static_cast<uint32_t>(high));
            if (value > high) {
                value = high;
            }
            GP4_GLOBAL(int32_t, 0x00601c28) = value;
            r->eax = static_cast<uint32_t>(value);
            r->edx = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00412bd1, FUN_00412bd1, "regs(eax:u32, edx:u32, esi:ptr[0xed]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
