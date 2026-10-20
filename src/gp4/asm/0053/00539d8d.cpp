#include <gp4/gp4.h>
#include <gp4/flags.h>

// Orders two signed pairs: if ebx > ebp swap them, then if eax > ecx swap them (xchg leaves flags
// alone, so the flags are those of the `cmp eax, ecx`).
// FUNCTION: GP4 0x00539d8d
void __cdecl FUN_00539d8d(gp4::Regs* r) {
    uint32_t eax = r->eax;
    uint32_t ebx = r->ebx;
    uint32_t ecx = r->ecx;
    uint32_t ebp = r->ebp;

    const uint32_t flags = gp4::flags::sub32(eax, ecx);

    if (static_cast<int32_t>(ebx) > static_cast<int32_t>(ebp)) {
        const uint32_t t = ebx;
        ebx = ebp;
        ebp = t;
    }
    if (static_cast<int32_t>(eax) > static_cast<int32_t>(ecx)) {
        const uint32_t t = eax;
        eax = ecx;
        ecx = t;
    }

    r->eax = eax;
    r->ebx = ebx;
    r->ecx = ecx;
    r->ebp = ebp;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00539d8d, FUN_00539d8d, "regs(eax:u32, ebx:u32, ecx:u32, ebp:u32) -> (eax, ebx, ecx, ebp, cf, pf, af, zf, sf, of)")
