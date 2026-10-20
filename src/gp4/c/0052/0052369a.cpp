#include <gp4/gp4.h>
#include <gp4/flags.h>

// When the dword 0x00a53ad0 is nonzero, stores 2 into 0x00645950, loads it back into eax and runs
// 0x0051006c (restore of saved slot 2). Otherwise only the flags of the `test` change.
// FUNCTION: GP4 0x0052369a
void __cdecl FUN_0052369a(gp4::Regs* r) {
    const uint32_t value = GP4_GLOBAL(uint32_t, 0x00a53ad0);
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(value));
    if (value != 0u) {
        GP4_GLOBAL(uint32_t, 0x00645950) = 2u;
        r->eax = GP4_GLOBAL(uint32_t, 0x00645950);
        gp4::call_regs(0x0051006c, *r);
    }
}
GP4_IMPL(0x0052369a, FUN_0052369a, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=save_00510309")
