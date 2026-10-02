#include <gp4/gp4.h>

// FUNCTION: GP4 0x004320a1
void __cdecl FUN_004320a1(gp4::Regs* r) {
    void* body = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const uint32_t bit = GP4_FIELD(uint8_t, body, 0x7c) & 1u;
    // test byte [esi+0x7c], 1: CF=OF=AF=SF=0, ZF = (bit == 0), PF = parity of the masked byte
    // (the movs of eax do not touch the flags)
    const uint32_t flags = (bit == 0u) ? (0x40u | 0x04u) : 0u;
    r->eax = (bit == 0u) ? 1u : 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004320a1, FUN_004320a1, "regs(esi:ptr[0x84]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
