#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: stores the signed byte at +0x72 of the selected car record [0x00679d30], plus one,
// to 0x00a6987c. popal restores every register; the flags stay those of the `add eax, 1`.
// FUNCTION: GP4 0x00516652
void __cdecl FUN_00516652(gp4::Regs* r) {
    const uint32_t car = GP4_GLOBAL(uint32_t, 0x00679d30);
    const uint32_t value = static_cast<uint32_t>(static_cast<int32_t>(GP4_FIELD(int8_t, car, 0x72)));
    GP4_GLOBAL(uint32_t, 0x00a6987c) = value + 1u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(value, 1u));
}
GP4_IMPL(0x00516652, FUN_00516652, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carbg_0066fef0+state_0044334d")
