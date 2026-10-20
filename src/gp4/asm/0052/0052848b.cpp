#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `cmp a, b` (a - b, 32-bit).
static inline uint32_t cmpFlags(uint32_t a, uint32_t b) {
    const uint32_t result = a - b;
    return (a < b ? 1u : 0u)
        | parityFlag8(result)
        | (((a ^ b ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((a ^ b) & (a ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0052848b
// Lookup in the 8-entry table at 0x81cd91 (stride 0x1c): returns the first entry whose dword +0
// equals ebx and whose pointer at +4 points at a dword equal to eax, else 0. ecx and edx are
// saved and restored. Flags are those of the last cmp (the pointed-at compare on a hit, the
// end-of-table compare on a miss).
void __cdecl FUN_0052848b(gp4::Regs* r) {
    const uint32_t wantPointee = r->eax;
    const uint32_t wantKey = r->ebx;
    const uint8_t* entry = GP4_ARRAY(uint8_t, 0x0081cd91);
    const uint8_t* const end = GP4_ARRAY(uint8_t, 0x0081ce71);

    uint32_t result;
    uint32_t flags;
    for (;;) {
        if (*(const uint32_t*)entry == wantKey) {
            const uint32_t pointee = **(const uint32_t* const*)(entry + 4);
            if (pointee == wantPointee) {
                result = (uint32_t)entry;
                flags = cmpFlags(pointee, wantPointee);
                break;
            }
        }
        entry += 0x1c;
        if (entry == end) {
            result = 0u;
            flags = cmpFlags((uint32_t)entry, (uint32_t)end);
            break;
        }
    }
    r->eax = result;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0052848b, FUN_0052848b, "regs(eax:u32{0,1,2,3}, ebx:u32{0,1,2,3}) -> (eax, cf, pf, af, zf, sf, of); globals=layout=keys_0052848b")
