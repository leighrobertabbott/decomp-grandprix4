#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal; copies 0x16 bytes from 0x0066d8f4 to 0x007c31dc one at a time (`loop`); popal restores
// every general register. The flags are those of the last `inc ebx` (ebx = 0x0066d909 + 1): CF is
// kept, the others describe 0x0066d90a.
// FUNCTION: GP4 0x00522c96
void __cdecl FUN_00522c96(gp4::Regs* r) {
    const uint8_t* src = GP4_ARRAY(uint8_t, 0x0066d8f4);
    uint8_t* dst = GP4_ARRAY(uint8_t, 0x007c31dc);
    for (uint32_t i = 0; i != 0x16u; ++i) {
        dst[i] = src[i];
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::inc32(0x0066d909u, r->eflags));
}
GP4_IMPL(0x00522c96, FUN_00522c96, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=copy_0066d8f4")
