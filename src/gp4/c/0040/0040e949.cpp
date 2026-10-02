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

// FUNCTION: GP4 0x0040e949
void __cdecl FUN_0040e949(gp4::Regs* r) {
    const uint32_t incomingEax = r->eax;
    void* record = reinterpret_cast<void*>(static_cast<uintptr_t>(r->esi));
    const uint32_t force = GP4_GLOBAL(uint32_t, 0x0066d824);
    bool update = force != 0u;
    uint32_t flags = 0u;
    if (!update) {
        const uint8_t value = GP4_FIELD(uint8_t, record, 0x70);
        const uint8_t threshold = GP4_GLOBAL(uint8_t, 0x007c2e96);
        r->eax = (incomingEax & 0xffffff00u) | value;
        flags = CompareFlags8(value, threshold);
        update = static_cast<int8_t>(value) > static_cast<int8_t>(threshold);
    }
    if (update) {
        const uint8_t decremented = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0x70) - 1u);
        GP4_FIELD(uint8_t, record, 0x70) = (decremented & 0x80u) != 0u ? 0u : decremented;
        GP4_FIELD(uint8_t, record, 0x8d) |= 4u;
        const uint8_t lastFlags = static_cast<uint8_t>(GP4_FIELD(uint8_t, record, 0xcc) | 0x10u);
        GP4_FIELD(uint8_t, record, 0xcc) = lastFlags;
        flags = LogicFlags8(lastFlags);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040e949, FUN_0040e949, "regs(eax:u32,esi:ptr[0xd4]:bytes) -> (eax,cf,pf,af,zf,sf,of); globals=layout=state_0040e949")
