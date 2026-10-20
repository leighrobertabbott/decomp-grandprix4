#include <gp4/gp4.h>

// `call 0x0054d1ad ; call 0x00546189 ; ret`: esi = the text id becomes the string pointer from the text
// table (0x0054d1ad), then 0x00546189 draws the string at the cursor pair 0x831560/64 (through
// 0x5083c0 or 0x54654a depending on the byte at 0x7ab53c). Registers and flags are whatever the second
// call leaves.
// FUNCTION: GP4 0x0054615d
void __cdecl FUN_0054615d(gp4::Regs* r) {
    gp4::call_regs(0x0054d1ad, *r);
    gp4::call_regs(0x00546189, *r);
}
GP4_IMPL(0x0054615d, FUN_0054615d, "regs(esi:u32[0..7]) -> (eax, ecx, edx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cursor_00545f09+surf_0054654a+script_00545efe")
