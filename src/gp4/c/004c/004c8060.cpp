#include <gp4/gp4.h>
#include <gp4/flags.h>

// al = (int)[0x00a71864] >= 3 (cmp dword [g], 3 ; setge al). Only al is written, so
// eax[31:8] stay the caller's; the flags are those of the cmp.
// FUNCTION: GP4 0x004c8060
void __cdecl FUN_004c8060(gp4::Regs* r) {
    const int level = GP4_GLOBAL(int, 0x00a71864);
    const uint32_t flags = gp4::flags::sub32((uint32_t)level, 3u);
    r->eax = (r->eax & 0xffffff00u) | (level >= 3 ? 1u : 0u);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x004c8060, FUN_004c8060, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
