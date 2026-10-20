#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t p = value & 0xffu;
    p ^= p >> 4;
    p ^= p >> 2;
    p ^= p >> 1;
    return (p & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 0x01u : 0u)
        | parityFlag8(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00441a1b
// Fills the per-car level byte 0x0066d94c[k] (k = ((byte +0x7c of the car at esi) - 1) & 0x3f, at most 21)
// from the 0x2c-byte level table 0x006f2c3e, indexed by the byte 0x0066d936[k], when the slot still holds
// 0xff and the table entry is below 2 (signed). eax = k; edx / cl are only set once the slot was 0xff;
// the flags are those of the last compare.
void __cdecl FUN_00441a1b(gp4::Regs* r) {
    const uint8_t* const car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));

    uint32_t index = (static_cast<uint32_t>(*(car + 0x7c)) - 1u) & 0x3fu;
    if (static_cast<int32_t>(index) > 0x15) {
        index = 0x15;
    }
    r->eax = index;

    const uint8_t level = GP4_ARRAY(uint8_t, 0x0066d94c)[index];
    uint32_t flags = cmpFlags8(level, 0xffu);
    if (level == 0xffu) {
        const uint32_t row = GP4_ARRAY(uint8_t, 0x0066d936)[index];
        r->edx = row;
        const int8_t entry = GP4_ARRAY(int8_t, 0x006f2c3e)[row];
        r->ecx = (r->ecx & 0xffffff00u) | static_cast<uint8_t>(entry);
        flags = cmpFlags8(static_cast<uint8_t>(entry), 2u);
        if (entry < 2) {
            GP4_ARRAY(uint8_t, 0x0066d94c)[index] = static_cast<uint8_t>(entry);
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00441a1b, FUN_00441a1b, "regs(ecx:u32,esi:ptr[0x84]:bytes) -> (eax,ecx,edx,cf,pf,af,zf,sf,of); globals=layout=tbl_0066d94c+counter_0040f0dd")
