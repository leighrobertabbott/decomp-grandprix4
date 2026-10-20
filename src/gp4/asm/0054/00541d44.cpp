#include <gp4/gp4.h>
#include <gp4/flags.h>

// Converts a slot address of the 0x1c-byte slot table at 0x00b1b8d8 (eax) into its slot index:
// eax = (eax - 0x00b1b8d8) / 0x1c (unsigned). ebx and edx are preserved; the flags are those of the
// `sub eax, 0x00b1b8d8` (DIV leaves them untouched).
// FUNCTION: GP4 0x00541d44
void __cdecl FUN_00541d44(gp4::Regs* r) {
    const uint32_t address = r->eax;
    const uint32_t offset = address - 0x00b1b8d8u;

    r->eax = offset / 0x1cu;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(address, 0x00b1b8d8u));
}
GP4_IMPL(0x00541d44, FUN_00541d44, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
