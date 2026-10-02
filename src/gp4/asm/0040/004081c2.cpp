#include <gp4/gp4.h>

// FUNCTION: GP4 0x004081c2
void __cdecl FUN_004081c2(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t bit = GP4_FIELD(uint8_t, record, 0x7b) & 0x40u;
    const uint32_t flags = bit != 0u ? 1u : 0x44u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004081c2, FUN_004081c2, "regs(esi:ptr[0x83]:bytes) -> (cf, pf, af, zf, sf, of)")
