#include <gp4/gp4.h>

// Runs the menu item handler 0x0052d5f8 (edx = event, esi = item; esi is pushed/popped around it, so it comes
// back unchanged), then copies the dword [0x0081e4c0] to [0x00823690] (eax is pushed/popped around the copy).
// The other registers and the flags are what the handler leaves.
// FUNCTION: GP4 0x0052d5e4
void __cdecl FUN_0052d5e4(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.st_xin_mask = 0;
    c.st_xout_mask = 0;
    gp4::call_regs(0x0052d5f8, c);
    c.esi = r->esi;
    GP4_GLOBAL(uint32_t, 0x00823690) = GP4_GLOBAL(uint32_t, 0x0081e4c0);
    *r = c;
}
GP4_IMPL(0x0052d5e4, FUN_0052d5e4, "regs(edx:u32{=2,2,0,1,3}, esi:ptr[0x40]:layout=mitem1_0052d5f8) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=menu_0052d5f8")
