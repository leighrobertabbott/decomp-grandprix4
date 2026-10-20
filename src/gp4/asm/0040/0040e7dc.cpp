#include <gp4/gp4.h>

struct UnknownRecord_0040e7dc {
    uint8_t Unknown00[0xe4];
    int8_t ModeE4;
    uint8_t UnknownE5[0x18f];
    uint32_t Value274;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the byte compare "cmp byte, imm8".
static inline uint32_t compareFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) << 4);
}

// Flags of the 32-bit "sub eax, mem".
static inline uint32_t subtractFlags32(uint32_t left, uint32_t right, uint32_t result) {
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result >> 24) & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) >> 20);
}

// Returns the per-record value at +0x274 in eax; for modes 2..5 (signed byte at +0xe4) it is
// DAT_0066d814 minus that value. The flags are those of the last compare/subtract executed.
// FUNCTION: GP4 0x0040e7dc
void __cdecl FUN_0040e7dc(gp4::Regs* r) {
    const UnknownRecord_0040e7dc* record = (const UnknownRecord_0040e7dc*)r->esi;
    const int32_t mode = record->ModeE4;

    uint32_t flags;
    uint32_t value;
    if (mode < 2) {
        flags = compareFlags8((uint8_t)mode, 2u);
        value = record->Value274;
    } else if (mode > 5) {
        flags = compareFlags8((uint8_t)mode, 5u);
        value = record->Value274;
    } else {
        const uint32_t base = GP4_GLOBAL(uint32_t, 0x0066d814);
        const uint32_t offset = record->Value274;
        value = base - offset;
        flags = subtractFlags32(base, offset, value);
    }

    r->eax = value;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040e7dc, FUN_0040e7dc, "regs(esi:ptr[0x27c]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
