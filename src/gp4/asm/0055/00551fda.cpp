#include <gp4/gp4.h>
#include <gp4/flags.h>

// Same test as 0x00551fcf with ecx as the index: leaves the flags of
// `cmp byte ptr [ecx + 0x954e04], dl`, except that when the byte equals dl the following
// `or ecx, ecx` replaces them with the logic flags of ecx. Registers are unchanged.
// FUNCTION: GP4 0x00551fda
void __cdecl FUN_00551fda(gp4::Regs* r) {
    const uint8_t* table = GP4_ARRAY(uint8_t, 0x00954e04);
    const uint32_t entry = table[r->ecx];
    const uint32_t wanted = r->edx & 0xffu;
    uint32_t flags = gp4::flags::sub8(entry, wanted);
    if (entry == wanted) {
        flags = gp4::flags::logic32(r->ecx);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00551fda, FUN_00551fda, "regs(ecx:u32[0..7], edx:u32[0..255]) -> (cf, pf, af, zf, sf, of)")
