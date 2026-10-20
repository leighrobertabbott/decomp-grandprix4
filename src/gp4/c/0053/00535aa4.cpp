#include <gp4/gp4.h>
#include <gp4/flags.h>

// eax = [[0x00826d11] + 0x40] + [0x007a93e0] - 2. The flags are those of the final `sub eax, 2`.
// FUNCTION: GP4 0x00535aa4
void __cdecl FUN_00535aa4(gp4::Regs* r) {
    const uint32_t block = GP4_GLOBAL(uint32_t, 0x00826d11);
    const uint32_t first = GP4_FIELD(uint32_t, block, 0x40);
    const uint32_t sum = first + GP4_GLOBAL(uint32_t, 0x007a93e0);
    r->eax = sum - 2u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(sum, 2u));
}
GP4_IMPL(0x00535aa4, FUN_00535aa4, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00826d11")
