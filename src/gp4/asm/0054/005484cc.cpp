#include <gp4/gp4.h>
#include <gp4/flags.h>

// Signed clamp that reports through the carry flag: when ecx <= edx (signed) the routine returns
// at once with `stc` (eax unchanged); otherwise eax = max(eax, edx) (signed) and `clc`. The other
// flags are those of the last compare (`cmp ecx, edx` resp. `cmp eax, edx`, taken before eax is
// raised to edx).
// FUNCTION: GP4 0x005484cc
void __cdecl FUN_005484cc(gp4::Regs* r) {
    const int32_t eax = static_cast<int32_t>(r->eax);
    const int32_t ecx = static_cast<int32_t>(r->ecx);
    const int32_t edx = static_cast<int32_t>(r->edx);
    uint32_t flags;
    if (ecx <= edx) {
        flags = (gp4::flags::sub32(r->ecx, r->edx) & ~gp4::flags::CF) | gp4::flags::CF;   // stc
    } else {
        flags = gp4::flags::sub32(r->eax, r->edx) & ~gp4::flags::CF;                       // clc
        if (eax < edx) {
            r->eax = r->edx;
        }
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005484cc, FUN_005484cc, "regs(eax:u32, ecx:u32, edx:u32) -> (eax, cf, pf, af, zf, sf, of)")
