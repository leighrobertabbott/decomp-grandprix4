#include <gp4/gp4.h>

// Tail wrapper: `call 0x0054fbfb ; ret`. The callee redraws the option list from the entry table at
// 0x0083e55e (count in the byte 0x0083e55c) and ends in 0x00549d12; registers and flags come straight
// back from it.
// FUNCTION: GP4 0x0052d6d0
void __cdecl FUN_0052d6d0(gp4::Regs* r) {
    gp4::call_regs(0x0054fbfb, *r);
}
GP4_IMPL(0x0052d6d0, FUN_0052d6d0, "regs(eax:u32, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); cases=40; globals=fuzz:i32+layout=surfcopy_00549d55; wall=3600")
