#include <gp4/gp4.h>
#include <gp4/flags.h>

// Percentage to Q14: eax is clamped to 0..100 (signed) and becomes (eax << 14) / 100 with the
// remainder in edx (`xor edx, edx; mov ecx, 100; div ecx`); ecx = 100 is left behind. `div`
// leaves the flags of the preceding `xor edx, edx` (ZF = PF = 1, the others clear).
// FUNCTION: GP4 0x005183e6
void __cdecl FUN_005183e6(gp4::Regs* r) {
    int32_t percent = static_cast<int32_t>(r->eax);
    if (percent > 100) {
        percent = 100;
    }
    if (percent < 0) {
        percent = 0;
    }
    const uint32_t scaled = static_cast<uint32_t>(percent) << 14;
    r->eax = scaled / 100u;
    r->edx = scaled % 100u;
    r->ecx = 100u;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::logic32(0u));
}
GP4_IMPL(0x005183e6, FUN_005183e6, "regs(eax:i32[-50..150]) -> (eax, ecx, edx, cf, pf, af, zf, sf, of)")
