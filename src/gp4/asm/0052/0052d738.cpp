#include <gp4/gp4.h>
#include <gp4/flags.h>

// Tests bit (al & 7) of byte (al >> 3) of the 32-byte bitmap at 0x008245a0 (`and al, [ebx]` with
// al = 1 << (al & 7)). pushal/popal restore every register; the flags are those of the `and`.
// FUNCTION: GP4 0x0052d738
void __cdecl FUN_0052d738(gp4::Regs* r) {
    const uint32_t key = r->eax & 0xffu;
    const uint32_t bits = GP4_ARRAY(uint8_t, 0x008245a0)[key >> 3];
    const uint32_t result = (1u << (key & 7u)) & bits;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(result));
}
GP4_IMPL(0x0052d738, FUN_0052d738, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=keys_008245a0")
