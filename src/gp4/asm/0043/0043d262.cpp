#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043d262
void __cdecl FUN_0043d262(gp4::Regs* r) {
    // CF = 1 when eax is non-null and the byte at [[eax + 0x1c] + 0x12] has both bit 0 and bit 1 set.
    // Every other path ends with "clc" after a test/or whose result is zero: ZF|PF.
    // The successful path ends with "stc" after "test byte, 2" whose result is 2: only CF.
    uint8_t* const root = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->eax));
    bool both = false;
    if (root != nullptr) {
        uint8_t* const link = GP4_FIELD(uint8_t*, root, 0x1c);
        const uint8_t flagsByte = GP4_FIELD(uint8_t, link, 0x12);
        both = (flagsByte & 1u) != 0u && (flagsByte & 2u) != 0u;
    }
    const uint32_t flags = both ? 0x01u : 0x44u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043d262, FUN_0043d262, "regs(eax:nptr[0x20]:layout=blk_0043d262) -> (cf, pf, af, zf, sf, of)")
