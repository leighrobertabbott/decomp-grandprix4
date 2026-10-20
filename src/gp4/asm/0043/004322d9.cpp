#include <gp4/gp4.h>

// EFLAGS bits: CF 0x1, PF 0x4, AF 0x10, ZF 0x40, SF 0x80, OF 0x800.
static inline uint32_t parityFlag(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x4u : 0u;
}

// Flags of the 32-bit "cmp left, right".
static inline uint32_t subFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u)
        | (left < right ? 0x1u : 0u);
}

// Flags of a 32-bit logic result ("or eax, eax"): CF = OF = AF = 0.
static inline uint32_t logicFlags(uint32_t result) {
    return parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Returns with CF clear when the entry's range value (edx) minus the 11-bit field of
// the linked record lies inside [0x62a940, 0x62a93c); CF set otherwise.
// FUNCTION: GP4 0x004322d9
void __cdecl FUN_004322d9(gp4::Regs* r) {
    const uint8_t* body = (const uint8_t*)r->esi;

    const uint32_t edx = (uint32_t)(body[0xfd] & 0x1fu) * 3u
        + (uint32_t)GP4_GLOBAL(uint16_t, 0x00868562)
        + GP4_GLOBAL(uint32_t, 0x0062a944);

    uint32_t eax = *(const uint32_t*)(body + 0x1c);
    uint32_t flags = subFlags(eax, GP4_GLOBAL(uint32_t, 0x00891500));
    bool linked = true;
    if (!(eax >= GP4_GLOBAL(uint32_t, 0x00891500))) {
        flags = subFlags(eax, GP4_GLOBAL(uint32_t, 0x00866f00));
        if (eax < GP4_GLOBAL(uint32_t, 0x00866f00)) {
            linked = false;
        } else {
            eax = *(const uint32_t*)(eax + 0x44);
            flags = logicFlags(eax);
            if (eax == 0u) {
                linked = false;
            }
        }
    }

    if (!linked) {
        flags |= 0x1u;                      // stc
    } else {
        const uint32_t field = (uint32_t)(*(const uint16_t*)(eax + 0x1a)) & 0x7ffu;
        eax = edx - field;
        flags = subFlags(eax, GP4_GLOBAL(uint32_t, 0x0062a940));
        if ((int32_t)eax < GP4_GLOBAL(int32_t, 0x0062a940)) {
            flags |= 0x1u;                  // stc
        } else {
            flags = subFlags(eax, GP4_GLOBAL(uint32_t, 0x0062a93c));
            if ((int32_t)eax >= GP4_GLOBAL(int32_t, 0x0062a93c)) {
                flags |= 0x1u;              // stc
            } else {
                flags &= ~0x1u;             // clc
            }
        }
    }

    r->eax = eax;
    r->edx = edx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004322d9, FUN_004322d9, "regs(esi:rec[0x66fef0,0x41c,0..21]) -> (eax, edx, cf, pf, af, zf, sf, of); globals=layout=cars_0066fef0")
