#include <gp4/gp4.h>
#include <gp4/flags.h>

// Loads ebx = DAT_00822244 >> 1 and then calls the empty routine at 0x0054d707. The flags are
// those of the `shr ebx, 1` (CF = bit shifted out, OF = original sign bit, AF modelled as 0).
// FUNCTION: GP4 0x0054d6f9
void __cdecl FUN_0054d6f9(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00822244);
    const uint32_t result = value >> 1;
    gp4::cdecl_<0x0054d707, void>();
    uint32_t flags = gp4::flags::szp(result, 32);
    if ((value & 1u) != 0u) {
        flags |= gp4::flags::CF;
    }
    if ((value & 0x80000000u) != 0u) {
        flags |= gp4::flags::OF;
    }
    r->ebx = result;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0054d6f9, FUN_0054d6f9, "regs() -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
