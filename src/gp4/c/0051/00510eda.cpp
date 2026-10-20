#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal wrapper: sets bit 5 of byte +0x83 of the car record pointed to by [0x00679d30]. popal restores
// every register; the flags are those of the `or byte [esi+0x83], 0x20`.
// FUNCTION: GP4 0x00510eda
void __cdecl FUN_00510eda(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00679d30)));
    const uint8_t value = static_cast<uint8_t>(GP4_FIELD(uint8_t, car, 0x83) | 0x20u);
    GP4_FIELD(uint8_t, car, 0x83) = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(value));
}
GP4_IMPL(0x00510eda, FUN_00510eda, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040f5f3+order_00418f8f")
