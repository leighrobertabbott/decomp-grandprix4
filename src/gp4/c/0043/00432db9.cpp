#include <gp4/gp4.h>

// If byte 0x00622454 is not already 0xff, set it to 0xff and run the 0x00432cc2 per-record
// update (esi = record). When it is 0xff, only the flags of `cmp byte, 0xff` (equal) remain.
// FUNCTION: GP4 0x00432db9
void __cdecl FUN_00432db9(gp4::Regs* r) {
    if (GP4_GLOBAL(uint8_t, 0x00622454) == 0xffu) {
        // flags of cmp byte [..], 0xff with equal operands: ZF = PF = 1, the rest 0
        r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
        return;
    }
    GP4_GLOBAL(uint8_t, 0x00622454) = 0xffu;

    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x00432cc2, callee);
    r->eax = callee.eax;
    r->ebx = callee.ebx;
    r->ecx = callee.ecx;
    r->edx = callee.edx;
    r->esi = callee.esi;
    r->edi = callee.edi;
    r->ebp = callee.ebp;
    r->eflags = (r->eflags & ~0x8d5u) | (callee.eflags & 0x8d5u);
}
GP4_IMPL(0x00432db9, FUN_00432db9, "regs(esi:ptr[0x290]:bytes, edx:u32, ebp:u32) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_00432cc2+flag_00622454")
