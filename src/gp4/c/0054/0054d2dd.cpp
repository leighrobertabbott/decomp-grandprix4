#include <gp4/gp4.h>
#include <gp4/flags.h>

// Runs the empty routine 0x0054d6f2 (which leaves eax alone), negates eax, adds it to the (unaligned)
// dword at 0x00833cc5 and sets the byte 0x00833f74 to 0xff. eax comes back negated; the flags are those
// of the `add` into memory.
// FUNCTION: GP4 0x0054d2dd
void __cdecl FUN_0054d2dd(gp4::Regs* r) {
    gp4::cdecl_<0x0054d6f2, void>();
    const uint32_t negated = 0u - r->eax;
    const uint32_t before = GP4_GLOBAL(uint32_t, 0x00833cc5);
    GP4_GLOBAL(uint32_t, 0x00833cc5) = before + negated;
    GP4_GLOBAL(uint8_t, 0x00833f74) = 0xffu;
    r->eax = negated;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(before, negated));
}
GP4_IMPL(0x0054d2dd, FUN_0054d2dd, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
