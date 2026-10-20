#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: stores (byte +0x7c of the selected car record [0x00679d30] & 0x3f) - 1 to
// 0x00a6987c. popal restores every GPR; the flags are those of the final `sub eax, 1`.
// FUNCTION: GP4 0x005164da
void __cdecl FUN_005164da(gp4::Regs* r) {
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint32_t value = static_cast<uint32_t>(GP4_FIELD(uint8_t, car, 0x7c)) & 0x3fu;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value - 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(value, 1u));
}
GP4_IMPL(0x005164da, FUN_005164da, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_0044334d")
