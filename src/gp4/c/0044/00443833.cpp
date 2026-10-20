#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit result with the given carry/aux/overflow bits.
static inline uint32_t flags8(uint32_t result, uint32_t cf, uint32_t af, uint32_t of) {
    result &= 0xffu;
    return (cf ? 0x001u : 0u) | parityFlag(result) | (af ? 0x010u : 0u) | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u) | (of ? 0x800u : 0u);
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    const uint32_t result = (left - right) & 0xffu;
    return flags8(result, left < right, (left ^ right ^ result) & 0x10u, ((left ^ right) & (left ^ result)) & 0x80u);
}

// Flags of the 8-bit `add left, right`.
static inline uint32_t addFlags8(uint32_t left, uint32_t right) {
    const uint32_t sum = left + right;
    const uint32_t result = sum & 0xffu;
    return flags8(result, sum > 0xffu, (left ^ right ^ result) & 0x10u, (~(left ^ right) & (left ^ result)) & 0x80u);
}

// Flags of an 8-bit `test` (CF = OF = AF = 0).
static inline uint32_t testFlags8(uint32_t value) {
    return flags8(value, 0, 0, 0);
}

static inline void setFlags(gp4::Regs& c, uint32_t flags) {
    c.eflags = (c.eflags & ~0x8d5u) | flags;
}

// Per-frame update of the session: accumulates 0x004437af, returns when no session is active (0x007a93b7 == 0xff),
// then either runs the 0x004431cf / 0x00443996 / 0x00442988 chain (0x007c2ea9 >= 0) or the 0x00443abb / 0x004438f8
// chain with the two byte counters 0x0066d8ac (+2) and 0x0066d8ae (+0x20) whose carries trigger 0x0043b689 and
// 0x00442018, ending in 0x004425e0. Registers and flags flow from callee to callee.
// FUNCTION: GP4 0x00443833
void __cdecl FUN_00443833(gp4::Regs* r) {
    gp4::Regs c = *r;
    c.st_in = 0;
    c.st_out = 0;

    gp4::call_regs(0x004437af, c);
    const uint32_t active = GP4_GLOBAL(uint8_t, 0x007a93b7);
    setFlags(c, cmpFlags8(active, 0xffu));
    if (active == 0xffu) {
        *r = c;
        return;
    }

    gp4::call_regs(0x00441eb8, c);
    const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    setFlags(c, testFlags8(mode));
    if ((mode & 0x80u) == 0u) {
        gp4::call_regs(0x004431cf, c);
        gp4::call_regs(0x00443996, c);
        gp4::call_regs(0x00442988, c);
        *r = c;
        return;
    }

    gp4::call_regs(0x00443abb, c);
    gp4::call_regs(0x004438f8, c);

    const uint32_t counterA = GP4_GLOBAL(uint8_t, 0x0066d8ac);
    GP4_GLOBAL(uint8_t, 0x0066d8ac) = static_cast<uint8_t>(counterA + 2u);
    setFlags(c, addFlags8(counterA, 2u));
    if (counterA + 2u > 0xffu) {
        gp4::call_regs(0x0043b689, c);
    }

    const uint32_t enabled = GP4_GLOBAL(uint8_t, 0x0066d87f);
    setFlags(c, testFlags8(enabled));
    if (enabled != 0u) {
        const uint32_t counterB = GP4_GLOBAL(uint8_t, 0x0066d8ae);
        GP4_GLOBAL(uint8_t, 0x0066d8ae) = static_cast<uint8_t>(counterB + 0x20u);
        setFlags(c, addFlags8(counterB, 0x20u));
        if (counterB + 0x20u > 0xffu) {
            gp4::call_regs(0x00442018, c);
        }
    }

    gp4::call_regs(0x004425e0, c);
    *r = c;
}
GP4_IMPL(0x00443833, FUN_00443833, "regs(ecx:u32) -> (eax, ecx, edx, ebx, esi, edi, ebp, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=cars_0066fef0+carcc_0043a785+ring_008943a0+records_008943a0+segw_00441e25+state_00441eb8+state_0040e949+flags_00432dcf+cars_00433d8d+state_00432f18+state_004320f2+lap_0043b482+cars_00443648+cars_004436f9+state_00442d1f+state_0044334d+state_0043b920+cur_0066d81c+cur_00852d30+stream_0062d1a0+stream_0062d1a8")
