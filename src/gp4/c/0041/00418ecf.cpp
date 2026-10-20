#include <gp4/gp4.h>

// Resets the 0xa8-byte entry found by 0x418901 (selected by the byte at esi+0x202): clears the
// dwords at +0 and +0xa4 and sets the dword at +4 to 1. pushal/popal restore every register, so
// only the flags of the final "or eax, 1" survive: CF = OF = AF = 0, result 1 gives PF = ZF = SF = 0.
// FUNCTION: GP4 0x00418ecf
void __cdecl FUN_00418ecf(gp4::Regs* r) {
    gp4::Regs callee = *r;
    callee.st_in = 0;
    callee.st_out = 0;
    gp4::call_regs(0x00418901, callee);

    uint8_t* entry = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(callee.eax));
    GP4_FIELD(uint32_t, entry, 0x00) = 0u;
    GP4_FIELD(uint32_t, entry, 0xa4) = 0u;
    uint32_t flag = 0u;
    flag |= 1u;
    GP4_FIELD(uint32_t, entry, 0x04) = flag;

    r->eflags = r->eflags & ~0x8d5u;
}
GP4_IMPL(0x00418ecf, FUN_00418ecf, "regs(esi:ptr[0x20a]:bytes) -> (cf, pf, af, zf, sf, of)")
