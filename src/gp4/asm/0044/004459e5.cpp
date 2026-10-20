#include <gp4/gp4.h>

static inline uint32_t byteParity(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 0x04u : 0u;
}

// Flags of the 32-bit ADD left, right.
static inline uint32_t addFlags32(uint32_t left, uint32_t right) {
    const uint32_t result = left + right;
    return (result < left ? 0x001u : 0u)
        | byteParity(result)
        | (((left ^ right ^ result) & 0x10u) != 0u ? 0x010u : 0u)
        | (result == 0u ? 0x040u : 0u)
        | ((result & 0x80000000u) != 0u ? 0x080u : 0u)
        | ((~(left ^ right) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
}

// Finds the car record (22 records of 0x41c bytes at 0x0066fef0) whose car number byte +0x7c equals al. Found: eax =
// the record address, CF clear, other flags those of the equal compare (ZF, PF). Not found: eax unchanged, CF set,
// other flags those of the last `add esi, 0x41c`. ecx and esi are saved and restored.
// FUNCTION: GP4 0x004459e5
void __cdecl FUN_004459e5(gp4::Regs* r) {
    const uint8_t key = static_cast<uint8_t>(r->eax);
    uint32_t record = 0x0066fef0u;
    uint32_t flags = 0u;
    uint32_t carry = 1u;
    for (uint32_t remaining = 0x16u; remaining != 0u; --remaining) {
        const uint8_t* const car = reinterpret_cast<const uint8_t*>(static_cast<uintptr_t>(record));
        if (car[0x7c] == key) {
            r->eax = record;
            flags = 0x44u;
            carry = 0u;
            break;
        }
        flags = addFlags32(record, 0x41cu);
        record += 0x41cu;
    }
    r->eflags = (r->eflags & ~0x8d5u) | (flags & ~1u) | carry;
}
GP4_IMPL(0x004459e5, FUN_004459e5, "regs(eax:u32[0..63]) -> (eax, cf, pf, af, zf, sf, of); globals=layout=carno_0066ff6c")
