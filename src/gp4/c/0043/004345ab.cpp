#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of the 16-bit `cmp lhs, rhs` (lhs - rhs).
static inline uint32_t cmpFlags16(uint16_t lhs, uint16_t rhs) {
    const uint32_t result = static_cast<uint16_t>(lhs - rhs);
    return (lhs < rhs ? 1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | ((((lhs ^ rhs) & (lhs ^ result)) & 0x8000u) != 0u ? 0x800u : 0u);
}

// Steps the record in edi along its link and re-walks the point at esi (0x00406123); with a link,
// rotates the point into the record's frame (0x0040435d) and tests the lateral offset left at
// 0x00934df0 against the record's half widths: |offset| <= width - 0x50, with the width the word at
// +0x60 for a negative offset and +0x62 otherwise. CF clear = inside, set = outside or no link. All
// registers are preserved (pushal/popal); the flags are those of the final 16-bit compare (or of
// 0x00406123 when it reports no link), with CF forced by clc/stc.
// FUNCTION: GP4 0x004345ab
void __cdecl FUN_004345ab(gp4::Regs* r) {
    gp4::Regs call = *r;
    gp4::call_regs(0x00406123, call);
    if ((call.eflags & 1u) != 0u) {
        // jb: popal; stc keeps the other flags of 0x00406123
        r->eflags = (r->eflags & ~0x8d5u) | (call.eflags & 0x8d5u) | 1u;
        return;
    }
    gp4::call_regs(0x0040435d, call);

    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(call.edi));
    uint16_t offset = GP4_GLOBAL(uint16_t, 0x00934df0);
    uint16_t width;
    if (static_cast<int16_t>(offset) < 0) {
        offset = static_cast<uint16_t>(0u - offset);
        width = static_cast<uint16_t>(GP4_FIELD(uint16_t, record, 0x60) - 0x50u);
    } else {
        width = static_cast<uint16_t>(GP4_FIELD(uint16_t, record, 0x62) - 0x50u);
    }
    uint32_t flags = cmpFlags16(offset, width);
    // jg: ZF clear and SF == OF -> stc, otherwise clc
    const bool greater = (flags & 0x40u) == 0u && (((flags >> 7) & 1u) == ((flags >> 11) & 1u));
    flags = (flags & ~1u) | (greater ? 1u : 0u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004345ab, FUN_004345ab, "regs(esi:ptr[0x40]:layout=pos_00405cf8, edi:rec[0x8943a0,0x160,0..25]) -> (cf, pf, af, zf, sf, of); globals=layout=ring_00405a12+links_00405a12")
