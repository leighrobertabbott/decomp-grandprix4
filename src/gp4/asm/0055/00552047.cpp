#include <gp4/gp4.h>
#include <gp4/flags.h>

// Resets a small state block: fills the 0x80 dwords at 0x0096526c with 0xf0000000, stores
// (int)[0x00876f58] >= 0 to 0x0096320c and the constants 0x80 / 0 / 1 / 2 to 0x00963240 /
// 0x00963244 / 0x0096325c / 0x0096321c. pushal/popal keep every register (the direction flag is
// clear); the flags are those of `test [0x00876f58], 0xffffffff`.
// FUNCTION: GP4 0x00552047
void __cdecl FUN_00552047(gp4::Regs* r) {
    uint32_t* table = GP4_ARRAY(uint32_t, 0x0096526c);
    for (uint32_t i = 0; i < 0x80u; ++i) {
        table[i] = 0xf0000000u;
    }

    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00876f58);
    GP4_GLOBAL(uint32_t, 0x0096320c) = (static_cast<int32_t>(value) >= 0) ? 1u : 0u;
    GP4_GLOBAL(uint32_t, 0x00963240) = 0x80u;
    GP4_GLOBAL(uint32_t, 0x00963244) = 0u;
    GP4_GLOBAL(uint32_t, 0x0096325c) = 1u;
    GP4_GLOBAL(uint32_t, 0x0096321c) = 2u;

    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x00552047, FUN_00552047, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
