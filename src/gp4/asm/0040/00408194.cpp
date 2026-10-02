#include <gp4/gp4.h>

// FUNCTION: GP4 0x00408194
void __cdecl FUN_00408194(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t bit = GP4_FIELD(uint8_t, record, 0x122) & 0x02u;
    // bit set: test result 2 (PF=0, ZF=0, SF=0) + stc -> CF only
    // bit clear: test result 0 (PF=1, ZF=1) + clc -> PF|ZF
    const uint32_t flags = bit != 0u ? 1u : 0x44u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408194, FUN_00408194, "regs(esi:ptr[0x12a]:bytes) -> (cf, pf, af, zf, sf, of)")
