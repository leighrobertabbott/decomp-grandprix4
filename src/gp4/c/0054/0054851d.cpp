#include <gp4/gp4.h>
#include <gp4/flags.h>

// Counts the entries equal to 1 in the 22 per-car bytes at 0x007c2a74 and stores the count in the
// dword 0x008236a8. eax, ebx and ecx are saved and restored. The flags are those of the final
// `dec ecx` (result 0), whose CF comes from the last `cmp byte, 1` (borrow when that byte is 0).
// FUNCTION: GP4 0x0054851d
void __cdecl FUN_0054851d(gp4::Regs* r) {
    const uint8_t* cars = GP4_ARRAY(uint8_t, 0x007c2a74);
    uint32_t count = 0u;
    uint32_t arith = 0u;
    for (uint32_t i = 0; i != 0x16u; ++i) {
        arith = gp4::flags::sub8(cars[i], 1u);
        if (cars[i] == 1u) {
            ++count;
        }
    }
    GP4_GLOBAL(uint32_t, 0x008236a8) = count;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::dec32(1u, arith));
}
GP4_IMPL(0x0054851d, FUN_0054851d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=tabs_0043ced2")
