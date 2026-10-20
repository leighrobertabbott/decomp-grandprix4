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

// Raises bit 6 of the byte 0x008914e0 when every non-null one of the two car record pointers [0x0066d820] and
// [0x0066d828] has bit 7 of its byte +0x78 set and at least one of them is non-null. esi / ebx are loaded with the
// pointers; the flags are those of the last test / or on the path taken.
// FUNCTION: GP4 0x004454a4
void __cdecl FUN_004454a4(gp4::Regs* r) {
    const uint32_t first = GP4_GLOBAL(uint32_t, 0x0066d820);
    const uint32_t second = GP4_GLOBAL(uint32_t, 0x0066d828);
    r->esi = first;
    r->ebx = second;

    uint32_t flags = logicFlags32(first);
    bool raise = false;
    if (first == 0u) {
        flags = logicFlags32(second);
        if (second != 0u) {
            const uint32_t state = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(second))[0x78] & 0x80u;
            flags = logicFlags8(state);
            raise = state != 0u;
        }
    } else {
        const uint32_t state = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(first))[0x78] & 0x80u;
        flags = logicFlags8(state);
        if (state != 0u) {
            flags = logicFlags32(second);
            if (second == 0u) {
                raise = true;
            } else {
                const uint32_t other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(second))[0x78] & 0x80u;
                flags = logicFlags8(other);
                raise = other != 0u;
            }
        }
    }

    if (raise) {
        const uint8_t raised = static_cast<uint8_t>(GP4_GLOBAL(uint8_t, 0x008914e0) | 0x40u);
        GP4_GLOBAL(uint8_t, 0x008914e0) = raised;
        flags = logicFlags8(raised);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x004454a4, FUN_004454a4, "regs() -> (ebx, esi, cf, pf, af, zf, sf, of); globals=fuzz:i32+layout=state_0040fa01")
