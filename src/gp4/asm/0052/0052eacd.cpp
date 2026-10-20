#include <gp4/gp4.h>

// Menu value handler wrapper: when the command in edx is 2 it first sets the byte flag at
// 0x82fd5f to 0xff, then runs the handler 0x538f91 with the same registers (`call; ret`), whose
// results (eax, edx = 0, flags) are returned as they are.
// FUNCTION: GP4 0x0052eacd
void __cdecl FUN_0052eacd(gp4::Regs* r) {
    if (r->edx == 2u) {
        GP4_GLOBAL(uint8_t, 0x0082fd5f) = 0xffu;
    }
    gp4::call_regs(0x00538f91, *r);
}
GP4_IMPL(0x0052eacd, FUN_0052eacd, "regs(eax:u32, edx:u32{1,2,5,6,0,1,2}, ebx:ptr[8]:u8{0,0,0,1,2}, esi:ptr[0x40]:u8{0,0,0,1,1,2,3,0x80}) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
