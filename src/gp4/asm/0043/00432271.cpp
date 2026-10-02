#include <gp4/gp4.h>

// FUNCTION: GP4 0x00432271
void __cdecl FUN_00432271(gp4::Regs* r) {
    void* body = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const uint8_t bit = GP4_FIELD(uint8_t, body, 0x26b) & 0x08u;
    // test byte [esi+0x26b], 8 then stc/clc:
    // bit set: result 8 (PF=0, ZF=0, SF=0) + stc -> CF only
    // bit clear: result 0 (PF=1, ZF=1) + clc -> PF|ZF
    const uint32_t flags = bit != 0u ? 1u : 0x44u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00432271, FUN_00432271, "regs(esi:ptr[0x273]:bytes) -> (cf, pf, af, zf, sf, of)")
