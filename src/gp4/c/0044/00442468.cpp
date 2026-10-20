#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// Flags of an 8-bit logic result (test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return parityFlag(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Arithmetic flags of the 8-bit `cmp left, right`.
static inline uint32_t cmpFlags8(uint32_t left, uint32_t right) {
    left &= 0xffu;
    right &= 0xffu;
    const uint32_t result = (left - right) & 0xffu;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u)
        | ((((left ^ right) & (left ^ result)) & 0x80u) != 0u ? 0x800u : 0u);
}

// Arithmetic flags of the 32-bit `cmp left, right` / `sub left, right`.
static inline uint32_t cmpFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left - right;
    return (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | ((((left ^ right) & (left ^ result)) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Flags of the 32-bit `inc` / `dec` without CF.
static inline uint32_t incFlags32(uint32_t before) {
    const uint32_t result = before + 1u;
    return parityFlag(result)
        | (((before ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (before == 0x7fffffffu ? 0x800u : 0u);
}
static inline uint32_t decFlags32(uint32_t before) {
    const uint32_t result = before - 1u;
    return parityFlag(result)
        | (((before ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (before == 0x80000000u ? 0x800u : 0u);
}

// Successor lookup in the list of flagged car numbers: the number in 0x0066d896 (bit 7 set, bit 6 clear)
// is searched in the byte list 0x0066d8f4 (count 0x0066d8ec). When found, the list is walked cyclically
// from the next entry for the first other entry that is flagged and has bit 6 clear; that entry (or the
// number itself when there is none) is stored in 0x0066d897 and CF is cleared. CF is set (nothing stored)
// when the number is unflagged, has bit 6, or is not in the list. al ends as the byte stored / loaded;
// ebx / edx are saved and restored; the flags other than CF are those of the last flag writer.
// FUNCTION: GP4 0x00442468
void __cdecl FUN_00442468(gp4::Regs* r) {
    const uint8_t* list = GP4_ARRAY(const uint8_t, 0x0066d8f4);
    uint8_t al = GP4_GLOBAL(uint8_t, 0x0066d896);
    uint32_t flags = logicFlags8(al);
    bool found = false;
    uint32_t carry = 1u;

    if ((al & 0x80u) != 0u) {
        flags = logicFlags8(al & 0x40u);
        if ((al & 0x40u) == 0u) {
            int32_t index = 0;
            for (;;) {
                flags = cmpFlags8(al, list[index]);
                if (al == list[index]) {
                    found = true;
                    break;
                }
                const int32_t count = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d8ec));
                flags = cmpFlags32(static_cast<uint32_t>(index + 1), static_cast<uint32_t>(count));
                index += 1;
                if (!(index < count)) {
                    break;
                }
            }
            if (found) {
                int32_t remaining = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d8ec));
                flags = cmpFlags32(static_cast<uint32_t>(remaining), 1u);
                remaining -= 1;
                for (;;) {
                    const int32_t count = static_cast<int32_t>(GP4_GLOBAL(uint32_t, 0x0066d8ec));
                    flags = cmpFlags32(static_cast<uint32_t>(index + 1), static_cast<uint32_t>(count));
                    index += 1;
                    if (!(index < count)) {
                        index = 0;
                    }
                    const uint8_t entry = list[index];
                    flags = logicFlags8(entry & 0x40u);
                    if ((entry & 0x40u) == 0u) {
                        flags = logicFlags8(entry);
                        if ((entry & 0x80u) != 0u) {
                            al = entry;
                            break;
                        }
                    }
                    flags = decFlags32(static_cast<uint32_t>(remaining));
                    remaining -= 1;
                    if (remaining < 0) {
                        al = GP4_GLOBAL(uint8_t, 0x0066d896);
                        break;
                    }
                }
                GP4_GLOBAL(uint8_t, 0x0066d897) = al;
                carry = 0u;
            }
        }
    }

    r->eax = (r->eax & 0xffffff00u) | al;
    r->eflags = (r->eflags & ~0x8d5u) | ((flags & 0x8d4u) | carry);
}
GP4_IMPL(0x00442468, FUN_00442468, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=order_00675958+tabs_0043ced2+list_00442468")
