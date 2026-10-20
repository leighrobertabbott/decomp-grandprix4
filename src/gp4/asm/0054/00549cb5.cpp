#include <gp4/gp4.h>
#include <gp4/flags.h>

// Rescales ebx: ((((ebx << 8) * 0x36) / w + 0x1400) * [0x00b1bf40] / 0x50 + 0x80) >> 8, where
// w = word [0x007c2e96] (the first quotient is 0 when w is 0). Both divisions are idiv of the
// zero-extended eax (edx is cleared first). eax/ecx/edx are saved and restored; the flags are those
// of the final `shr ebx, 8` (CF = bit 7 of the pre-shift value, OF = AF = 0).
// FUNCTION: GP4 0x00549cb5
void __cdecl FUN_00549cb5(gp4::Regs* r) {
    uint32_t scaled = (r->ebx << 8) * 0x36u;
    const uint32_t divisor = GP4_GLOBAL(uint16_t, 0x007c2e96);
    int32_t rem;
    int32_t quotient = 0;
    if (divisor != 0u) {
        quotient = gp4::idiv64(static_cast<int64_t>(scaled), static_cast<int32_t>(divisor), &rem);
    }
    scaled = (static_cast<uint32_t>(quotient) + 0x1400u) * GP4_GLOBAL(uint32_t, 0x00b1bf40);
    quotient = gp4::idiv64(static_cast<int64_t>(scaled), 0x50, &rem);
    const uint32_t value = static_cast<uint32_t>(quotient) + 0x80u;
    const uint32_t result = value >> 8;

    r->ebx = result;
    r->eflags = gp4::flags::merge(r->eflags, gp4::flags::szp(result, 32) | (((value >> 7) & 1u) ? gp4::flags::CF : 0u));
}
GP4_IMPL(0x00549cb5, FUN_00549cb5, "regs(ebx:u32[0..0xffff]) -> (ebx, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040ed29")
