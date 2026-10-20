#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// flags of cmp left, right
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 1u : 0u);
}

// FUNCTION: GP4 0x0043ec1a
void __cdecl FUN_0043ec1a(gp4::Regs* r) {
    // pushal ... popal: every register comes back, only the flags of the last cmp/test remain
    const int32_t first = GP4_GLOBAL(int32_t, 0x0062ac48);
    uint32_t flags = 0x44u;   // "test [count], -1" with count == 0: ZF and PF
    if (first != 0) {
        uint32_t value = GP4_GLOBAL(uint32_t, 0x0062ac64);
        uint32_t body = GP4_GLOBAL(uint32_t, 0x00626990);
        int32_t index = 0;
        for (;;) {
            gp4::Regs call = *r;
            call.eax = value;
            call.ebx = value;
            call.ecx = static_cast<uint32_t>(index);
            call.edi = body;
            gp4::call_regs(0x0043ebc2, call);
            value += GP4_GLOBAL(uint32_t, 0x0062ac68);

            body += 0x160u;
            if (GP4_GLOBAL(uint8_t, 0x0086857a) != 0) {
                // wrap back one record when the end pointer is reached
                if (body >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                    body -= 0x160u;
                }
            } else if (body >= GP4_GLOBAL(uint32_t, 0x00866ecc)) {
                if (body < GP4_GLOBAL(uint32_t, 0x00891500)) {
                    body -= GP4_GLOBAL(uint32_t, 0x00866ec4);
                } else if (body >= GP4_GLOBAL(uint32_t, 0x00866ef8)) {
                    body = GP4_GLOBAL(uint32_t, 0x00866ef8) - 0x160u;
                }
            }

            ++index;
            const int32_t limit = GP4_GLOBAL(int32_t, 0x0062ac48);
            flags = subFlags(static_cast<uint32_t>(index), static_cast<uint32_t>(limit));
            if (index >= limit) {
                break;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043ec1a, FUN_0043ec1a, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=ring_0043ec1a")
