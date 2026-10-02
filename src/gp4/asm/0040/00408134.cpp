#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408134
void __cdecl FUN_00408134(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t bit = GP4_FIELD(uint8_t, record, 0x161) & 0x02u;
    // test sets PF/ZF from the masked byte, then stc (bit set) or clc (bit clear)
    const uint32_t flags = bit != 0u ? 1u : 0x44u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408134, FUN_00408134, "regs(esi:ptr[0x169]:bytes) -> (cf, pf, af, zf, sf, of)")
