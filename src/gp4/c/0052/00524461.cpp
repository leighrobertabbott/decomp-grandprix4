#include <gp4/gp4.h>
#include <gp4/flags.h>

// `test dword [0x6f2bfc], 0xff`: when the low byte is non-zero those are the flags; when it is
// zero the routine continues with `or eax, eax` and returns the flags of that instruction.
// FUNCTION: GP4 0x00524461
void __cdecl FUN_00524461(gp4::Regs* r) {
    const uint32_t masked = GP4_GLOBAL(uint32_t, 0x006f2bfc) & 0xffu;
    uint32_t flags = gp4::flags::logic32(masked);
    if (masked == 0u) {
        flags = gp4::flags::logic32(r->eax);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00524461, FUN_00524461, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
