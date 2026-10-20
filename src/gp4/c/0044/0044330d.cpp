#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit logic op `or reg, reg` (CF = OF = AF = 0).
static inline uint32_t orFlags32(uint32_t value) {
    return byteParity(value)
        | (value == 0u ? 0x040u : 0u)
        | ((value & 0x80000000u) != 0u ? 0x080u : 0u);
}

// Flags of an 8-bit TEST byte, mask (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint8_t value) {
    return byteParity(value)
        | (value == 0u ? 0x040u : 0u)
        | ((value & 0x80u) != 0u ? 0x080u : 0u);
}

// Flags of the 8-bit CMP left, right.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit CMP left, right.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Carry-flag predicate on the car record [0x0066d820]: CF is set when the car is null, has any of
// the flag bits 0x80/0x40/0x10 in byte +0x7a, or has byte +0x70 >= 2 and the clock [0x0066d814] is at
// least 6000 past the dword +0xc0. The other flags are those of the last test/compare; eax and esi
// are saved and restored.
// FUNCTION: GP4 0x0044330d
void __cdecl FUN_0044330d(gp4::Regs* r) {
    const uint8_t* car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x0066d820)));
    uint32_t flags = orFlags32(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(car)));
    uint32_t carry = 1u;

    if (car != nullptr) {
        flags = testFlags8(car[0x7a] & 0x80u);
        if ((car[0x7a] & 0x80u) == 0u) {
            flags = testFlags8(car[0x7a] & 0x40u);
            if ((car[0x7a] & 0x40u) == 0u) {
                flags = testFlags8(car[0x7a] & 0x10u);
                if ((car[0x7a] & 0x10u) == 0u) {
                    const uint8_t lap = car[0x70];
                    flags = cmpFlags8(lap, 2u);
                    if (static_cast<int8_t>(lap) < 2) {
                        carry = 0u;
                    } else {
                        const uint32_t elapsed = GP4_GLOBAL(uint32_t, 0x0066d814) - *reinterpret_cast<const uint32_t*>(car + 0xc0);
                        flags = cmpFlags32(elapsed, 0x1770u);
                        if (static_cast<int32_t>(elapsed) < 0x1770) {
                            carry = 0u;
                        }
                    }
                }
            }
        }
    }

    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x0044330d, FUN_0044330d, "regs() -> (cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01+flags_00408f7b+lap_0043b482+clock_0066d814+due_0066ffb0")
