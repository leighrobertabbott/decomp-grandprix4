#include <gp4/gp4.h>
#include <gp4/flags.h>

// Looks the id in eax up in the slot table with 0x00541d06 (eax = slot address, or 0 when there is none);
// a hit becomes the slot's index through 0x00541d44, a miss gives -1. The flags are those of the
// `or eax, eax` on a miss, otherwise the callee's.
// FUNCTION: GP4 0x00541d8d
void __cdecl FUN_00541d8d(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    gp4::call_regs(0x00541d06, c);
    c.eflags = gp4::flags::merge(c.eflags, gp4::flags::logic32(c.eax));
    if (c.eax != 0u) {
        gp4::call_regs(0x00541d44, c);
    } else {
        c.eax = 0xffffffffu;
    }
    *r = c;
}
GP4_IMPL(0x00541d8d, FUN_00541d8d, "regs(eax:u32{0,1,0x1f1f,0xffffffff,5}, ebx:u32, ecx:u32, edx:u32, esi:u32, edi:u32, ebp:u32) -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=slots_00541d5a")
