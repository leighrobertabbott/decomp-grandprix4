#include <gp4/gp4.h>
#include <gp4/flags.h>

// Rewrites the dword 0x0096547c with itself, then loads the flag dword 0x00965484 into eax: when none of
// the bits 0x00600030 is set the bits 0x001800c3 are cleared (and eax, 0xffe7ff3c). The result is
// stored back and left in eax; the flags are those of the `test` or of the `and`.
// FUNCTION: GP4 0x00551685
void __cdecl FUN_00551685(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0096547c) = GP4_GLOBAL(uint32_t, 0x0096547c);
    uint32_t value = GP4_GLOBAL(uint32_t, 0x00965484);
    uint32_t arith = gp4::flags::logic32(value & 0x00600030u);
    if ((value & 0x00600030u) == 0u) {
        value &= 0xffe7ff3cu;
        arith = gp4::flags::logic32(value);
    }
    GP4_GLOBAL(uint32_t, 0x00965484) = value;
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x00551685, FUN_00551685, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
