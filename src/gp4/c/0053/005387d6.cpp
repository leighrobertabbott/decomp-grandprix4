#include <gp4/gp4.h>

// Tail wrapper: `call 0x005387dc ; ret`. The callee allocates eax bytes at the used end of the block pool and
// answers in eax (0 when nothing fits) and the flags; ebx, ecx, esi and edi are preserved by its own frame.
// FUNCTION: GP4 0x005387d6
void __cdecl FUN_005387d6(gp4::Regs* r) {
    gp4::call_regs(0x005387dc, *r);
}
GP4_IMPL(0x005387d6, FUN_005387d6, "regs(eax:u32{=0,1,2,3,4,5,8,0xc,0x10,0xfffffff0,0xffffffff}, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=layout=pool_005387dc+serial_005387dc")
