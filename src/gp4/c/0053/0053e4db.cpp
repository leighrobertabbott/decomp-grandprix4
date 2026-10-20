#include <gp4/gp4.h>
#include <gp4/flags.h>

// push edx; xor edx, edx; call 0x0053e4e5; pop edx; ret
// Runs the item-chain flag walk 0x0053e4e5 with edx = 0 (the high bits shifted into the item flags).
// The callee is a pushal/popal routine, so every register comes back as it went in and edx is
// restored by the pop; only the flags of the callee's last flag writer are left.
// FUNCTION: GP4 0x0053e4db
void __cdecl FUN_0053e4db(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.edx = 0u;
    gp4::call_regs(0x0053e4e5, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0053e4db, FUN_0053e4db, "regs(eax:u32, esi:ptr[0x28]:layout=snode1_0053e56a) -> (cf, pf, af, zf, sf, of)")
