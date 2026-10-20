#include <gp4/gp4.h>

// FUNCTION: GP4 0x004081a1
// Carry set when bits 1 and 4 of byte [esi+0xe5] are both set, or bit 0x40 of byte [esi+0xcc] is set.
void __cdecl FUN_004081a1(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint8_t modeBits = GP4_FIELD(uint8_t, record, 0xe5);
    const uint8_t stateBits = GP4_FIELD(uint8_t, record, 0xcc);
    const bool set = ((modeBits & 1u) != 0u && (modeBits & 4u) != 0u) || (stateBits & 0x40u) != 0u;
    // set: the last test left a nonzero single-bit result (PF=ZF=SF=0) and stc -> CF only
    // clear: the last test left 0 (PF=ZF=1) and clc -> PF|ZF
    const uint32_t flags = set ? 1u : 0x44u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004081a1, FUN_004081a1, "regs(esi:ptr[0xed]:bytes) -> (cf, pf, af, zf, sf, of)")
