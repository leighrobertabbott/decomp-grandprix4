#include <gp4/gp4.h>
#include <gp4/flags.h>

// FUNCTION: GP4 0x0051c977
// Per-texel brightness of a radial pattern (called 0x100 times per row by 0x0051c9cb): the
// distance sqrt(((esi >> 1)^2) + ((ebp + [0x811918])^2)) indexes the byte table at 0x9cfc28; that
// byte plus a masked random number ([0x81190c] & the 0x004014ba generator), scaled by 2^9 and
// biased by ecx, is brought down by >> 12 / >> 4 and clamped to 0x6f..0x87. edx and ecx are
// preserved; the flags are those of `cmp eax, 0x6f` on the value after the upper clamp.
void __cdecl FUN_0051c977(gp4::Regs* r) {
    const int32_t half = static_cast<int32_t>(r->esi) >> 1;
    const uint32_t squareX = static_cast<uint32_t>(static_cast<int64_t>(half) * half);
    const int32_t y = static_cast<int32_t>(r->ebp + GP4_GLOBAL(uint32_t, 0x00811918));
    const uint32_t squareY = static_cast<uint32_t>(static_cast<int64_t>(y) * y);

    gp4::Regs sqrtCall = {};
    sqrtCall.eax = squareY + squareX;
    gp4::call_regs(0x004010f3, sqrtCall);

    const uint32_t index = sqrtCall.eax & 0xffffu;
    const uint32_t texel = GP4_ARRAY(uint8_t, 0x009cfc28)[index];

    gp4::Regs randomCall = {};
    randomCall.eax = index;
    gp4::call_regs(0x004015dc, randomCall);

    uint32_t value = ((randomCall.eax & GP4_GLOBAL(uint32_t, 0x0081190c)) << 9) + r->ecx;
    value = (value >> 12) + texel;
    int32_t level = static_cast<int32_t>(value) >> 4;
    if (level > 0x87) {
        level = 0x87;
    }
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(level), 0x6fu);
    if (level < 0x6f) {
        level = 0x6f;
    }

    r->eax = static_cast<uint32_t>(level);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x0051c977, FUN_0051c977, "regs(esi:u32, ebp:u32, ecx:u32[0..0x900000]) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0062d1b4")
