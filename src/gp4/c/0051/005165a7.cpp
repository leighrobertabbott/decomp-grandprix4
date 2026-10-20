#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: stores in 0x00a6987c whether bit 1 of byte +0x8d of the selected car record
// (pointer at 0x00679d30) is set. popal restores every register but not the flags, so the flags are
// those of `test byte [esi+0x8d], 2`.
// FUNCTION: GP4 0x005165a7
void __cdecl FUN_005165a7(gp4::Regs* r) {
    const uint8_t* car = (const uint8_t*)(uintptr_t)GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint32_t masked = car[0x8d] & 2u;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = (masked != 0u) ? 1u : 0u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(masked));
}
GP4_IMPL(0x005165a7, FUN_005165a7, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=ptr_00679d30")
