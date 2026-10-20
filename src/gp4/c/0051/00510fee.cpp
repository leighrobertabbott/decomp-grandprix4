#include <gp4/gp4.h>
#include <gp4/flags.h>

static __declspec(noinline) uint32_t spanBytes(uint32_t end, uint32_t begin) {
    return end - begin;
}

// pushal / popal around a clear of the 0x42220 bytes 0x006f6dc4..0x00738fe3 (`rep stosb` of 0, direction
// flag clear). Every register is restored; the flags are those of `sub ecx, edi` (the stos does not
// touch them and popal does not restore them).
// FUNCTION: GP4 0x00510fee
void __cdecl FUN_00510fee(gp4::Regs* r) {
    uint8_t* block = GP4_ARRAY(uint8_t, 0x006f6dc4);
    const uint32_t count = spanBytes(0x00738fe4u, 0x006f6dc4u);
    for (uint32_t i = 0; i != count; ++i) {
        block[i] = 0;
    }
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(0x00738fe4u, 0x006f6dc4u));
}
GP4_IMPL(0x00510fee, FUN_00510fee, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32; wall=3000")
