#include <gp4/gp4.h>
#include <gp4/flags.h>

// Sets up the mode-4 input handler: 0x00522435 refreshes the word 0x0066d8b4, the mode dword
// 0x0088ff30 is set to 4, the input dispatcher 0x005141b8 runs the mode's handler, the byte
// 0x0095404e is set to 0xff and 0x00436504 appends the 0x0c record to the opcode stream when its
// flag is set. The flags are those of the last call (0x00436504 rewrites all six).
// FUNCTION: GP4 0x005223fe
void __cdecl FUN_005223fe(gp4::Regs* r) {
    gp4::Regs c = *r;
    gp4::call_regs(0x00522435, c);
    GP4_GLOBAL(uint32_t, 0x0088ff30) = 4u;
    gp4::call_regs(0x005141b8, c);
    GP4_GLOBAL(uint8_t, 0x0095404e) = 0xffu;
    gp4::call_regs(0x00436504, c);
    r->eflags = gp4::flags::merge(r->eflags, c.eflags);
}
GP4_IMPL(0x005223fe, FUN_005223fe, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=player_0040f8d9+disp_005141b8+stream_0062d1a0+h4_005223fe")
