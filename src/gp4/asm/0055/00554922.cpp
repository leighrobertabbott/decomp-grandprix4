#include <gp4/gp4.h>
#include <gp4/flags.h>

// Nudges a value pair by a signed step: al is compared (signed bytes) with the limit [0x00874edd],
// ch is the signed step. Below the limit a negative step gives ah = al + ch and a non-negative one
// al = ah - ch; at or above the limit the roles swap (negative: al = ah - ch, non-negative:
// ah = al + ch). The flags are those of the add/sub that produced the result.
// FUNCTION: GP4 0x00554922
void __cdecl FUN_00554922(gp4::Regs* r) {
    const uint8_t al = static_cast<uint8_t>(r->eax);
    const uint8_t ah = static_cast<uint8_t>(r->eax >> 8);
    const uint8_t step = static_cast<uint8_t>(r->ecx >> 8);
    const int8_t limit = GP4_GLOBAL(int8_t, 0x00874edd);
    const bool below = static_cast<int8_t>(al) < limit;
    const bool negative = static_cast<int8_t>(step) < 0;
    uint32_t flags;
    if (below == negative) {
        // ah = al + ch
        const uint8_t sum = static_cast<uint8_t>(al + step);
        r->eax = (r->eax & 0xffff00ffu) | (static_cast<uint32_t>(sum) << 8);
        flags = gp4::flags::add8(al, step);
    } else {
        // al = ah - ch
        const uint8_t diff = static_cast<uint8_t>(ah - step);
        r->eax = (r->eax & 0xffffff00u) | diff;
        flags = gp4::flags::sub8(ah, step);
    }
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00554922, FUN_00554922, "regs(eax:u32, ecx:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32")
