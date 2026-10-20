#include <gp4/gp4.h>

// `call 0x0054d1ad ; call 0x0054d211 ; ret`: esi = string-table entry [0x835846 + 4 * esi]
// (0x0054d1ad), then 0x0054d211 draws that string at the text cursor and advances the cursor x
// (a pushal/popal wrapper over 0x004fd820). The registers and flags each call leaves carry into the
// next; everything comes back from the second call.
// FUNCTION: GP4 0x0054d1a2
void __cdecl FUN_0054d1a2(gp4::Regs* r) {
    gp4::call_regs(0x0054d1ad, *r);
    gp4::call_regs(0x0054d211, *r);
}
GP4_IMPL(0x0054d1a2, FUN_0054d1a2, "regs(esi:u32[0..7]) -> (esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=script_0054d798+textout_0054d211")
