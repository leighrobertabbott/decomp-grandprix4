#include <gp4/gp4.h>
#include <gp4/flags.h>

// Finds the position (counted from 'a' + 0) of the (eax + 1)-th set bit of the mask at 0x81e5d4:
// al starts at 0x60 and is incremented for every bit examined from bit 0 upward; the scan stops
// at the (eax + 1)-th set bit (al = 0x60 + bit index + 1) or, when the mask runs out first, al =
// 0x63. Only al changes (ebx and ecx are saved and restored); the flags are those of the last
// executed flag writer: `or ebx, ebx` on the empty-mask exit, else `dec ecx` (CF kept from the
// `shr ebx, 1` that set it).
// FUNCTION: GP4 0x0052f408
void __cdecl FUN_0052f408(gp4::Regs* r) {
    uint32_t count = r->eax + 1u;
    uint32_t mask = GP4_GLOBAL(uint32_t, 0x0081e5d4);
    uint32_t al = 0x60u;
    uint32_t arith;
    for (;;) {
        arith = gp4::flags::logic32(mask);                      // or ebx, ebx
        if (mask == 0u) {
            al = 0x63u;
            break;
        }
        al = (al + 1u) & 0xffu;                                 // inc al
        const uint32_t carry = mask & 1u;
        mask >>= 1;                                             // shr ebx, 1
        if (carry == 0u) {                                      // jae
            continue;
        }
        arith = gp4::flags::dec32(count, gp4::flags::CF);       // dec ecx (CF = 1 from the shr)
        count -= 1u;
        if (count == 0u) {
            break;
        }
    }
    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0052f408, FUN_0052f408, "regs(eax:u32[0..20]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
