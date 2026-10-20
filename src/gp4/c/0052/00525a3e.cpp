#include <gp4/gp4.h>
#include <gp4/flags.h>

// Returns in eax 0 when bit 0x100 of the state dword [0x006f3adc] is set, else 1 when bit 0x200 is set;
// eax is unchanged when neither is. The flags are those of the last `test dword [0x6f3adc], mask`
// executed.
// FUNCTION: GP4 0x00525a3e
void __cdecl FUN_00525a3e(gp4::Regs* r) {
    const uint32_t state = GP4_GLOBAL(uint32_t, 0x006f3adc);
    uint32_t arith;
    if ((state & 0x100u) != 0u) {
        arith = gp4::flags::logic32(state & 0x100u);
        r->eax = 0u;
    } else {
        arith = gp4::flags::logic32(state & 0x200u);
        if ((state & 0x200u) != 0u) {
            r->eax = 1u;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00525a3e, FUN_00525a3e, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
