#include <gp4/gp4.h>
#include <gp4/flags.h>

// Range test reported through the carry flag: eax >= edx (signed) returns with `stc` (flags of
// `cmp eax, edx`, ecx untouched). Otherwise CF is cleared; when ecx >= edx (signed) ecx is
// clamped to edx - 1 (flags of that `dec ecx`), else the flags are those of `cmp ecx, edx`.
// FUNCTION: GP4 0x005484db
void __cdecl FUN_005484db(gp4::Regs* r) {
    const int32_t eax = static_cast<int32_t>(r->eax);
    const int32_t ecx = static_cast<int32_t>(r->ecx);
    const int32_t edx = static_cast<int32_t>(r->edx);
    uint32_t flags;
    if (eax >= edx) {
        flags = gp4::flags::sub32(r->eax, r->edx) | gp4::flags::CF;     // stc
    } else {
        flags = gp4::flags::sub32(r->ecx, r->edx);
        if (ecx >= edx) {
            r->ecx = r->edx - 1u;
            flags = gp4::flags::dec32(r->edx, flags);
        }
        flags &= ~gp4::flags::CF;                                       // clc
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x005484db, FUN_005484db, "regs(eax:u32, ecx:u32, edx:u32) -> (ecx, cf, pf, af, zf, sf, of)")
