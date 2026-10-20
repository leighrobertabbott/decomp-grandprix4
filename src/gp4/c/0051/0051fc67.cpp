#include <gp4/gp4.h>

// Resets the 0x00412708 timing setup: [0x006f2e90] = the value 0x0041276c returns, words/dwords
// 0x0066d86c, 0x0066d814, 0x0066d86e, 0x0066d85c = 0, then runs 0x00412708 (a regs routine that leaves
// ebp and the flags of its last shift). Every register as 0x00412708 leaves it is the result.
// FUNCTION: GP4 0x0051fc67
void __cdecl FUN_0051fc67(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    gp4::call_regs(0x0041276c, c);
    GP4_GLOBAL(uint32_t, 0x006f2e90) = c.eax;
    GP4_GLOBAL(uint16_t, 0x0066d86c) = 0;
    GP4_GLOBAL(uint32_t, 0x0066d814) = 0;
    GP4_GLOBAL(uint16_t, 0x0066d86e) = 0;
    GP4_GLOBAL(uint32_t, 0x0066d85c) = 0;
    gp4::call_regs(0x00412708, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0051fc67, FUN_0051fc67, "regs() -> (eax, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00412708")
