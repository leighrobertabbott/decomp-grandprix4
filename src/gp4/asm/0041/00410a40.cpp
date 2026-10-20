#include <gp4/gp4.h>

struct UnknownSource_00410a40 {
    uint8_t Unknown00[0xb];
    uint8_t FlagsB;
};

struct UnknownTarget_00410a40 {
    uint8_t Unknown00[0xcf];
    uint8_t FlagsCF;
};

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Mirrors bit 7 of the byte at ebx + 0xb into bit 4 of the byte at esi + 0xcf and into the byte
// global DAT_00601f1f (0xff when set, else 0). The flags are those of the final "or"/"and" on
// the esi byte (CF = OF = AF = 0).
// FUNCTION: GP4 0x00410a40
void __cdecl FUN_00410a40(gp4::Regs* r) {
    const UnknownSource_00410a40* source = (const UnknownSource_00410a40*)r->ebx;
    UnknownTarget_00410a40* target = (UnknownTarget_00410a40*)r->esi;

    uint8_t result;
    if ((source->FlagsB & 0x80u) != 0u) {
        result = (uint8_t)(target->FlagsCF | 0x10u);
        target->FlagsCF = result;
        GP4_GLOBAL(uint8_t, 0x00601f1f) = 0xff;
    } else {
        result = (uint8_t)(target->FlagsCF & 0xefu);
        target->FlagsCF = result;
        GP4_GLOBAL(uint8_t, 0x00601f1f) = 0;
    }

    const uint32_t flags = parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00410a40, FUN_00410a40, "regs(ebx:ptr[0x40]:bytes, esi:ptr[0xd8]:bytes) -> (cf, pf, af, zf, sf, of)")
