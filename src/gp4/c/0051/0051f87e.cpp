#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: runs 0x00433c4c with esi = the record pointer [0x00a69878] and stores its three
// results eax, edx, ecx in 0x00a6987c, 0x00a69880 and 0x00a69884. popal restores every GPR; the
// flags stay those 0x00433c4c leaves.
// FUNCTION: GP4 0x0051f87e
void __cdecl FUN_0051f87e(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    c.esi = GP4_GLOBAL(uint32_t, 0x00a69878);
    gp4::call_regs(0x00433c4c, c);
    GP4_GLOBAL(uint32_t, 0x00a6987c) = c.eax;
    GP4_GLOBAL(uint32_t, 0x00a69880) = c.edx;
    GP4_GLOBAL(uint32_t, 0x00a69884) = c.ecx;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x0051f87e, FUN_0051f87e, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carptr_0051f87e+state_00431ffb+state_004059ef+geom_008943a0+rec_00433c4c+off_00433c4c+saved_00934de8")
