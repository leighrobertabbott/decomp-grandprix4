#include <gp4/gp4.h>

// FUNCTION: GP4 0x004168df
// Clears the two dwords at 0x006380d8 and 0x006383d8 (ebx + 0x300) and leaves
// ebx = 0x006380d8, eax = 0. The flags are those of "shl eax, 0x10" on zero.
void __cdecl FUN_004168df(gp4::Regs* r) {
    uint8_t* base = GP4_ARRAY(uint8_t, 0x006380d8);
    GP4_FIELD(uint32_t, base, 0x000) = 0u;
    GP4_FIELD(uint32_t, base, 0x300) = 0u;

    r->ebx = reinterpret_cast<uint32_t>(base);
    r->eax = 0u;
    // result 0: ZF | PF, CF = OF = AF = SF = 0
    r->eflags = (r->eflags & ~0x8d5u) | 0x44u;
}
GP4_IMPL(0x004168df, FUN_004168df, "regs() -> (eax, ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
