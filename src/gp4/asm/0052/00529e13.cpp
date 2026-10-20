#include <gp4/gp4.h>
#include <gp4/flags.h>

// Screen reset: when edx (the menu state) is 5, 6 or 7 it first runs 0x00529bf5, 0x00527f81, 0x0053cd0f and
// 0x00549d12; then always 0x005394b1 and 0x00529dea. Registers and flags thread through every callee, the
// flags of the last one (0x00529dea) are the result.
// FUNCTION: GP4 0x00529e13
void __cdecl FUN_00529e13(gp4::Regs* r) {
    const uint32_t state = r->edx;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    const uint32_t compared = (state == 5u) ? 5u : ((state == 6u) ? 6u : 7u);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::sub32(state, compared));

    if (state == 5u || state == 6u || state == 7u) {
        gp4::call_regs(0x00529bf5, c);
        gp4::call_regs(0x00527f81, c);
        gp4::call_regs(0x0053cd0f, c);
        gp4::call_regs(0x00549d12, c);
    }
    gp4::call_regs(0x005394b1, c);
    gp4::call_regs(0x00529dea, c);

    r->eax = c.eax;
    r->ebx = c.ebx;
    r->ecx = c.ecx;
    r->edx = c.edx;
    r->esi = c.esi;
    r->edi = c.edi;
    r->ebp = c.ebp;
    r->eflags = c.eflags;
}
GP4_IMPL(0x00529e13, FUN_00529e13, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32{5,6,0,1,2,3,4}, esi:u32{=0x843040,0x843066,0x84308c,0x8430b2,0x8430d8,0x8430fe,0x843128,0x843152,0x84317c,0x8431a6,0x8431d0,0x8431fa,0x843224}, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=sel_00529bf5+surfcopy_00549d55+menu_0084300a+flag4_005394b1+drv22_0052d402+cnt_0081d229+slots_005388ed+cells_00527eae+slot8_005388ba+anim_0052822a+list_00549db9+pool_00549db9; cases=40; wall=3600; emu=skip:win32 PeekMessageA")
