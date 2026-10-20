#include <gp4/gp4.h>

static inline uint32_t parityFlag(uint32_t value) {
    uint32_t parity = value & 0xffu;
    parity ^= parity >> 4;
    parity ^= parity >> 2;
    parity ^= parity >> 1;
    return (parity & 1u) == 0u ? 4u : 0u;
}

// FUNCTION: GP4 0x0040c6b6
// Compares edi with 0x00675a82 + 10 * word [record+0x7a], where record is the 0x160-byte record
// before [0x00866ecc]. pushal/popal keep every register but not the flags.
void __cdecl FUN_0040c6b6(gp4::Regs* r) {
    uint8_t* record = reinterpret_cast<uint8_t*>(static_cast<uintptr_t>(GP4_GLOBAL(uint32_t, 0x00866ecc) - 0x160u));
    const uint32_t expected = static_cast<uint32_t>(GP4_FIELD(uint16_t, record, 0x7a)) * 0xau + 0x00675a82u;
    const uint32_t left = r->edi;
    const uint32_t result = left - expected;
    uint32_t flags;
    if (left != expected) {
        // cmp edi, ebx ; jne: the flags stay those of the cmp
        flags = (left < expected ? 1u : 0u)
            | parityFlag(result)
            | (((left ^ expected ^ result) & 0x10u) != 0u ? 0x10u : 0u)
            | ((result & 0x80000000u) != 0u ? 0x80u : 0u)
            | (((left ^ expected) & (left ^ result) & 0x80000000u) != 0u ? 0x800u : 0u);
    } else {
        // or eax, eax leaves CF=OF=AF=0
        const uint32_t value = r->eax;
        flags = parityFlag(value) | (value == 0u ? 0x40u : 0u) | ((value & 0x80000000u) != 0u ? 0x80u : 0u);
    }
    r->eflags = (r->eflags & ~0x8d5u) | flags;
}
GP4_IMPL(0x0040c6b6, FUN_0040c6b6, "regs(eax:u32, edi:u32[6773378..6773420]) -> (cf, pf, af, zf, sf, of); globals=layout=records_008943a0")
