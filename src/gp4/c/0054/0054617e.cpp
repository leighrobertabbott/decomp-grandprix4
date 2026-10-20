#include <gp4/gp4.h>

// `call 0x0054d1ad ; call 0x00546219 ; ret`: esi = string-table entry [0x835846 + 4 * esi]
// (0x0054d1ad), then the text drawer 0x00546219 draws that string at ([0x831560], [0x831564]):
// 0x005084e0 (GDI TextOutA) when the byte [0x7ab53c] is zero, else the vertical writer 0x005465f7.
// The registers and flags each call leaves carry into the next; everything comes back from the
// second call.
// FUNCTION: GP4 0x0054617e
void __cdecl FUN_0054617e(gp4::Regs* r) {
    gp4::call_regs(0x0054d1ad, *r);
    gp4::call_regs(0x00546219, *r);
}
GP4_IMPL(0x0054617e, FUN_0054617e, "regs(esi:u32[0..7]) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+surf_0054654a; emu=skip:win32 TextOutA")
