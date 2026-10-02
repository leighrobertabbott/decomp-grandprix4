#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x00418097
void __cdecl FUN_00418097(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->esi));
    if ((GP4_FIELD(uint8_t, record, 0x7b) & 2u) != 0u) {
        GP4_FIELD(uint8_t, record, 0x7b) &= 0xfdu;
        GP4_FIELD(uint8_t, record, 0xce) |= 1u;
    }
    const uint32_t result = GP4_FIELD(uint8_t, record, 0xfe) & 0xdfu;
    GP4_FIELD(uint8_t, record, 0xfe) = static_cast<uint8_t>(result);
    // and byte: CF=OF=0, AF cleared in the pinned model
    const uint32_t flags = parityFlag(result)
        | (result == 0u ? 0x40u : 0u)
        | (result & 0x80u);
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00418097, FUN_00418097, "regs(esi:ptr[0x106]:bytes) -> (cf, pf, af, zf, sf, of)")
