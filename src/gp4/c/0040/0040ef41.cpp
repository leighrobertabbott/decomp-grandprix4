#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of the byte compare `cmp left, right`.
static inline uint32_t compareFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of `test value, -1` (32-bit): CF = OF = AF = 0.
static inline uint32_t testFlags32(uint32_t value) {
    return parityFlag(value)
        | (value == 0u ? 0x40u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
}

struct Unknown_0040ef41 {
    uint8_t Unknown000[0x415];
    uint8_t Unknown415;
};

// Starts the delayed sound/effect slot (0x00418fd0, delay 0x7d0, scale 0x1388) that belongs to the
// kind stored in bits 1..7 of the byte at +0x415 (kind 0, 1, 2, 3..5 -> tags 0xe, 0xf, 0x10, 0x11),
// unless byte 0x007c2e75 is set. Kinds 6 and above, or DAT_0066d824 == 0, do nothing. eax ends as the
// tag (or, for kinds >= 6, with its low byte replaced by the kind); ecx and edx are preserved and
// the flags are those of the last compare, test or of the callee.
// FUNCTION: GP4 0x0040ef41
void __cdecl FUN_0040ef41(gp4::Regs* r) {
    if (GP4_GLOBAL(uint32_t, 0x0066d824) == 0u) {
        r->eflags = (r->eflags & ~0x8d5u) | testFlags32(0u);
        return;
    }

    const Unknown_0040ef41* body = reinterpret_cast<const Unknown_0040ef41*>(static_cast<uintptr_t>(r->esi));
    const uint8_t kind = static_cast<uint8_t>(body->Unknown415 >> 1);

    uint32_t tag;
    if (kind == 0u) {
        tag = 0xeu;
    } else if (kind == 1u) {
        tag = 0xfu;
    } else if (kind == 2u) {
        tag = 0x10u;
    } else if (static_cast<int8_t>(kind) < 6) {
        tag = 0x11u;
    } else {
        r->eax = (r->eax & 0xffffff00u) | kind;
        r->eflags = (r->eflags & ~0x8d5u) | compareFlags8(kind, 6u);
        return;
    }

    r->eax = tag;
    const uint8_t muted = GP4_GLOBAL(uint8_t, 0x007c2e75);
    uint32_t flags = compareFlags8(muted, 0u);
    if (muted == 0u) {
        gp4::Regs call = *r;
        call.st_in = 0;
        call.st_out = 0;
        call.ecx = tag;
        call.eax = 0x7d0u;
        call.edx = 0x1388u;
        gp4::call_regs(0x00418fd0, call);
        flags = call.eflags & 0x8d5u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040ef41, FUN_0040ef41, "regs(eax:u32, esi:ptr[0x420]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_0040e949")
