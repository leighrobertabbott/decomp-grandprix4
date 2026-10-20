#include <gp4/gp4.h>
#include <gp4/flags.h>

// Smoothing step on the state block at 0x8310cc {current, limit, previous quotient}. The new
// quotient is limit * 0x4000 / [0x83137f] (64-bit product divided by the unsigned dword; the
// divisor itself when it is not above the product's high half, so a zero divisor is safe) and is
// stored at +8. The current value moves by half the quotient's decrease ((old - new) >> 1,
// arithmetic), clamped below at 0 (signed overflow aware, as the `jge` after the add) and above at
// limit - quotient, and goes back to +0. pushal/popal keeps every register; the flags are those of
// the closing `cmp eax, ebx`.
// FUNCTION: GP4 0x00543340
void __cdecl FUN_00543340(gp4::Regs* r) {
    const uint32_t previous = GP4_FIELD(uint32_t, 0x008310ccu, 8);
    const int32_t limit = GP4_FIELD(int32_t, 0x008310ccu, 4);
    const int64_t product = static_cast<int64_t>(limit) * 0x4000;
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t low = static_cast<uint32_t>(product);
    const uint32_t divisor = GP4_GLOBAL(uint32_t, 0x0083137f);

    uint32_t quotient;
    if (divisor <= high) {
        quotient = divisor;
    } else {
        uint32_t remainder;
        quotient = gp4::div64((static_cast<uint64_t>(high) << 32) | low, divisor, &remainder);
    }
    GP4_FIELD(uint32_t, 0x008310ccu, 8) = quotient;

    const int32_t step = static_cast<int32_t>(previous - quotient) >> 1;
    const int32_t current = GP4_FIELD(int32_t, 0x008310ccu, 0);
    const uint32_t sumFlags = gp4::flags::add32(static_cast<uint32_t>(current), static_cast<uint32_t>(step));
    int32_t value = static_cast<int32_t>(static_cast<uint32_t>(current) + static_cast<uint32_t>(step));
    if (((sumFlags & gp4::flags::SF) != 0u) != ((sumFlags & gp4::flags::OF) != 0u)) {
        value = 0;
    }
    const int32_t ceiling = static_cast<int32_t>(static_cast<uint32_t>(limit) - quotient);
    const uint32_t flags = gp4::flags::sub32(static_cast<uint32_t>(value), static_cast<uint32_t>(ceiling));
    if (value > ceiling) {
        value = ceiling;
    }
    GP4_FIELD(int32_t, 0x008310ccu, 0) = value;
    r->eflags = gp4::flags::merge(r->eflags, flags);
}
GP4_IMPL(0x00543340, FUN_00543340, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=scroll_00543340")
