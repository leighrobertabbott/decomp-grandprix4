#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x00533e11
// Loads ebx/eax/ebp/ecx from the (unaligned) dwords 0x008280dd/e1/e5/e9, then calls the empty
// routine 0x00531b31 inside a pushal/popal pair (the registers are adjusted by +2/+2/-2/-2 first,
// but popal restores the loaded values) and the empty routine 0x00531b37 with edx = 0x66.
// Leaves ebx/eax/ebp/ecx = the loaded values, edx = 0x66; the flags are those of the last
// flag writer before the popal, `sub ecx, 2` (popal does not restore flags).
// The callees are empty, so edx = 0x531913 for the first call is not visible.
void __cdecl FUN_00533e11(gp4::Regs* r) {
    const uint32_t ebx = GP4_GLOBAL(uint32_t, 0x008280dd);
    const uint32_t eax = GP4_GLOBAL(uint32_t, 0x008280e1);
    const uint32_t ebp = GP4_GLOBAL(uint32_t, 0x008280e5);
    const uint32_t ecx = GP4_GLOBAL(uint32_t, 0x008280e9);

    gp4::cdecl_<0x00531b31, void>();
    gp4::cdecl_<0x00531b37, void>();

    r->ebx = ebx;
    r->eax = eax;
    r->ebp = ebp;
    r->ecx = ecx;
    r->edx = 0x66u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::sub32(ecx, 2u));
}
GP4_IMPL(0x00533e11, FUN_00533e11, "regs() -> (eax, ebx, ecx, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32")
