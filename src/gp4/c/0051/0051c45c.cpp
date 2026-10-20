#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051c45c
// Smallest q with q + (q >> 1) == n for n = DAT_0096a374, starting from q = 2n / 3: first tries
// q = floor(2n / 3), then q + 1 (no further fallback; the second miss simply falls out).
// Returns q in eax. edx and ebp are saved and restored. The flags are those of the last
// `cmp edx, [0x96a374]` (the first one when it matched, otherwise the second one).
// `cdq; div 3` faults for n >= 0x40000000 (the doubled value has its sign bit set).
void __cdecl FUN_0051c45c(gp4::Regs* r) {
    const uint32_t n = GP4_GLOBAL(uint32_t, 0x0096a374);
    uint32_t q = (n << 1) / 3u;
    uint32_t t = q + (q >> 1);
    uint32_t f = gp4::flags::sub32(t, n);
    if (t != n) {
        q += 1u;
        t = q + (q >> 1);
        f = gp4::flags::sub32(t, GP4_GLOBAL(uint32_t, 0x0096a374));
    }
    r->eax = q;
    r->eflags = gp4::flags::merge(r->eflags, f);
}
GP4_IMPL(0x0051c45c, FUN_0051c45c, "regs() -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0051c45c")
