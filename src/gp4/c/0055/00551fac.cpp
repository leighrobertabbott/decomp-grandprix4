#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags-only test: the word at 0x006f3c80 must be zero and the word at 0x008911f4 must be 0xc1; only
// when both hold does the routine reach `or eax, eax`. The flags are those of the last of
// `cmp word [0x6f3c80], 0`, `cmp word [0x8911f4], 0xc1` and `or eax, eax` executed.
// FUNCTION: GP4 0x00551fac
void __cdecl FUN_00551fac(gp4::Regs* r) {
    const uint16_t first = GP4_GLOBAL(uint16_t, 0x006f3c80);
    uint32_t flags = gp4::flags::sub16(first, 0u);
    if (first == 0u) {
        const uint16_t second = GP4_GLOBAL(uint16_t, 0x008911f4);
        flags = gp4::flags::sub16(second, 0xc1u);
        if (second == 0xc1u) {
            flags = gp4::flags::logic32(r->eax);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00551fac, FUN_00551fac, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
