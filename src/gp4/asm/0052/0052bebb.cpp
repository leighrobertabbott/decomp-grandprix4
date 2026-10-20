#include <gp4/gp4.h>
#include <gp4/flags.h>

// Bitwise combiner on al and dl (all 8-bit): with a = al and d = dl,
//   x = (d | a) ^ a                 (the bits of d not in a)
//   m = d & a
//   k = ((m >> 1) | (m << 1)) & a   (neighbours of the common bits, kept where a is set)
//   dl = x | k
// ebx is saved/restored; only dl and the flags (those of the last `or dl, bl`) change.
// FUNCTION: GP4 0x0052bebb
void __cdecl FUN_0052bebb(gp4::Regs* r) {
    const uint8_t a = static_cast<uint8_t>(r->eax);
    const uint8_t d = static_cast<uint8_t>(r->edx);

    const uint8_t outside = static_cast<uint8_t>((d | a) ^ a);
    const uint8_t common = static_cast<uint8_t>(d & a);
    const uint8_t neighbours = static_cast<uint8_t>(((common >> 1) | static_cast<uint8_t>(common << 1)) & a);
    const uint8_t result = static_cast<uint8_t>(outside | neighbours);

    r->edx = (r->edx & 0xffffff00u) | result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic8(result));
}
GP4_IMPL(0x0052bebb, FUN_0052bebb, "regs(eax:u32, edx:u32) -> (edx, cf, pf, af, zf, sf, of)")
