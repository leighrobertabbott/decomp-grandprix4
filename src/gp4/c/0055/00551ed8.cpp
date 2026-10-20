#include <gp4/gp4.h>
#include <gp4/flags.h>

// Flags-only test: `cmp word [0x6f3c80], 0` and, when that word is zero, `cmp dword [0x874fb4], 0`;
// when the dword is nonzero the final `or eax, eax` replaces the flags with the logic flags of eax.
// No register or memory is written.
// FUNCTION: GP4 0x00551ed8
void __cdecl FUN_00551ed8(gp4::Regs* r) {
    const uint16_t word = GP4_GLOBAL(uint16_t, 0x006f3c80);
    uint32_t flags = gp4::flags::sub16(word, 0u);
    if (word == 0u) {
        const uint32_t dword = GP4_GLOBAL(uint32_t, 0x00874fb4);
        flags = gp4::flags::sub32(dword, 0u);
        if (dword != 0u) {
            flags = gp4::flags::logic32(r->eax);
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00551ed8, FUN_00551ed8, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
