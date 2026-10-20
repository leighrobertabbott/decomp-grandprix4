#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags-only test for edi == &DAT_008a0498 (the address, not the contents): leaves the flags of
// `cmp edi, 0x8a0498`, except that when equal the following `or eax, eax` replaces them with the
// logic flags of eax. Registers are unchanged.
// FUNCTION: GP4 0x0055b83a
void __cdecl FUN_0055b83a(gp4::Regs* r) {
    const uint32_t address = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(GP4_ARRAY(uint8_t, 0x008a0498)));
    uint32_t flags = gp4::flags::sub32(r->edi, address);
    if (r->edi == address) {
        flags = gp4::flags::logic32(r->eax);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0055b83a, FUN_0055b83a, "regs(eax:i32[-2..2], edi:u32[0x8a0494..0x8a049c]) -> (cf, pf, af, zf, sf, of)")
