#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Walks the 22 car records at 0x0066fef0 (stride 0x41c) and sets flag byte +0x237 on every car whose
// dword +0xf0 is a nonzero due time already reached by the clock [0x0066d814], unless byte +0x7b has
// bit 1 or +0x237 is already set. The registers are saved/restored; the flags left are those of the
// final `add esi, 0x41c`.
// FUNCTION: GP4 0x004436f9
void __cdecl FUN_004436f9(gp4::Regs* r) {
    uint8_t* car = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(0x0066fef0u));
    for (int remaining = 0x16; remaining != 0; --remaining) {
        if ((car[0x7b] & 2u) == 0u && car[0x237] == 0u) {
            const uint32_t due = *reinterpret_cast<const uint32_t*>(car + 0xf0);
            if (due != 0u) {
                const uint32_t elapsed = GP4_GLOBAL(uint32_t, 0x0066d814) - due;
                if ((elapsed & 0x80000000u) == 0u) {
                    car[0x237] = 1u;
                }
            }
        }
        car += 0x41c;
    }

    r->eflags = (r->eflags & ~0x8d5u) | addFlags32(0x0067553cu, 0x41cu);
}
GP4_IMPL(0x004436f9, FUN_004436f9, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=carx_0042fe2f+cars_004436f9+clock_0066d814")
