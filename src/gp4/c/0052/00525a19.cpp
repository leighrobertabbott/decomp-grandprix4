#include <gp4/gp4.h>
#include <gp4/flags.h>

// Returns in eax 0 when bit 0x10 of the state dword [0x006f3adc] is set, else 1 when bit 0x20 is set;
// eax is unchanged when neither is. The flags are those of the last `test dword [0x6f3adc], mask`
// executed.
// FUNCTION: GP4 0x00525a19
void __cdecl FUN_00525a19(gp4::Regs* r) {
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x006f3adc);
    uint32_t arith;
    if ((state & 0x10u) != 0u) {
        arith = gp4::flags::logic32(state & 0x10u);
        r->eax = 0u;
    } else {
        arith = gp4::flags::logic32(state & 0x20u);
        if ((state & 0x20u) != 0u) {
            r->eax = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00525a19, FUN_00525a19, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
