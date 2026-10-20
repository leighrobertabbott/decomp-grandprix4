#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags (CF/PF/AF/ZF/SF/OF) of `left - right` (cmp) at the given operand width.
static inline uint32_t cmpFlags(uint32_t left, uint32_t right, uint32_t bits) {
    const uint32_t mask = bits == 32u ? 0xffffffffu : ((1u << bits) - 1u);
    const uint32_t sign = 1u << (bits - 1u);
    left &= mask;
    right &= mask;
    const uint32_t result = (left - right) & mask;
    return (left < right ? 1u : 0u) | parityFlag(result) | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | ((result & sign) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & sign) != 0u ? 0x800u : 0u);
}

// Predicate for the record at esi: CF = 1 when its level byte [+0x235] is at least 3, the bearing/distance
// from FUN_00414439 gives a distance (edx) below 0x6224fc and the bearing differs from the word [esi+0xe]
// by less than 0x4000 (16-bit signed compare of the absolute difference). eax and edx are preserved.
// FUNCTION: GP4 0x00414562
void __cdecl FUN_00414562(gp4::Regs* r) {
    const uint8_t* rec = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t flags;
    bool carry = false;

    const uint32_t level = rec[0x235];
    flags = cmpFlags(level, 3u, 8);
    if (level >= 3u) {
        gp4::Regs c = *r;
        c.st_in = 0;
        c.st_out = 0;
        gp4::call_regs(0x00414439, c);

        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x006224fc);
        flags = cmpFlags(c.edx, limit, 32);
        if (static_cast<int32_t>(c.edx) < static_cast<int32_t>(limit)) {
            uint16_t ax = static_cast<uint16_t>(c.eax - *reinterpret_cast<const uint16_t*>(rec + 0xe));
            if (static_cast<int16_t>(ax) < 0) ax = static_cast<uint16_t>(0u - ax);
            flags = cmpFlags(ax, 0x4000u, 16);
            carry = static_cast<int16_t>(ax) < 0x4000;
        }
    }

    flags = carry ? (flags | 1u) : (flags & ~1u);  // stc / clc
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00414562, FUN_00414562, "regs(eax:u32, edx:u32, esi:ptr[0x23d]:layout=rec_00414562) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
