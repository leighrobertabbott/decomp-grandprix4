#include <gp4/gp4.h>

static inline uint32_t parityFlag8(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of `or reg, reg` (CF=OF=AF=0).
static inline uint32_t orFlags32(uint32_t value) {
    return parityFlag8(value) | (value == 0u ? 0x40u : 0u) | ((value >> 31) << 7);
}

// Flags of `sub left, right` on 32 bits (CF included).
static inline uint32_t subFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag8(result)
        | ((left ^ right ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 31) << 7)
        | (((left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x0042fa52
void __cdecl FUN_0042fa52(gp4::Regs* r) {
    uint8_t* values = GP4_ARRAY(uint8_t, 0x006077e4);
    uint8_t* keys = GP4_ARRAY(uint8_t, 0x00607860);
    const uint32_t count = GP4_GLOBAL(uint32_t, 0x006077dc);
    int32_t offset = static_cast<int32_t>(count - 4u);
    uint32_t value = r->ebx;
    uint32_t key = r->ebp;
    uint32_t flags;
    bool found = false;

    if (offset < 0) {
        flags = subFlags32(count, 4u);
    } else if (offset == 0) {
        value = GP4_FIELD(uint32_t, values, 0);
        flags = orFlags32(value);
        if (value != 0u) {
            key = GP4_FIELD(uint32_t, keys, 0);
            found = true;
        }
    } else {
        key = 0x7fffffffu;
        value = 0u;
        do {
            const uint32_t entry = GP4_FIELD(uint32_t, values, offset);
            if (entry != 0u && static_cast<int32_t>(key) >= static_cast<int32_t>(GP4_FIELD(uint32_t, keys, offset))) {
                key = GP4_FIELD(uint32_t, keys, offset);
                value = entry;
            }
            offset -= 4;
        } while (offset >= 0);
        flags = orFlags32(value);
        found = value != 0u;
    }

    r->eax = static_cast<uint32_t>(offset);
    r->ebx = value;
    r->ebp = key;
    // clc on success, stc otherwise; the other flags stay as the last compare/or left them.
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | (found ? 0u : 1u);
}
GP4_IMPL(0x0042fa52, FUN_0042fa52, "regs(ebx:u32, ebp:u32) -> (eax, ebx, ebp, cf, pf, af, zf, sf, of); globals=layout=state_0042fa52")
