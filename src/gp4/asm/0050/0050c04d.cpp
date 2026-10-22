#include <gp4/gp4.h>
#include <gp4/flags.h>

// Address of the (7 - (eax & 7))th 0x2000-byte block starting at the pointer
// held in 0x0073d258; eax = (((eax & 7) ^ 7) << 13) + [0x73d258].
// FUNCTION: GP4 0x0050c04d
void __cdecl FUN_0050c04d(gp4::Regs* r) {
    const uint32_t index = r->eax;
    const uint32_t base = GP4_GLOBAL(uint32_t, 0x0073d258);
    const uint32_t offset = (((index & 7u) ^ 7u) << 13);
    r->eax = offset + base;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(offset, base));
}
GP4_IMPL(0x0050c04d, FUN_0050c04d, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
