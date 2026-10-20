#include <gp4/gp4.h>
#include <gp4/flags.h>

// Running estimate of a per-sample amount: counts one more sample in [0x93243c] and adds the
// per-sample step [0x6f2c00] (capped at twice the rate [0x7ad8b8] when the byte [0x82fd31] is set)
// to the accumulator [0x932438], which saturates at 0x400000. [0x7a8dec] =
// clamp(((acc * 0x4000) / (count * rate)) * rate >> 14, 0x444, 0x2000). Leaves ebp = count * rate
// (low dword), edx = high dword of the last product and eax = the clamped result; the flags are those
// of the final `cmp eax, 0x2000`.
// FUNCTION: GP4 0x00516c68
void __cdecl FUN_00516c68(gp4::Regs* r) {
    const int32_t rate = GP4_GLOBAL(int32_t, 0x007ad8b8);

    const int32_t count = GP4_GLOBAL(int32_t, 0x0093243c) + 1;
    GP4_GLOBAL(int32_t, 0x0093243c) = count;
    const int32_t divisor = static_cast<int32_t>(static_cast<int64_t>(count) * rate);

    int32_t step = GP4_GLOBAL(int32_t, 0x006f2c00);
    if (GP4_GLOBAL(uint8_t, 0x0082fd31) != 0u) {
        const int32_t cap = rate << 1;
        if (step > cap) step = cap;
    }
    int32_t acc = static_cast<int32_t>(static_cast<uint32_t>(step) + GP4_GLOBAL(uint32_t, 0x00932438));
    if (acc > 0x400000) acc = 0x400000;
    GP4_GLOBAL(int32_t, 0x00932438) = acc;

    int32_t remainder;
    const int32_t quotient = gp4::idiv64(static_cast<int64_t>(acc) * 0x4000, divisor, &remainder);
    const int64_t product = static_cast<int64_t>(quotient) * rate;
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    int32_t value = static_cast<int32_t>(static_cast<uint64_t>(product) >> 14);

    if (value < 0x444) value = 0x444;
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(value), 0x2000u);
    if (value > 0x2000) value = 0x2000;
    GP4_GLOBAL(int32_t, 0x007a8dec) = value;

    r->eax = static_cast<uint32_t>(value);
    r->edx = high;
    r->ebp = static_cast<uint32_t>(divisor);
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00516c68, FUN_00516c68, "regs(ebp:u32{0,1,5}) -> (eax, edx, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=rate_007ad8b8+rate_00516c68")
