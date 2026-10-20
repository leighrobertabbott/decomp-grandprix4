#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of a byte logic op (test/or): CF=OF=AF=0, PF/ZF/SF from the result.
static inline uint32_t logicFlags8(uint8_t result) {
    return parityFlag8(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80u) != 0u ? 0x80u : 0u);
}

// `cmp left, right` on bytes.
static inline uint32_t cmpFlags8(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// `inc` on 32 bits: CF is left alone (the caller keeps it).
static inline uint32_t incFlags32(uint32_t before, uint32_t result) {
    return parityFlag8(result)
        | (((before ^ 1u ^ result) & 0x10u))
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (before == 0x7fffffffu ? 0x800u : 0u);
}

// `dec` on 32 bits: CF is left alone (the caller keeps it).
static inline uint32_t decFlags32(uint32_t before, uint32_t result) {
    return parityFlag8(result)
        | (((before ^ 1u ^ result) & 0x10u))
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (before == 0x80000000u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0040f112
void __cdecl FUN_0040f112(gp4::Regs* r) {
    const uint8_t* body = reinterpret_cast<const uint8_t*>(r->esi);
    uint8_t* table = GP4_ARRAY(uint8_t, 0x007c3260);
    uint32_t eax = r->eax;
    uint32_t ecx = r->ecx;
    uint32_t edx = r->edx;
    uint32_t flags;

    const uint8_t guard = GP4_GLOBAL(uint8_t, 0x00852d8c);
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
    if (guard != 0) {
        flags = logicFlags8(guard);
    } else if (mode != 0x80) {
        flags = cmpFlags8(mode, 0x80);
    } else {
        const uint32_t before = body[0x70];
        edx = before - 1u;
        // `cmp mode, 0x80` was equal, so CF is clear for the dec.
        flags = decFlags32(before, edx);
        if (static_cast<int32_t>(edx) >= 0) {
            edx *= 0x16u;
            eax = body[0x415] >> 1;
            bool store = false;
            if (table[edx + eax] == 0) {
                store = true;
            } else {
                eax = 0;
                ecx = 0x16;
                for (;;) {
                    if (table[edx + eax] == 0) {
                        store = true;
                        break;
                    }
                    eax += 1u;
                    ecx -= 1u;
                    if (ecx == 0u) {
                        // `test` cleared CF before the `inc`.
                        flags = incFlags32(eax - 1u, eax);
                        break;
                    }
                }
            }
            if (store) {
                ecx = body[0x7c] & 0x3fu;
                const uint8_t bit = static_cast<uint8_t>(body[0x8d] & 2u);
                flags = logicFlags8(bit);
                if (bit != 0u) {
                    ecx |= 0x80u;
                    flags = logicFlags8(static_cast<uint8_t>(ecx));
                }
                table[edx + eax] = static_cast<uint8_t>(ecx);
            }
        }
    }

    r->eax = eax;
    r->ecx = ecx;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040f112, FUN_0040f112, "regs(esi:ptr[0x428]:layout=rec_0040f112, eax:u32, ecx:u32, edx:u32) -> (eax, ecx, edx, cf, pf, af, zf, sf, of); globals=layout=state_0040f112")
