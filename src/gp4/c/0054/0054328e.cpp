#include <gp4/gp4.h>
#include <gp4/flags.h>

struct Unknown_008310c0 {
    int32_t Unknown00;
    int32_t Unknown04;
    int32_t Unknown08;
};

// pushal wrapper: q = (Unknown04 * 0x4000) / [0x83137b] (64-bit unsigned divide, edx:eax by the
// divisor) when [0x83137b] is above the high half of the product, else q = [0x83137b]. Stores q in
// Unknown08, then Unknown00 = clamp(Unknown00 + ((old08 - q) >> 1), 0 .. Unknown04 - Unknown08).
// Every GPR is restored; the flags are those of the final `cmp eax, ebx`.
// FUNCTION: GP4 0x0054328e
void __cdecl FUN_0054328e(gp4::Regs* r) {
    Unknown_008310c0* s = reinterpret_cast<Unknown_008310c0*>(0x008310c0u);
    const uint32_t previous = static_cast<uint32_t>(s->Unknown08);
    const int64_t product = static_cast<int64_t>(s->Unknown04) * 0x4000;
    const uint32_t high = static_cast<uint32_t>(static_cast<uint64_t>(product) >> 32);
    const uint32_t divisor = GP4_GLOBAL(uint32_t, 0x0083137b);
    uint32_t quotient;
    if (divisor <= high) {
        quotient = GP4_GLOBAL(uint32_t, 0x0083137b);
    } else {
        uint32_t rem;
        quotient = gp4::div64(static_cast<uint64_t>(product), divisor, &rem);
    }
    s->Unknown08 = static_cast<int32_t>(quotient);
    const int32_t half = static_cast<int32_t>(previous - quotient) >> 1;
    const int32_t base = s->Unknown00;
    int32_t value = static_cast<int32_t>(static_cast<uint32_t>(base) + static_cast<uint32_t>(half));
    if (static_cast<int64_t>(base) + static_cast<int64_t>(half) < 0) {
        value = 0;
    }
    const int32_t limit = static_cast<int32_t>(static_cast<uint32_t>(s->Unknown04) - static_cast<uint32_t>(s->Unknown08));
    const uint32_t arith = gp4::flags::sub32(static_cast<uint32_t>(value), static_cast<uint32_t>(limit));
    if (value > limit) {
        value = limit;
    }
    s->Unknown00 = value;
    r->eflags = gp4::flags::merge(r->eflags, arith);
}
GP4_IMPL(0x0054328e, FUN_0054328e, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=scroll_00543340")
