#include <gp4/gp4.h>
#include <gp4/flags.h>

// Control-bit test through the code byte [0x007ab567]. Code 0 = unbound: CF cleared, the other
// flags of `cmp bl, 0`, eax untouched. Otherwise FUN_0051ddda (bl = code) is asked: its carry
// (code with no bit) gives CF cleared and its eax; else CF is set (stc) with eax = 1 when the bit is
// set (callee ZF clear) and 0 when it is clear (ZF set); the other flags are the callee's. ebx, ecx,
// edx, ebp, esi and edi are saved and restored.
// FUNCTION: GP4 0x0051dd08
void __cdecl FUN_0051dd08(gp4::Regs* r) {
    const uint32_t code = GP4_GLOBAL(uint8_t, 0x007ab567);
    if (code == 0u) {
        r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub8(code, 0u) & ~gp4::flags::CF);
        return;
    }
    gp4::Regs query = *r;
    query.ebx = (r->ebx & 0xffffff00u) | code;
    gp4::call_regs(0x0051ddda, query);
    if ((query.eflags & gp4::flags::CF) != 0u) {
        r->eax = query.eax;
        r->eflags = gp4::flags::merge(r->eflags, query.eflags & ~gp4::flags::CF);
        return;
    }
    r->eax = (query.eflags & gp4::flags::ZF) != 0u ? 0u : 1u;
    r->eflags = gp4::flags::merge(r->eflags, query.eflags | gp4::flags::CF);
}
GP4_IMPL(0x0051dd08, FUN_0051dd08, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=layout=bits_006f2d90+codes_0051dd08")
