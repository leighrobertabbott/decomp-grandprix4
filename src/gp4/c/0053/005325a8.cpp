#include <gp4/gp4.h>
#include <gp4/flags.h>

// `call 0x005357d8 ; mov eax, 0 ; adc eax, 0 ; ret`: runs the calibration-file load (0x005357d8,
// which reports failure in the carry flag) and returns the carry in eax. The flags are those of
// `adc eax, 0` with eax = 0, i.e. of the result 0 or 1; every other register comes from the callee.
// FUNCTION: GP4 0x005325a8
void __cdecl FUN_005325a8(gp4::Regs* r) {
    gp4::call_regs(0x005357d8, *r);
    const uint32_t carry = r->eflags & 1u;
    r->eax = carry;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::add32(0u, carry));
}
GP4_IMPL(0x005325a8, FUN_005325a8, "regs() -> (eax, ebx, ecx, edx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32; emu=skip:win32 PeekMessageA")
