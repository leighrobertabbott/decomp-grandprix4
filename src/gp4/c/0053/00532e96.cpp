#include <gp4/gp4.h>

// Tail wrapper: `call 0x00538440 ; ret`. Cached temp-file read, all register inputs and
// results pass straight through.
// FUNCTION: GP4 0x00532e96
void __cdecl FUN_00532e96(gp4::Regs* r) {
    gp4::call_regs(0x00538440, *r);
}
GP4_IMPL(0x00532e96, FUN_00532e96, "regs(eax:u32, ecx:u32[0..0x200], edx:u32{0xffffffff,0,1}, esi:u32[0..0x400], edi:ptr[0x200]:bytes) -> (eax, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=fuzz:i32")
