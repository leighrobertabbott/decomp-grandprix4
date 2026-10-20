#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the 4 bytes at 0x0066d8c8 to 0x007c2e9c, one byte at a time (movsb loop). pushal/popal
// restore every register; the flags are those of the final `cmp edx, edi` (edx == edi == the end
// address 0x007c2ea0, so ZF = PF = 1).
// FUNCTION: GP4 0x005204a9
void __cdecl FUN_005204a9(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(const uint8_t, 0x0066d8c8);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007c2e9c);
    uint8_t* const end = GP4_ARRAY(uint8_t, 0x007c2ea0);
    do {
        *dst = *src;
        ++src;
        ++dst;
    } while (dst != end);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x007c2ea0u, 0x007c2ea0u));
}
GP4_IMPL(0x005204a9, FUN_005204a9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
