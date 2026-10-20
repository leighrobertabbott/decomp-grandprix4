#include <gp4/gp4.h>
#include <gp4/flags.h>

// ORs together the dwords of the table at 0x00875dc8 selected by the set bits of ax (bit k selects
// dword k). eax = the OR; ebx and edx are saved and restored. The flags are those of the last
// `shr ax, 1`, which always shifts a zero (ZF and PF set).
// FUNCTION: GP4 0x005581ba
void __cdecl FUN_005581ba(gp4::Regs* r) {
    const uint32_t* table = GP4_ARRAY(uint32_t, 0x00875dc8);
    uint16_t bits = static_cast<uint16_t>(r->eax);
    uint32_t merged = 0u;
    for (uint32_t k = 0; bits != 0u; ++k, bits = static_cast<uint16_t>(bits >> 1)) {
        if ((bits & 1u) != 0u) {
            merged |= table[k];
        }
    }
    r->eax = merged;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic16(0u));
}
GP4_IMPL(0x005581ba, FUN_005581ba, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=masks_00875dc8")
