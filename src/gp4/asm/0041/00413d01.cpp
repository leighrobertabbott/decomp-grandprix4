#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Arithmetic flags of `cmp left, right` except CF.
static inline uint32_t cmpFlagsNoCarry(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00413d01
void __cdecl FUN_00413d01(gp4::Regs* r) {
    // Checks four pairs of dwords (|[+0x34c+4i]| <= [0x602084] and [0x602088] <= [+0x2ec+4i] <
    // [0x60208c], all signed). The first violation returns CF = 1 with the flags of the failing
    // cmp and eax = the compared value. If all four pass, bit 4 of byte [esi+0x203] is cleared
    // and CF = 0 (the other flags are those of the AND). ecx is saved and restored.
    uint8_t* body = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint32_t eax = r->eax;
    uint32_t flags;
    uint32_t i = 0;
    for (;;) {
        eax = GP4_FIELD(uint32_t, body, 0x34c + i * 4);
        if (static_cast<int32_t>(eax) < 0) eax = 0u - eax;
        const uint32_t limit = GP4_GLOBAL(uint32_t, 0x00602084);
        if (static_cast<int32_t>(eax) > static_cast<int32_t>(limit)) {
            flags = cmpFlagsNoCarry(eax, limit) | 1u;
            break;
        }
        eax = GP4_FIELD(uint32_t, body, 0x2ec + i * 4);
        const uint32_t low = GP4_GLOBAL(uint32_t, 0x00602088);
        if (static_cast<int32_t>(eax) < static_cast<int32_t>(low)) {
            flags = cmpFlagsNoCarry(eax, low) | 1u;
            break;
        }
        const uint32_t high = GP4_GLOBAL(uint32_t, 0x0060208c);
        if (static_cast<int32_t>(eax) >= static_cast<int32_t>(high)) {
            flags = cmpFlagsNoCarry(eax, high) | 1u;
            break;
        }
        if (++i == 4u) {
            const uint32_t result = GP4_FIELD(uint8_t, body, 0x203) & 0xfbu;
            GP4_FIELD(uint8_t, body, 0x203) = static_cast<uint8_t>(result);
            // logic flags: CF = OF = AF = 0
            flags = parityFlag(result)
                | (result == 0u ? 0x40u : 0u)
                | ((result & 0x80u) != 0u ? 0x80u : 0u);
            break;
        }
    }
    r->eax = eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00413d01, FUN_00413d01, "regs(esi:ptr[0x35c]:layout=rec_00413d01) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00413d01")
