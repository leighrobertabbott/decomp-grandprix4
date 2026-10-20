#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t value) {
    value &= 0xffu;
    return parityFlag(value) | (value == 0u ? 0x40u : 0u) | (value & 0x80u);
}

// FUNCTION: GP4 0x00443393
// Per-frame race-clock update. Two 48-bit fixed-point accumulators advance by the per-frame deltas
// [0x679b14]:[0x679b56] (-> [0x66d85c]:[0x66d86e]) and [0x679b10]:[0x679b54] (-> the clock [0x66d814]:[0x66d86c]);
// [0x66d8cc] = max(0, [0x679b04] - clock) (time remaining), and its decimal split by 0x0040996f, shifted right by 8,
// is kept in [0x66d8ca] (a change also raises [0x852d70] / [0x852d6e]). Depending on the session flags
// ([0x7c2ea9] sign, [0x7c2e75], [0x7aafd4], [0x66d87c]) it then arms the countdown ([0x66d87a] = [0x66d870] = 0x80,
// deadline [0x7c2eb4] = clock + 6000) or, once armed and past the deadline, raises bit 6 / bit 5 of [0x7c2ead].
// For the selected car [0x679d30] equal to [0x679d2c] it refreshes the lap-time digits through 0x0044334d and sets
// bit 1 of the car's byte +0x89 when [0x66d860] changed (eax then holds the old [0x66d860]). [0x7c2e80] becomes 0xff
// while [0x852d8c] is zero. esi = [0x679d30]; the flags are those of the final `test byte [0x852d8c], 0xff`.
void __cdecl FUN_00443393(gp4::Regs* r) {
    GP4_GLOBAL(uint32_t, 0x0066d85c) += GP4_GLOBAL(uint32_t, 0x00679b14);
    {
        const uint16_t delta = GP4_GLOBAL(uint16_t, 0x00679b56);
        const uint16_t sum = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x0066d86e) + delta);
        GP4_GLOBAL(uint16_t, 0x0066d86e) = sum;
        if (sum < delta) {
            GP4_GLOBAL(uint32_t, 0x0066d85c) += 1u;
        }
    }
    GP4_GLOBAL(uint32_t, 0x0066d814) += GP4_GLOBAL(uint32_t, 0x00679b10);
    {
        const uint16_t delta = GP4_GLOBAL(uint16_t, 0x00679b54);
        const uint16_t sum = static_cast<uint16_t>(GP4_GLOBAL(uint16_t, 0x0066d86c) + delta);
        GP4_GLOBAL(uint16_t, 0x0066d86c) = sum;
        if (sum < delta) {
            GP4_GLOBAL(uint32_t, 0x0066d814) += 1u;
        }
    }

    int32_t remaining = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x00679b04) - GP4_GLOBAL(uint32_t, 0x0066d814));
    if (remaining < 0) {
        remaining = 0;
    }
    GP4_GLOBAL(int32_t, 0x0066d8cc) = remaining;

    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;
    c.eax = static_cast<uint32_t>(remaining);
    gp4::call_regs(0x0040996f, c);
    uint32_t eax = static_cast<uint32_t>(static_cast<int32_t>(c.eax) >> 8);

    GP4_GLOBAL(uint16_t, 0x0066d8ca) = static_cast<uint16_t>(eax);
    if (GP4_GLOBAL(uint16_t, 0x00603850) != static_cast<uint16_t>(eax)) {
        GP4_GLOBAL(uint16_t, 0x00603850) = static_cast<uint16_t>(eax);
        GP4_GLOBAL(uint8_t, 0x00852d70) = 0x80;
        GP4_GLOBAL(uint16_t, 0x00852d6e) = 0x80;
    }

    // Does the flag state reach the countdown block (0x44347e)?
    bool reachTail;
    if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x80u) != 0u) {
        reachTail = true;
    } else {
        reachTail = !(GP4_GLOBAL(uint8_t, 0x007c2e75) == 0u && GP4_GLOBAL(uint32_t, 0x0066d8cc) != 0u);
    }
    bool countdown = false;
    if (reachTail) {
        if (GP4_GLOBAL(uint8_t, 0x007aafd4) != 0u && (GP4_GLOBAL(uint8_t, 0x0066d87c) & 0x40u) == 0u) {
            countdown = false;
        } else {
            countdown = (GP4_GLOBAL(uint8_t, 0x0066d87c) & 0x80u) != 0u;
        }
    }
    if (countdown) {
        if ((GP4_GLOBAL(uint8_t, 0x0066d87a) & 0x80u) == 0u) {
            GP4_GLOBAL(uint8_t, 0x0066d87a) = 0x80;
            GP4_GLOBAL(uint8_t, 0x0066d870) = 0x80;
            GP4_GLOBAL(uint32_t, 0x007c2eb4) = GP4_GLOBAL(uint32_t, 0x0066d814);
            GP4_GLOBAL(uint32_t, 0x007c2eb4) += 0x1770u;
        } else if (GP4_GLOBAL(uint32_t, 0x007c2eb4) != 0u) {
            const uint32_t elapsed = GP4_GLOBAL(uint32_t, 0x0066d814) - GP4_GLOBAL(uint32_t, 0x007c2eb4);
            if ((elapsed & 0x80000000u) == 0u) {
                if ((GP4_GLOBAL(uint8_t, 0x007c2ea9) & 0x80u) != 0u) {
                    GP4_GLOBAL(uint8_t, 0x007c2ead) |= 0x40;
                } else {
                    GP4_GLOBAL(uint8_t, 0x007c2ead) |= 0x20;
                }
            }
        }
    }

    const uint32_t esi = GP4_GLOBAL(uint32_t, 0x00679d30);
    if (esi == GP4_GLOBAL(uint32_t, 0x00679d2c)) {
        const uint32_t before = GP4_GLOBAL(uint32_t, 0x0066d860);
        c.eax = eax;
        c.esi = esi;
        gp4::call_regs(0x0044334d, c);
        eax = before;
        if (before != GP4_GLOBAL(uint32_t, 0x0066d860)) {
            GP4_FIELD(uint8_t, reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(esi)), 0x89) |= 2;
        }
    }

    const uint32_t gate = GP4_GLOBAL(uint8_t, 0x00852d8c);
    if (gate == 0u) {
        GP4_GLOBAL(uint8_t, 0x007c2e80) = 0xff;
    }

    r->eax = eax;
    r->esi = esi;
    r->eflags = (r->eflags & ~0x8d5u) | testFlags8(gate);
}
GP4_IMPL(0x00443393, FUN_00443393, "regs() -> (eax, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040f898+state_0044334d+order_00418f8f+state_004320f2")
