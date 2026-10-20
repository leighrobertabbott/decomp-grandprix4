#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets the 256-byte map at 0x954060 to zero (`rep stosd` of 0x40 dwords), then sets to 0xff the map byte
// selected by each of the 0x20 bytes at 0x948ba0. Inside pushal/popal, so registers are preserved; the flags
// stay those of the final `cmp ecx, 0x20` (equal). Same body as 0x0051b6d4, which reads 0x948b80.
// FUNCTION: GP4 0x0051b701
void __cdecl FUN_0051b701(gp4::Regs* r) {
    uint8_t* map = GP4_ARRAY(uint8_t, 0x00954060);
    const uint8_t* indexes = GP4_ARRAY(const uint8_t, 0x00948ba0);

    for (uint32_t i = 0; i != 0x100u; ++i) {
        map[i] = 0;
    }
    for (uint32_t i = 0; i != 0x20u; ++i) {
        map[indexes[i]] = 0xff;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x20u, 0x20u));
}
GP4_IMPL(0x0051b701, FUN_0051b701, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
