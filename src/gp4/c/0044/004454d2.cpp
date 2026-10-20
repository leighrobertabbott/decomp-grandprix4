#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags left by an OR / AND / TEST: CF = OF = AF = 0, ZF/SF/PF from the result.
static inline uint32_t logicFlags8(uint32_t result) {
    return parityFlag(result) | ((result & 0xffu) == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

static inline uint32_t logicFlags32(uint32_t result) {
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Clears bit 7 of byte +0x78 of the car records whose pointers sit in 0x0066d820 and 0x0066d828 (each skipped
// when null). esi ends as [0x0066d828]; flags are those of the last flag writer (the `or esi, esi` test or the
// byte AND).
// FUNCTION: GP4 0x004454d2
void __cdecl FUN_004454d2(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x0066d820));
    uint32_t flags = logicFlags32(reinterpret_cast<uint32_t>(record));
    if (record != 0) {
        const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x78) & 0x7fu);
        GP4_FIELD(uint8_t, record, 0x78) = value;
        flags = logicFlags8(value);
    }

    record = reinterpret_cast<uint8_t*>(GP4_GLOBAL(uint32_t, 0x0066d828));
    flags = logicFlags32(reinterpret_cast<uint32_t>(record));
    if (record != 0) {
        const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x78) & 0x7fu);
        GP4_FIELD(uint8_t, record, 0x78) = value;
        flags = logicFlags8(value);
    }

    r->esi = reinterpret_cast<uint32_t>(record);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004454d2, FUN_004454d2, "regs() -> (esi, cf, pf, af, zf, sf, of); globals=layout=state_0040fa01")
