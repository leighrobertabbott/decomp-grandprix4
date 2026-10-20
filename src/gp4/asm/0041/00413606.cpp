#include <gp4/gp4.h>

// If bit 3 of the byte at +0xe6 of the record at esi is clear, ax = the word at +0x23a (the
// upper half of eax is kept; the flags are those of the `test` that gave zero: ZF = PF = 1).
// Otherwise the result and the flags are those of 0x00413170. Everything else is pushed and
// popped, so only eax and the flags leave.
// FUNCTION: GP4 0x00413606
void __cdecl FUN_00413606(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    if ((record[0xe6] & 8u) == 0u) {
        const uint32_t word = *reinterpret_cast<const uint16_t*>(record + 0x23a);
        r->eax = (r->eax & 0xffff0000u) | word;
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }

    gp4::Regs inner = *r;
    gp4::call_regs(0x00413170, inner);
    r->eax = inner.eax;
    r->eflags = (r->eflags & ~0x8d5u) | (inner.eflags & 0x8d5u);
}
GP4_IMPL(0x00413606, FUN_00413606, "regs(eax:u32, esi:ptr[0x242]:bytes, edi:rec[0x8943a0,0x160,0..29]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=ring_008943a0+words_00413090+engine_00413606")
