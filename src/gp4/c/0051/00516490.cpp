#include <gp4/gp4.h>
#include <gp4/flags.h>

// `pushal ; ecx = 6 ; bt word [0x007ad892], cx ; ... ; popal`: stores bit 6 of the word at 0x007ad892
// as 0 / 1 in the dword 0x00a6987c. Every GPR is restored; `bt` replaces only CF (the other flags keep
// their entry values).
// FUNCTION: GP4 0x00516490
void __cdecl FUN_00516490(gp4::Regs* r) {
    const uint32_t bit = (static_cast<uint32_t>(GP4_GLOBAL(uint16_t, 0x007ad892)) >> 6) & 1u;
    GP4_GLOBAL(uint32_t, 0x00a6987c) = bit;
    r->eflags = (r->eflags & ~static_cast<uint32_t>(gp4::flags::CF)) | bit;
}
GP4_IMPL(0x00516490, FUN_00516490, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32")
