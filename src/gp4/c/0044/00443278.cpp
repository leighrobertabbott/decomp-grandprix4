#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags8(uint32_t result) {
    result &= 0xffu;
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// Flags of a 32-bit logic result (or / and / test): CF = OF = AF = 0.
static inline uint32_t logicFlags32(uint32_t result) {
    return byteParity(result) | (result == 0u ? 0x40u : 0u) | ((result & 0x80000000u) != 0u ? 0x80u : 0u);
}

// Arithmetic flags of `inc value` (CF is not touched by INC and is clear on every path here).
static inline uint32_t incFlags32(uint32_t value) {
    const uint32_t result = value + 1u;
    return byteParity(result)
        | (((value ^ 1u ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
        | (value == 0x7fffffffu ? 0x800u : 0u);
}

// Scans the [0x0066d8ec] key bytes at 0x0066d8f4 (a set bit 7 skips the entry) and hands each to 0x0044329f until
// one reports carry. Returns CF set when found (flags of that call), otherwise CF clear with the flags of the
// last `inc esi` (or of `or ecx, ecx` when the count is zero). pushal / popal restore every register.
// FUNCTION: GP4 0x00443278
void __cdecl FUN_00443278(gp4::Regs* r) {
    uint32_t count = GP4_GLOBAL(uint32_t, 0x0066d8ec);
    uint32_t flags = logicFlags32(count);
    uint32_t carry = 0u;
    if (count != 0u) {
        uint32_t entry = 0x0066d8f4u;
        for (;;) {
            const uint32_t key = *reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(entry));
            flags = logicFlags8(key);
            if ((key & 0x80u) == 0u) {
                gp4::Regs call = *r;
                call.st_in = 0;
                call.st_out = 0;
                call.eax = (r->eax & 0xffffff00u) | key;
                call.esi = entry;
                gp4::call_regs(0x0044329f, call);
                flags = call.eflags & 0x8d5u;
                if ((flags & 1u) != 0u) {
                    carry = 1u;
                    break;
                }
            }
            flags = incFlags32(entry);
            ++entry;
            --count;
            if (count == 0u) {
                break;
            }
        }
    }
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x00443278, FUN_00443278, "regs() -> (cf, pf, af, zf, sf, of); globals=layout=flags_0066d909+keys_0044329f+list_00443278")
