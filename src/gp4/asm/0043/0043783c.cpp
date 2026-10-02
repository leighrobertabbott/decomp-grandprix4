#include <gp4/gp4.h>

// FUNCTION: GP4 0x0043783c  (je targets the next insn: ZF/PF only)
void __cdecl FUN_0043783c(gp4::Regs* r) {
    const uint8_t* body = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t masked = body[0x25f] & 0x40u;
    // test byte, 0x40: CF=OF=AF=SF=0; PF and ZF from the result (0 -> PF=ZF=1, 0x40 -> PF=0)
    const uint32_t flags = masked == 0u ? 0x44u : 0u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043783c, FUN_0043783c, "regs(esi:ptr[0x267]:bytes) -> (cf, pf, af, zf, sf, of)")
