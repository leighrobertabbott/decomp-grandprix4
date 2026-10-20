#include <gp4/gp4.h>

static inline uint32_t Parity(uint32_t result) {
    uint32_t parity = result & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ~parity & 1u;
}

// flags of an 8-bit test: CF = OF = AF = 0
static inline uint32_t TestFlags8(uint32_t result) {
    return (Parity(result) << 2) | ((result & 0xffu) == 0u ? 0x40u : 0u) | (result & 0x80u);
}

// flags of an 8-bit compare a - b
static inline uint32_t CmpFlags8(uint32_t a, uint32_t b) {
    const uint32_t result = (a - b) & 0xffu;
    return (a < b ? 1u : 0u) | (Parity(result) << 2) | ((a ^ b ^ result) & 0x10u)
        | (result == 0u ? 0x40u : 0u) | (result & 0x80u)
        | (((a ^ b) & (a ^ result) & 0x80u) << 4);
}

// FUNCTION: GP4 0x00411017
void __cdecl FUN_00411017(gp4::Regs* r) {
    const uint8_t* self = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    const uint32_t savedEbx = r->ebx;   // push ebx ... pop ebx

    uint32_t result;
    uint32_t flags;
    if ((self[0xfeu] & 4u) != 0u) {
        result = 1u;
        flags = TestFlags8(self[0xfeu] & 4u);
    } else {
        const uint32_t mode = GP4_GLOBAL(uint8_t, 0x007c2ea9);
        result = 0u;
        flags = CmpFlags8(mode, 0x80u);
        if (mode == 0x80u) {
            gp4::call_regs(0x0043b974, *r);     // sets ebx and CF (jae = no teammate hit)
            flags = r->eflags & 0x8d5u;
            if ((r->eflags & 1u) != 0u) {
                const uint8_t* other = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->ebx));
                const uint32_t bit = other[0xfeu] & 4u;
                flags = TestFlags8(bit);
                if (bit != 0u) {
                    result = 2u;
                }
            }
        }
    }

    r->eax = result;
    r->ebx = savedEbx;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00411017, FUN_00411017, "regs(esi:ptr[0x106]:layout=rec_00411017) -> (eax, cf, pf, af, zf, sf, of); globals=layout=state_00411017")
