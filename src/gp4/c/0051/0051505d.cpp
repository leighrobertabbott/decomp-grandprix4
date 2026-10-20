#include <gp4/gp4.h>
#include <gp4/flags.h>

// Copies the byte at 0x007c2e96, zero-extended, to the dword at 0x00866090. eax keeps the value and the
// flags are those of the `and eax, 0xff`.
// FUNCTION: GP4 0x0051505d
void __cdecl FUN_0051505d(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint8_t, 0x007c2e96);
    GP4_GLOBAL(uint32_t, 0x00866090) = value;
    r->eax = value;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
}
GP4_IMPL(0x0051505d, FUN_0051505d, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
