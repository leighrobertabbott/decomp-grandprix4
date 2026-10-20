#include <gp4/gp4.h>
#include <gp4/flags.h>

// Makes two byte tables strictly increasing. Each table has a count byte (0x0081d1fa and
// 0x0081d223) and its entries start two bytes after the first table's count (0x0081d1fc) and one
// byte after the second's (0x0081d224). For a count of at least 2, every entry after the first is
// raised to previous+1 when it is not already greater than the previous one (unsigned bytes).
// eax/ebx/ecx are saved and restored by the original; the only register effect is the flags: those
// of "cmp ecx,2" when the second table has fewer than two entries, otherwise the CF of the last
// "cmp al,[ebx]" with the ZF/PF left by the final "dec ecx" (inc/dec keep CF).
//
// Runs the compare-and-raise loop over `count - 1` entries starting at p (p[-1] is the first
// predecessor) and returns the arithmetic flags the original leaves behind.
static inline uint32_t raiseRun(uint8_t* p, uint32_t count) {
    uint32_t remaining = count - 1u;
    uint32_t carry = 0u;
    do {
        const uint8_t previous = p[-1];
        const uint8_t current = *p;
        carry = gp4::flags::sub8(previous, current) & gp4::flags::CF;
        if (!(previous < current)) {
            *p = (uint8_t)(previous + 1u);
        }
        ++p;
        --remaining;
    } while (remaining != 0u);
    return carry | gp4::flags::ZF | gp4::flags::PF;
}

// FUNCTION: GP4 0x005295e0
void __cdecl FUN_005295e0(gp4::Regs* r) {
    uint32_t count = GP4_GLOBAL(uint8_t, 0x0081d1fa);
    if (count >= 2u) {
        raiseRun(GP4_ARRAY(uint8_t, 0x0081d1fd), count);
    }
    count = GP4_GLOBAL(uint8_t, 0x0081d223);
    uint32_t arith;
    if (count < 2u) {
        arith = gp4::flags::sub32(count, 2u);
    } else {
        arith = raiseRun(GP4_ARRAY(uint8_t, 0x0081d225), count);
    }
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x005295e0, FUN_005295e0, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=lists_005295e0")
