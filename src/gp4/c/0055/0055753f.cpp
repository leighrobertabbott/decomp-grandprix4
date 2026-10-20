#include <gp4/gp4.h>
#include <gp4/flags.h>

extern "C" void __cdecl __debugbreak(void);

// Appends one entry to a 0x80-slot pair of byte tables: both counters 0x00871c8c and 0x00871c80 are
// incremented; the new index (0x00871c80) must be below 0x80 and equal to the mirror counter
// 0x00871c8c, else the routine executes int3 (an assertion). The bytes 0x00875b46 and 0x00875c4a
// are copied to slot [index] of the tables at the same bases. pushal/popal: the registers are kept
// and the flags are those of the final `cmp edx, [0x871c8c]` (equal).
// FUNCTION: GP4 0x0055753f
void __cdecl FUN_0055753f(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x00871c8c) += 1u;
    GP4_GLOBAL(uint32_t, 0x00871c80) += 1u;
    if (GP4_GLOBAL(uint32_t, 0x00871c80) >= 0x80u) {
        __debugbreak();
        return;
    }
    const uint32_t index = GP4_GLOBAL(uint32_t, 0x00871c80);
    const uint32_t mirror = GP4_GLOBAL(uint32_t, 0x00871c8c);
    if (index != mirror) {
        __debugbreak();
        return;
    }
    GP4_ARRAY(uint8_t, 0x00875b46)[index] = GP4_GLOBAL(uint8_t, 0x00875b46);
    GP4_ARRAY(uint8_t, 0x00875c4a)[index] = GP4_GLOBAL(uint8_t, 0x00875c4a);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(index, mirror));
}
GP4_IMPL(0x0055753f, FUN_0055753f, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=stack_0055753f")
