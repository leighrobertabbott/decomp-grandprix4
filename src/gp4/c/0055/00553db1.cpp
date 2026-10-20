#include <gp4/gp4.h>

// Loads esi with the pointer stored at 0x008914e8 (the loaded block that the unpacker expects) and
// tail-calls the block unpacker 0x00553dbd, whose register results (eax, ebx, ecx, esi, edi, flags)
// are left in place for the caller.
// FUNCTION: GP4 0x00553db1
void __cdecl FUN_00553db1(gp4::Regs* r) {
    const uint32_t block = GP4_GLOBAL(uint32_t, 0x008914e8);
    r->esi = block;
    gp4::call_regs(0x00553dbd, *r);
}
GP4_IMPL(0x00553db1, FUN_00553db1, "regs() -> (eax, ebx, ecx, esi, edi, cf, pf, af, zf, sf, of); globals=layout=ptr_008914e8")
