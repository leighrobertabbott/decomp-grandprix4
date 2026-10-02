#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0043fab0
void __cdecl FUN_0043fab0(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(r->edi));
    // ax = [edi+0x160] + [edi+0x178] - [edi] - [edi+0x18]
    uint16_t value = GP4_FIELD(uint16_t, record, 0x160);
    value = static_cast<uint16_t>(value + GP4_FIELD(uint16_t, record, 0x178));
    value = static_cast<uint16_t>(value - GP4_FIELD(uint16_t, record, 0));
    const uint16_t left = value;
    const uint16_t right = GP4_FIELD(uint16_t, record, 0x18);
    const uint16_t result = static_cast<uint16_t>(left - right);
    // the flags are those of the final 16-bit sub
    const uint32_t flags = (left < right ? 1u : 0u)
        | parityFlag(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result & 0x8000u) != 0u ? 0x80u : 0u)
        | (((left ^ right) & (left ^ result) & 0x8000u) != 0u ? 0x800u : 0u);
    // movsx eax, ax
    r->eax = static_cast<uint32_t>(static_cast<int32_t>(static_cast<int16_t>(result)));
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0043fab0, FUN_0043fab0, "regs(edi:ptr[0x182]:bytes) -> (eax, cf, pf, af, zf, sf, of)")
