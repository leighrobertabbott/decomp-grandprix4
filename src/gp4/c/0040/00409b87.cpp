#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return ((value & 1u) == 0u) ? 4u : 0u;
}

// flags of "cmp byte, byte"
static inline uint32_t compareByteFlags(uint8_t left, uint8_t right) {
    const uint8_t result = static_cast<uint8_t>(left - right);
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | (((left ^ right) & (left ^ result) & 0x80u) != 0u ? 0x800u : 0u);
}

// FUNCTION: GP4 0x00409b87
void __cdecl FUN_00409b87(gp4::Regs* r) {
    // pushal ... popal: every register comes back, only the flags of the final cmp remain
    uint8_t* const first = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint8_t* selected = first;
    const uint8_t sought = GP4_GLOBAL(uint8_t, 0x0066d896);
    if (sought != 0u) {
        // look the id up in byte +0x7c of the 22 records (stride 0x41c)
        uint8_t* record = first;
        uint32_t remaining = 0x16u;
        bool found = false;
        while (remaining != 0u) {
            if (record[0x7c] == sought) {
                found = true;
                break;
            }
            record += 0x41c;
            --remaining;
        }
        if (found) {
            selected = record;
        } else {
            GP4_GLOBAL(uint8_t, 0x0066d896) = 0u;
        }
    }

    const uint32_t pointer = reinterpret_cast<uint32_t>(selected);
    GP4_GLOBAL(uint32_t, 0x00679d30) = pointer;
    GP4_GLOBAL(uint32_t, 0x00679d2c) = pointer;
    GP4_GLOBAL(uint32_t, 0x0066d820) = 0u;
    GP4_GLOBAL(uint32_t, 0x0066d81c) = 0u;
    GP4_GLOBAL(uint32_t, 0x00852d30) = 0u;
    if (GP4_GLOBAL(uint8_t, 0x0066d896) != 0u) {
        GP4_GLOBAL(uint32_t, 0x0066d820) = pointer;
        GP4_GLOBAL(uint32_t, 0x00852d30) = pointer;
        GP4_GLOBAL(uint32_t, 0x0066d81c) = pointer;
    }
    const uint8_t mode = GP4_GLOBAL(uint8_t, 0x0066d7a4);
    if (mode > 1u) {
        GP4_GLOBAL(uint32_t, 0x00679d2c) = 0u;
    }
    r->eflags = (r->eflags & ~0x8d5u) | compareByteFlags(mode, 1u);
}
GP4_IMPL(0x00409b87, FUN_00409b87, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=carno_0066ff6c")
