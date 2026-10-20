#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    value &= 0xffu;
    value ^= value >> 4;
    value ^= value >> 2;
    value ^= value >> 1;
    return (value & 1u) == 0u ? 4u : 0u;
}

// flags of the 32-bit "add lhs, rhs" with result = lhs + rhs
static inline uint32_t addFlags32(uint32_t lhs, uint32_t rhs, uint32_t result) {
    return (result < lhs ? 0x1u : 0u)
        | parityFlag(result)
        | (((lhs ^ rhs ^ result) & 0x10u) != 0u ? 0x10u : 0u)
        | (result == 0u ? 0x40u : 0u)
        | ((result >> 24) & 0x80u)
        | (((~(lhs ^ rhs) & (lhs ^ result)) >> 31) != 0u ? 0x800u : 0u);
}

// Searches the 22 records (0x41c bytes each) of the table at 0x0066fef0 for one whose byte at +0x7c
// equals AL. Found: eax = record address, CF set (ZF/PF from the equal compare). Not found: eax
// unchanged, CF clear, other flags from the last "add esi, 0x41c" of the loop (loop leaves flags).
// FUNCTION: GP4 0x00408359
void __cdecl FUN_00408359(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    uint8_t* record = GP4_ARRAY(uint8_t, 0x0066fef0);
    uint32_t previous = 0u;
    for (uint32_t remaining = 0x16u; remaining != 0u; --remaining) {
        if (key == GP4_FIELD(uint8_t, record, 0x7c)) {
            r->eax = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
            // equal compare: ZF and PF set; stc sets CF
            r->eflags = (r->eflags & ~0x8d5u) | 0x45u;
            return;
        }
        previous = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record));
        record += 0x41c;
    }
    const uint32_t flags = addFlags32(previous, 0x41cu, static_cast<uint32_t>(reinterpret_cast<uintptr_t>(record))) & ~0x1u;
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x00408359, FUN_00408359, "regs(eax:u32) -> (eax, cf, pf, af, zf, sf, of)")
