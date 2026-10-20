#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts how many of the 22 per-car state bytes at 0x007c2a74 equal 1 and returns the count in eax.
// ebx and ecx are saved and restored. The flags are those of the last `dec ecx` (1 -> 0), whose CF is
// the one left by the final `cmp byte [ebx], 1` (the inc/dec instructions in between keep it).
// FUNCTION: GP4 0x00548ec5
void __cdecl FUN_00548ec5(gp4::Regs* r) {
    const uint8_t* state = GP4_ARRAY(uint8_t, 0x007c2a74);
    uint32_t count = 0u;
    uint32_t arith = 0u;
    for (uint32_t i = 0u; i < 0x16u; ++i) {
        arith = gp4::flags::sub8(state[i], 1u);
        if (state[i] == 1u) {
            count += 1u;
        }
    }
    r->eax = count;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, arith));
}
GP4_IMPL(0x00548ec5, FUN_00548ec5, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=layout=tabs_0043ced2")
