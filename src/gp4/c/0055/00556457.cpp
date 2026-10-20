#include <gp4/gp4.h>
#include <gp4/flags.h>

// Per-record step of the ring walk 0x0055652b (edi = the 0x160-byte record). Two independent
// "latch" updates on the bit-set dwords of the record: while ebp is not -1 it is set to -1 and the
// bits 0x200 / 0x1000000 are moved by 0x00556414; when ebp is already -1 the bits 0x200 are
// cleared by 0x00556442. Then the same with ecx and the bits 0x100 / 0x800000. Leaves ebp = ecx = -1,
// eax = the mask (0x100) or its complement, ebx = the last mask loaded (0x800000, else 0x1000000
// when only the first branch loaded one, else the input) and the flags of the last callee.
// FUNCTION: GP4 0x00556457
void __cdecl FUN_00556457(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;

    if (c.ebp == 0xffffffffu) {
        c.eax = 0x200u;
        gp4::call_regs(0x00556442, c);
    } else {
        c.ebp = 0xffffffffu;
        c.eax = 0x200u;
        c.ebx = 0x1000000u;
        gp4::call_regs(0x00556414, c);
    }

    if (c.ecx == 0xffffffffu) {
        c.eax = 0x100u;
        gp4::call_regs(0x00556442, c);
    } else {
        c.ecx = 0xffffffffu;
        c.eax = 0x100u;
        c.ebx = 0x800000u;
        gp4::call_regs(0x00556414, c);
    }

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->ebp = c.ebp;
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x00556457, FUN_00556457, "regs(ebx:u32, ebp:u32{0xffffffff,0xffffffff,0,1}, ecx:u32{0xffffffff,0xffffffff,0,1}, edi:ptr[0x48]:bytes) -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of)")
