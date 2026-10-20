#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags-only test: `cmp word [0x6f3c80], 0` and, when that word is zero, `cmp word [0x8911f4], 5`;
// when that word equals 5 the final `or eax, eax` replaces the flags with the logic flags of eax.
// No register or memory is written.
// FUNCTION: GP4 0x00551ec1
void __cdecl FUN_00551ec1(gp4::Regs* r) {
    const uint16_t first = GP4_GLOBAL(uint16_t, 0x006f3c80);
    uint32_t flags = gp4::flags::sub16(first, 0u);
    if (first == 0u) {
        const uint16_t second = GP4_GLOBAL(uint16_t, 0x008911f4);
        flags = gp4::flags::sub16(second, 5u);
        if (second == 5u) {
            flags = gp4::flags::logic32(r->eax);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00551ec1, FUN_00551ec1, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
