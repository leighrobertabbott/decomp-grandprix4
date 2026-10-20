#include <gp4/gp4.h>

// pushal ... popal wrapper: runs the 0x00410841 per-channel update for channels 0..3 (ecx) with
// eax = 0 on the record in esi. All registers are restored by popal; the flags left behind are
// those of the final `cmp ecx, 4` (equal: ZF and PF set, everything else clear).
// FUNCTION: GP4 0x00410894
void __cdecl FUN_00410894(gp4::Regs* r) {
    for (uint32_t channel = 0; channel < 4u; ++channel) {
        gp4::Regs callee = *r;
        callee.st_in = 0;
        callee.st_out = 0;
        callee.eax = 0;
        callee.ecx = channel;
        gp4::call_regs(0x00410841, callee);
    }
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x00410894, FUN_00410894, "regs(esi:ptr[0x358]:layout=rec_00410894) -> (cf, pf, af, zf, sf, of); globals=layout=state_00410894")
