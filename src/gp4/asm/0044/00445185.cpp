#include <gp4/gp4.h>

static inline uint32_t byteParity(uint8_t value) {
    uint32_t parity = value;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of an 8-bit OR / TEST result (CF = OF = AF = 0).
static inline uint32_t logicFlags8(uint8_t result) {
    return byteParity(result)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80u) != 0u ? 0x080u : 0u);
}

// FUNCTION: GP4 0x00445185
void __cdecl FUN_00445185(gp4::Regs* r) {
    const uint8_t* record = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(r->esi));
    uint8_t bits = 0;
    uint32_t flags = 0;

    if ((record[0x231] & 0x20) != 0) {
        bits |= 0x08;
    }
    if ((record[0x71] & 0x80) != 0) {
        bits |= 0x10;
    }
    if ((record[0xfe] & 0x04) != 0) {
        bits |= 0x20;
    }
    if ((record[0x230] & 0x80) != 0) {
        bits |= 0x40;
    }
    if ((record[0x7b] & 0x02) != 0) {
        bits |= 0x80;
        flags = logicFlags8(bits);
    } else {
        flags = logicFlags8(static_cast<uint8_t>(record[0x7b] & 0x02));
    }

    r->eax = (r->eax & 0xffffff00u) | bits;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00445185, FUN_00445185, "regs(eax:u32, esi:ptr[0x239]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
