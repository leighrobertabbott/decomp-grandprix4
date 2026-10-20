#include <gp4/gp4.h>
#include <gp4/flags.h>

// pushal / and eax,0xffff / mov [0x827559],eax / popal / ret: stores the low word of eax
// (zero-extended) into the dword at 0x00827559. popal restores every register but not the
// flags, which stay those of the `and`.
// FUNCTION: GP4 0x00531af4
void __cdecl FUN_00531af4(gp4::Regs* r) {
    const uint32_t value = r->eax & 0xffffu;
    GP4_GLOBAL(uint32_t, 0x00827559) = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x00531af4, FUN_00531af4, "regs(eax:u32) -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
