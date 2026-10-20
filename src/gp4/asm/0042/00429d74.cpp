#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// PF/AF/ZF/SF/OF of cmp left, right (the carry is replaced by stc/clc)
static inline uint32_t cmpFlags(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// |(int16)(a - b)| as the original computes it: movsx of the word difference, then neg.
static inline int32_t absWordDiff(uint16_t a, uint16_t b) {
    int32_t d = (int16_t)(uint16_t)(a - b);
    if (d < 0) {
        d = -d;
    }
    return d;
}

// FUNCTION: GP4 0x00429d74
void __cdecl FUN_00429d74(gp4::Regs* r) {
    void* rec = (void*)r->esi;
    int32_t eax = GP4_FIELD(int32_t, rec, 0x1a0);
    uint32_t flags = cmpFlags((uint32_t)eax, GP4_GLOBAL(uint32_t, 0x00603e0c));
    bool carry = true;  // stc

    if (eax < GP4_GLOBAL(int32_t, 0x00603e0c)) {
        eax = absWordDiff(GP4_FIELD(uint16_t, rec, 0x90), GP4_FIELD(uint16_t, rec, 0x94));
        flags = cmpFlags((uint32_t)eax, GP4_GLOBAL(uint32_t, 0x00603e10));
        if (eax >= GP4_GLOBAL(int32_t, 0x00603e10)) {
            carry = false;  // clc
        } else {
            const uint16_t other = GP4_FIELD(uint16_t, rec, 0x416);
            r->edx = (r->edx & 0xffff0000u) | other;
            eax = absWordDiff(GP4_FIELD(uint16_t, rec, 0x18), other);
            flags = cmpFlags((uint32_t)eax, GP4_GLOBAL(uint32_t, 0x00603e14));
            if (eax >= GP4_GLOBAL(int32_t, 0x00603e14)) {
                carry = false;  // clc
            }
        }
    }

    r->eax = (uint32_t)eax;
    r->eflags = (r->eflags & ~0x8d5u) | flags | (carry ? 1u : 0u);
}
GP4_IMPL(0x00429d74, FUN_00429d74, "regs(edx:u32, esi:ptr[0x41e]:bytes) -> (eax, edx, cf, pf, af, zf, sf, of); globals=fuzz:i32")
