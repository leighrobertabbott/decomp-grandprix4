#include <gp4/gp4.h>
#include <gp4/flags.h>

// The unaligned dword 0x00833cc9 grows by [0x00833f58] + [0x00833f70], then the dword at 0x00833ccd
// is copied to 0x00833cc5. eax ends as that copied value; the flags are those of
// `add [0x833cc9], eax` (the mov does not touch them).
// FUNCTION: GP4 0x0054d2a4
void __cdecl FUN_0054d2a4(gp4::Regs* r) {
    const uint32_t step = GP4_GLOBAL(uint32_t, 0x00833f58) + GP4_GLOBAL(uint32_t, 0x00833f70);
    const uint32_t old = GP4_GLOBAL(uint32_t, 0x00833cc9);
    GP4_GLOBAL(uint32_t, 0x00833cc9) = old + step;
    const uint32_t next = GP4_GLOBAL(uint32_t, 0x00833ccd);
    GP4_GLOBAL(uint32_t, 0x00833cc5) = next;
    r->eax = next;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(old, step));
}
GP4_IMPL(0x0054d2a4, FUN_0054d2a4, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
