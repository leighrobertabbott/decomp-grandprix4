#include <gp4/gp4.h>

// Frame item wrapper: the record at esi is queued by 0x0054e8e7 with edx = 0xd3 as the base value when the pointer
// [0x00b1d3e8] is nonzero, [0x007aafd3] is 0 and [0x007ab184] is nonzero, and with 0xd6 otherwise. Every register and
// the flags come back from the callee.
// FUNCTION: GP4 0x0054e81f
void __cdecl FUN_0054e81f(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    uint32_t base = 0xd6u;
    if (GP4_GLOBAL(uint32_t, 0x00b1d3e8) != 0u && GP4_GLOBAL(uint8_t, 0x007aafd3) == 0u &&
        GP4_GLOBAL(uint8_t, 0x007ab184) != 0u) {
        base = 0xd3u;
    }
    c.edx = base;
    gp4::call_regs(0x0054e8e7, c);
    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x0054e81f, FUN_0054e81f, "regs(esi:ptr[0x40]:layout=item_0054e8e7, eax:u32, ebx:u32, ecx:u32, ebp:u32) -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=list_00549db9+item_0054e8e7")
