#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Flags of a 32-bit `add left, right` (CF PF AF ZF SF OF).
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
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

// Index of the record's slot: (0x43207c's slot address, advanced by 0x160 by 0x432c03 for kinds 2/3,
// minus the dword at +0x1c) / 0x160, plus 1 for kind 2 and 2 for kind 3 (byte +0x7e). edx and ebp
// are saved and restored. The flags are those of the last add (kinds 2, 3) or of the compare with 3.
// FUNCTION: GP4 0x00432c21
void __cdecl FUN_00432c21(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x0043207c, callee);
    gp4::call_regs(0x00432c03, callee);

    const uint32_t difference = callee.eax - GP4_FIELD(uint32_t, record, 0x1c);
    int32_t remainder;
    int32_t index = gp4::idiv64(static_cast<int64_t>(static_cast<int32_t>(difference)), 0x160, &remainder);

    const uint32_t kind = GP4_FIELD(uint8_t, record, 0x7e);
    uint32_t flags;
    if (kind == 2u) {
        flags = addFlags32(static_cast<uint32_t>(index), 1u);
        index += 1;
    } else if (kind == 3u) {
        flags = addFlags32(static_cast<uint32_t>(index) + 1u, 1u);
        index += 2;
    } else {
        flags = compareFlags8(kind, 3u);
    }

    r->eax = static_cast<uint32_t>(index);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432c21, FUN_00432c21, "regs(esi:ptr[0x105]:bytes) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
