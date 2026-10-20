#include <gp4/gp4.h>
#include <gp4/flags.h>

// Compares the dword at 0x00b1bb40 with the dword at 0x00ae6b64 and returns only the flags of that
// `cmp`; eax is saved and restored around it.
// FUNCTION: GP4 0x00541a0a
void __cdecl FUN_00541a0a(gp4::Regs* r) {
    const uint32_t a = GP4_GLOBAL(uint32_t, 0x00b1bb40);
    const uint32_t b = GP4_GLOBAL(uint32_t, 0x00ae6b64);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(a, b));
}
GP4_IMPL(0x00541a0a, FUN_00541a0a, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
