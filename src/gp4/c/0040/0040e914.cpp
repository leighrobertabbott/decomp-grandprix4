#include <gp4/gp4.h>

static inline uint32_t LogicFlags8(uint8_t result) {
    uint32_t parity = result;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return ((~parity & 1u) << 2) | (result == 0u ? 0x40u : 0u)
        | (static_cast<uint32_t>(result) & 0x80u);
}

static inline uint32_t CompareFlags8(uint8_t lhs, uint8_t rhs) {
    const uint8_t result = static_cast<uint8_t>(lhs - rhs);
    return LogicFlags8(result) | (lhs < rhs ? 1u : 0u)
        | ((lhs ^ rhs ^ result) & 0x10u)
        | (((lhs ^ rhs) & (lhs ^ result) & 0x80u) << 4);
}

// FUNCTION: GP4 0x0040e914
void __cdecl FUN_0040e914(gp4::Regs* r) {
    const uint32_t incomingEax = r->eax;
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const uint32_t force = GP4_GLOBAL(uint32_t, 0x0066d824);
    if (force == 0u) {
        const uint8_t value = GP4_FIELD(uint8_t, record, 0x70);
        const uint8_t threshold = GP4_GLOBAL(uint8_t, 0x007c2e96);
        r->eax = (incomingEax & 0xffffff00u) | value;
        if (static_cast<int8_t>(value) <= static_cast<int8_t>(threshold)) {
            r->eflags = (r->eflags & ~0x8d5u) | CompareFlags8(value, threshold);
            return;
        }
        if ((GP4_FIELD(uint8_t, record, 0x8d) & 2u) == 0u) {
            GP4_GLOBAL(uint32_t, 0x0066d824) = r->esi;
        }
    }
    GP4_FIELD(uint8_t, record, 0x8d) |= 4u;
    const uint8_t lastFlags = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0xcc) | 0x10u);
    GP4_FIELD(uint8_t, record, 0xcc) = lastFlags;
    r->eflags = (r->eflags & ~0x8d5u) | LogicFlags8(lastFlags);
}
GP4_IMPL(0x0040e914, FUN_0040e914, "regs(eax:u32,esi:ptr[0xd4]:bytes) -> (eax,cf,pf,af,zf,sf,of); globals=layout=state_0040e949")
